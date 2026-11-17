#include "teradata_filter.hpp"

#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_conjunction_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/planner/expression/bound_operator_expression.hpp"
#include "duckdb/planner/filter/expression_filter.hpp"
#include "duckdb/planner/filter/table_filter_functions.hpp"
#include "duckdb/planner/table_filter_set.hpp"

#include <algorithm>
#include <iterator>

namespace duckdb {

static string TransformBlob(const string &val) {
	char const HEX_DIGITS[] = "0123456789ABCDEF";

	string result = "'";
	for (idx_t i = 0; i < val.size(); i++) {
		uint8_t byte_val = static_cast<uint8_t>(val[i]);
		result += HEX_DIGITS[(byte_val >> 4) & 0xf];
		result += HEX_DIGITS[byte_val & 0xf];
	}

	// Hexadecimal byte literal
	// https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/SQL-Data-Types-and-Literals/Data-Literals/Hexadecimal-Byte-Literals
	result += "'XBV";
	return result;
}

static string TransformLiteral(const Value &val) {
	// TODO: More literals
	switch (val.type().id()) {
	case LogicalTypeId::BLOB:
		return TransformBlob(StringValue::Get(val));
	default:
		return KeywordHelper::WriteQuoted(val.ToString());
	}
}

// A translated filter predicate. An empty optional means the predicate always holds and can be dropped,
// e.g. an optional or join-derived filter that DuckDB does not need for correctness.
using Predicate = optional<string>;

struct ComparisonEntry {
	ExpressionType type;
	const char *op;
};

static const ComparisonEntry COMPARISONS[] = {
    {ExpressionType::COMPARE_EQUAL, "="},
    {ExpressionType::COMPARE_NOTEQUAL, "<>"},
    {ExpressionType::COMPARE_LESSTHAN, "<"},
    {ExpressionType::COMPARE_GREATERTHAN, ">"},
    {ExpressionType::COMPARE_LESSTHANOREQUALTO, "<="},
    {ExpressionType::COMPARE_GREATERTHANOREQUALTO, ">="},
};

static optional_ptr<const ComparisonEntry> FindComparison(ExpressionType type) {
	const auto entry = std::find_if(std::begin(COMPARISONS), std::end(COMPARISONS),
	                                [&](const ComparisonEntry &e) { return e.type == type; });
	return entry == std::end(COMPARISONS) ? nullptr : entry;
}

// Join-derived filters that DuckDB keeps enforcing itself, so they never need to be pushed down
static bool IsSkippableInternalFilter(const BoundFunctionExpression &func) {
	const auto &name = func.Function().GetName();
	return name == DynamicFilterScalarFun::NAME || name == BloomFilterScalarFun::NAME ||
	       name == PrefixRangeScalarFun::NAME;
}

[[noreturn]] static void ThrowUnsupported(const Expression &expr) {
	throw NotImplementedException("Unsupported filter in Teradata scan: %s", expr.ToString());
}

// An operand of a predicate: the filtered column or a constant
static string TransformOperand(const string &column_name, const Expression &expr) {
	switch (expr.GetExpressionClass()) {
	case ExpressionClass::BOUND_REF:
	case ExpressionClass::BOUND_COLUMN_REF:
		return column_name;
	case ExpressionClass::BOUND_CONSTANT:
		return TransformLiteral(expr.Cast<BoundConstantExpression>().GetValue());
	default:
		ThrowUnsupported(expr);
	}
}

static vector<string> TransformOperands(const string &column_name, const vector<unique_ptr<Expression>> &exprs) {
	vector<string> result;
	std::transform(exprs.begin(), exprs.end(), std::back_inserter(result),
	               [&](const unique_ptr<Expression> &expr) { return TransformOperand(column_name, *expr); });
	return result;
}

static Predicate TransformPredicate(const string &column_name, const Expression &expr);

static Predicate TransformConjunction(const string &column_name, const BoundConjunctionExpression &conj) {
	vector<Predicate> children;
	for (auto &child : conj.GetChildren()) {
		children.push_back(TransformPredicate(column_name, *child));
	}
	const auto is_always_true = [](const Predicate &p) {
		return !p.has_value();
	};
	const auto is_and = conj.GetExpressionType() == ExpressionType::CONJUNCTION_AND;
	if (!is_and && std::any_of(children.begin(), children.end(), is_always_true)) {
		// "x OR TRUE" is always TRUE
		return {};
	}
	vector<string> terms;
	for (auto &child : children) {
		if (child) {
			terms.push_back(std::move(*child));
		}
	}
	if (terms.empty()) {
		return {};
	}
	return "(" + StringUtil::Join(terms, is_and ? " AND " : " OR ") + ")";
}

static Predicate TransformOperator(const string &column_name, const BoundOperatorExpression &op) {
	auto operands = TransformOperands(column_name, op.GetChildren());
	switch (op.GetExpressionType()) {
	case ExpressionType::OPERATOR_IS_NULL:
		return operands[0] + " IS NULL";
	case ExpressionType::OPERATOR_IS_NOT_NULL:
		return operands[0] + " IS NOT NULL";
	case ExpressionType::COMPARE_IN:
	case ExpressionType::COMPARE_NOT_IN: {
		const vector<string> in_list(operands.begin() + 1, operands.end());
		const auto in_op = op.GetExpressionType() == ExpressionType::COMPARE_IN ? " IN (" : " NOT IN (";
		return operands[0] + in_op + StringUtil::Join(in_list, ", ") + ")";
	}
	default:
		ThrowUnsupported(op);
	}
}

// Optional filters are not required for correctness: push the child down when we can, otherwise drop it
static Predicate TransformOptional(const string &column_name, const BoundFunctionExpression &func) {
	const auto child = ExpressionFilter::GetOptionalFilterChild(func);
	if (!child) {
		return {};
	}
	try {
		return TransformPredicate(column_name, *child);
	} catch (NotImplementedException &) {
		return {};
	}
}

static Predicate TransformFunction(const string &column_name, const BoundFunctionExpression &func) {
	if (ExpressionFilter::IsRootOptionalExpression(func)) {
		return TransformOptional(column_name, func);
	}
	if (IsSkippableInternalFilter(func)) {
		return {};
	}
	const auto comparison = FindComparison(func.GetExpressionType());
	if (!comparison) {
		ThrowUnsupported(func);
	}
	return StringUtil::Format("%s %s %s", TransformOperand(column_name, BoundComparisonExpression::Left(func)),
	                          comparison->op, TransformOperand(column_name, BoundComparisonExpression::Right(func)));
}

// Translate a pushed-down filter expression into Teradata SQL.
// Expressions that cannot be translated throw, so that no filter is ever silently ignored.
static Predicate TransformPredicate(const string &column_name, const Expression &expr) {
	switch (expr.GetExpressionClass()) {
	case ExpressionClass::BOUND_CONJUNCTION:
		return TransformConjunction(column_name, expr.Cast<BoundConjunctionExpression>());
	case ExpressionClass::BOUND_OPERATOR:
		return TransformOperator(column_name, expr.Cast<BoundOperatorExpression>());
	case ExpressionClass::BOUND_FUNCTION:
		return TransformFunction(column_name, expr.Cast<BoundFunctionExpression>());
	default:
		ThrowUnsupported(expr);
	}
}

string TeradataFilter::Transform(const vector<column_t> &column_ids, optional_ptr<TableFilterSet> filters,
                                 const vector<string> &names) {
	if (filters->HasMultiColumnFilters()) {
		throw NotImplementedException("Multi-column filters are not supported in Teradata scans");
	}

	vector<string> terms;
	for (const auto &entry : *filters) {
		const idx_t column_index = entry.GetIndex();
		const auto col_name = KeywordHelper::WriteQuoted(names[column_ids[column_index]], '"');
		const auto &expr_filter = ExpressionFilter::GetExpressionFilter(entry.Filter(), "TeradataFilter::Transform");
		auto predicate = TransformPredicate(col_name, *expr_filter.expr);
		if (predicate) {
			terms.push_back(std::move(*predicate));
		}
	}
	return StringUtil::Join(terms, " AND ");
}

} // namespace duckdb