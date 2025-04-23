#include "teradata_filter.hpp"

#include <teradata_type.hpp>
#include <duckdb/planner/filter/constant_filter.hpp>
#include <duckdb/planner/filter/in_filter.hpp>
#include <duckdb/planner/filter/optional_filter.hpp>
#include <duckdb/planner/filter/struct_filter.hpp>

namespace duckdb {

static string TransformFilter(const string &column_name, const TableFilter &filter);
static string CreateExpression(const string &column_name, const vector<unique_ptr<TableFilter>> &filters,
                               const string &op) {
	vector<string> filter_entries;
	for (auto &filter : filters) {
		auto filter_str = TransformFilter(column_name, *filter);
		if (!filter_str.empty()) {
			filter_entries.push_back(std::move(filter_str));
		}
	}
	if (filter_entries.empty()) {
		return string();
	}
	return "(" + StringUtil::Join(filter_entries, " " + op + " ") + ")";
}

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

static string TransformComparision(ExpressionType type) {
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
		throw NotImplementedException("Unsupported expression type");
	}
}

static string TransformFilter(const string &column_name, const TableFilter &filter) {
	switch (filter.filter_type) {
	case TableFilterType::IS_NULL:
		return column_name + " IS NULL";
	case TableFilterType::IS_NOT_NULL:
		return column_name + " IS NOT NULL";
	case TableFilterType::CONJUNCTION_AND: {
		auto &conjunction_filter = filter.Cast<ConjunctionAndFilter>();
		return CreateExpression(column_name, conjunction_filter.child_filters, "AND");
	}
	case TableFilterType::CONJUNCTION_OR: {
		auto &conjunction_filter = filter.Cast<ConjunctionOrFilter>();
		return CreateExpression(column_name, conjunction_filter.child_filters, "OR");
	}
	case TableFilterType::CONSTANT_COMPARISON: {
		auto &constant_filter = filter.Cast<ConstantFilter>();
		auto constant_string = TransformLiteral(constant_filter.constant);
		auto operator_string = TransformComparision(constant_filter.comparison_type);
		return StringUtil::Format("%s %s %s", column_name, operator_string, constant_string);
	}
	case TableFilterType::STRUCT_EXTRACT: {
		throw NotImplementedException("Struct extract filter is not supported in Teradata");
	}
	case TableFilterType::OPTIONAL_FILTER: {
		auto &optional_filter = filter.Cast<OptionalFilter>();
		return TransformFilter(column_name, *optional_filter.child_filter);
	}
	case TableFilterType::IN_FILTER: {
		auto &in_filter = filter.Cast<InFilter>();
		string in_list;
		for (auto &val : in_filter.values) {
			if (!in_list.empty()) {
				in_list += ", ";
			}
			in_list += TransformLiteral(val);
		}
		return column_name + " IN (" + in_list + ")";
	}
	case TableFilterType::DYNAMIC_FILTER:
		return string();
	default:
		throw InternalException("Unsupported table filter type");
	}
}

string TeradataFilter::Transform(const vector<column_t> &column_ids, optional_ptr<TableFilterSet> filters,
                                 const vector<string> &names) {

	string result;
	for (auto &entry : filters->filters) {
		auto col_name = KeywordHelper::WriteQuoted(names[column_ids[entry.first]], '"');
		auto &filter = entry.second;
		auto filter_text = TransformFilter(col_name, *filter);
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