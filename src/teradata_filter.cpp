#include "teradata_filter.hpp"

#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_conjunction_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/planner/expression/bound_operator_expression.hpp"
#include "duckdb/planner/filter/expression_filter.hpp"
#include "duckdb/planner/filter/table_filter_functions.hpp"
#include "duckdb/planner/table_filter_set.hpp"

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

static string TransformComparison(ExpressionType type) {
	switch (type) {
	case ExpressionType::COMPARE_EQUAL:
		return "=";
	case ExpressionType::COMPARE_NOTEQUAL:
		return "<>";
	case ExpressionType::COMPARE_LESSTHAN:
		return "<";
	case ExpressionType::COMPARE_GREATERTHAN:
		return ">";
	case ExpressionType::COMPARE_LESSTHANOREQUALTO:
		return "<=";
	case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		return ">=";
	default:
		return string();
	}
}

// Filters produced by DuckDB itself (join-derived or optional) that are never needed for correctness.
// Returns true if the filter can be skipped entirely.
static bool IsSkippableInternalFilter(const BoundFunctionExpression &func) {
	const auto &name = func.Function().GetName();
	return name == DynamicFilterScalarFun::NAME || name == BloomFilterScalarFun::NAME ||
	       name == PrefixRangeScalarFun::NAME;
}

// Translate a pushed-down filter expression into Teradata SQL.
// An empty result means the (sub)expression is always satisfied and can be dropped (e.g. an optional filter).
// Expressions that cannot be translated throw, so that no filter is ever silently ignored.
static string TransformExpression(const string &column_name, const Expression &expr) {
	switch (expr.GetExpressionClass()) {
	case ExpressionClass::BOUND_REF:
	case ExpressionClass::BOUND_COLUMN_REF:
		return column_name;
	case ExpressionClass::BOUND_CONSTANT:
		return TransformLiteral(expr.Cast<BoundConstantExpression>().GetValue());
	case ExpressionClass::BOUND_CONJUNCTION: {
		auto &conj = expr.Cast<BoundConjunctionExpression>();
		const auto is_and = conj.GetExpressionType() == ExpressionType::CONJUNCTION_AND;
		vector<string> entries;
		for (auto &child : conj.GetChildren()) {
			auto child_str = TransformExpression(column_name, *child);
			if (child_str.empty()) {
				if (!is_and) {
					// "x OR TRUE" is always TRUE
					return string();
				}
				continue;
			}
			entries.push_back(std::move(child_str));
		}
		if (entries.empty()) {
			return string();
		}
		return "(" + StringUtil::Join(entries, is_and ? " AND " : " OR ") + ")";
	}
	case ExpressionClass::BOUND_OPERATOR: {
		auto &op = expr.Cast<BoundOperatorExpression>();
		auto &children = op.GetChildren();
		switch (op.GetExpressionType()) {
		case ExpressionType::OPERATOR_IS_NULL:
			return TransformExpression(column_name, *children[0]) + " IS NULL";
		case ExpressionType::OPERATOR_IS_NOT_NULL:
			return TransformExpression(column_name, *children[0]) + " IS NOT NULL";
		case ExpressionType::COMPARE_IN:
		case ExpressionType::COMPARE_NOT_IN: {
			string in_list;
			for (idx_t i = 1; i < children.size(); i++) {
				if (i > 1) {
					in_list += ", ";
				}
				in_list += TransformExpression(column_name, *children[i]);
			}
			const auto in_op = op.GetExpressionType() == ExpressionType::COMPARE_IN ? " IN (" : " NOT IN (";
			return TransformExpression(column_name, *children[0]) + in_op + in_list + ")";
		}
		default:
			break;
		}
		break;
	}
	case ExpressionClass::BOUND_FUNCTION: {
		auto &func = expr.Cast<BoundFunctionExpression>();
		if (ExpressionFilter::IsRootOptionalExpression(func)) {
			// Optional filters are not required for correctness - push down the child if we can
			auto child = ExpressionFilter::GetOptionalFilterChild(func);
			if (!child) {
				return string();
			}
			try {
				return TransformExpression(column_name, *child);
			} catch (NotImplementedException &) {
				return string();
			}
		}
		if (IsSkippableInternalFilter(func)) {
			return string();
		}
		const auto op_str = TransformComparison(func.GetExpressionType());
		if (!op_str.empty()) {
			auto left = TransformExpression(column_name, BoundComparisonExpression::Left(func));
			auto right = TransformExpression(column_name, BoundComparisonExpression::Right(func));
			return StringUtil::Format("%s %s %s", left, op_str, right);
		}
		break;
	}
	default:
		break;
	}
	throw NotImplementedException("Unsupported filter in Teradata scan: %s", expr.ToString());
}

string TeradataFilter::Transform(const vector<column_t> &column_ids, optional_ptr<TableFilterSet> filters,
                                 const vector<string> &names) {
	if (filters->HasMultiColumnFilters()) {
		throw NotImplementedException("Multi-column filters are not supported in Teradata scans");
	}

	string result;
	for (const auto &entry : *filters) {
		const idx_t column_index = entry.GetIndex();
		auto col_name = KeywordHelper::WriteQuoted(names[column_ids[column_index]], '"');
		auto &expr_filter = ExpressionFilter::GetExpressionFilter(entry.Filter(), "TeradataFilter::Transform");
		auto filter_text = TransformExpression(col_name, *expr_filter.expr);
		if (filter_text.empty()) {
			continue;
		}
		if (!result.empty()) {
			result += " AND ";
		}
		result += filter_text;
	}

	return result;
}

} // namespace duckdb