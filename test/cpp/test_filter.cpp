#include "catch.hpp"

#include "teradata_delete_update.hpp"
#include "teradata_filter.hpp"

#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_conjunction_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_operator_expression.hpp"
#include "duckdb/planner/expression/bound_reference_expression.hpp"
#include "duckdb/planner/filter/expression_filter.hpp"
#include "duckdb/planner/filter/table_filter_functions.hpp"
#include "duckdb/planner/table_filter_set.hpp"

namespace duckdb {

namespace {

unique_ptr<Expression> Col() {
	return make_uniq<BoundReferenceExpression>(LogicalType::INTEGER, 0ULL);
}

unique_ptr<Expression> Const(Value v) {
	return make_uniq<BoundConstantExpression>(std::move(v));
}

unique_ptr<Expression> Cmp(ExpressionType type, unique_ptr<Expression> left, unique_ptr<Expression> right) {
	return BoundComparisonExpression::Create(type, std::move(left), std::move(right));
}

unique_ptr<Expression> ColCmp(ExpressionType type, int32_t value) {
	return Cmp(type, Col(), Const(Value::INTEGER(value)));
}

unique_ptr<Expression> Conj(ExpressionType type, unique_ptr<Expression> a, unique_ptr<Expression> b) {
	auto result = make_uniq<BoundConjunctionExpression>(type);
	result->GetChildrenMutable().push_back(std::move(a));
	result->GetChildrenMutable().push_back(std::move(b));
	return std::move(result);
}

unique_ptr<Expression> Not() {
	return make_uniq<BoundOperatorExpression>(ExpressionType::OPERATOR_NOT, LogicalType::BOOLEAN);
}

unique_ptr<Expression> Optional(unique_ptr<Expression> child) {
	return CreateOptionalFilterExpression(std::move(child), LogicalType::INTEGER);
}

TableFilterSet FiltersOn(idx_t column, unique_ptr<Expression> expr) {
	TableFilterSet set;
	set.PushFilter(ProjectionIndex(column), make_uniq<ExpressionFilter>(std::move(expr)));
	return set;
}

string Transform(unique_ptr<Expression> expr) {
	auto set = FiltersOn(0, std::move(expr));
	return TeradataFilter::Transform({0}, &set, {"c"});
}

} // namespace

TEST_CASE("Comparisons are pushed down", "[filter]") {
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_EQUAL, 5)) == "\"c\" = '5'");
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_NOTEQUAL, 5)) == "\"c\" <> '5'");
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_LESSTHAN, 5)) == "\"c\" < '5'");
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_GREATERTHAN, 5)) == "\"c\" > '5'");
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_LESSTHANOREQUALTO, 5)) == "\"c\" <= '5'");
	CHECK(Transform(ColCmp(ExpressionType::COMPARE_GREATERTHANOREQUALTO, 5)) == "\"c\" >= '5'");
	CHECK(Transform(Cmp(ExpressionType::COMPARE_LESSTHAN, Const(Value::INTEGER(5)), Col())) == "'5' < \"c\"");
}

TEST_CASE("IN, NOT IN and NULL checks are pushed down", "[filter]") {
	auto in = ExpressionFilter::CreateInExpression(Col(), {Value::INTEGER(1), Value::INTEGER(2)});
	CHECK(Transform(std::move(in)) == "\"c\" IN ('1', '2')");

	auto not_in = make_uniq<BoundOperatorExpression>(ExpressionType::COMPARE_NOT_IN, LogicalType::BOOLEAN);
	not_in->GetChildrenMutable().push_back(Col());
	not_in->GetChildrenMutable().push_back(Const(Value::INTEGER(3)));
	CHECK(Transform(std::move(not_in)) == "\"c\" NOT IN ('3')");

	CHECK(Transform(ExpressionFilter::CreateNullCheckExpression(Col(), ExpressionType::OPERATOR_IS_NULL)) ==
	      "\"c\" IS NULL");
	CHECK(Transform(ExpressionFilter::CreateNullCheckExpression(Col(), ExpressionType::OPERATOR_IS_NOT_NULL)) ==
	      "\"c\" IS NOT NULL");
}

TEST_CASE("Conjunctions are pushed down", "[filter]") {
	CHECK(Transform(Conj(ExpressionType::CONJUNCTION_AND, ColCmp(ExpressionType::COMPARE_GREATERTHAN, 1),
	                     ColCmp(ExpressionType::COMPARE_LESSTHANOREQUALTO, 10))) == "(\"c\" > '1' AND \"c\" <= '10')");
	CHECK(Transform(Conj(ExpressionType::CONJUNCTION_OR, ColCmp(ExpressionType::COMPARE_EQUAL, 1),
	                     ColCmp(ExpressionType::COMPARE_EQUAL, 2))) == "(\"c\" = '1' OR \"c\" = '2')");
}

TEST_CASE("Literals are quoted for Teradata", "[filter]") {
	CHECK(Transform(Cmp(ExpressionType::COMPARE_EQUAL, Col(), Const(Value("it's")))) == "\"c\" = 'it''s'");
	CHECK(Transform(Cmp(ExpressionType::COMPARE_EQUAL, Col(),
	                    Const(Value::BLOB(const_data_ptr_cast("\x01\xAB"), 2)))) == "\"c\" = '01AB'XBV");
}

TEST_CASE("Optional filters are pushed down when possible and dropped otherwise", "[filter]") {
	CHECK(Transform(Optional(ColCmp(ExpressionType::COMPARE_GREATERTHAN, 3))) == "\"c\" > '3'");
	// An untranslatable optional filter is TRUE: it disappears from an AND ...
	CHECK(Transform(Conj(ExpressionType::CONJUNCTION_AND, ColCmp(ExpressionType::COMPARE_GREATERTHAN, 1),
	                     Optional(Not()))) == "(\"c\" > '1')");
	// ... and makes an OR always true
	CHECK(Transform(Conj(ExpressionType::CONJUNCTION_OR, ColCmp(ExpressionType::COMPARE_EQUAL, 1), Optional(Not()))) ==
	      "");
}

TEST_CASE("Untranslatable mandatory filters throw instead of being dropped", "[filter]") {
	REQUIRE_THROWS_AS(Transform(Not()), NotImplementedException);
	REQUIRE_THROWS_AS(Transform(Conj(ExpressionType::CONJUNCTION_AND, ColCmp(ExpressionType::COMPARE_EQUAL, 1), Not())),
	                  NotImplementedException);
	REQUIRE_THROWS_AS(Transform(Cmp(ExpressionType::COMPARE_DISTINCT_FROM, Col(), Const(Value::INTEGER(1)))),
	                  NotImplementedException);
}

TEST_CASE("Filters on several columns are joined with AND", "[filter]") {
	TableFilterSet set;
	set.PushFilter(ProjectionIndex(0), make_uniq<ExpressionFilter>(ColCmp(ExpressionType::COMPARE_EQUAL, 1)));
	set.PushFilter(ProjectionIndex(1), make_uniq<ExpressionFilter>(ExpressionFilter::CreateNullCheckExpression(
	                                       Col(), ExpressionType::OPERATOR_IS_NULL)));
	// Scan column i reads table column column_ids[i]
	CHECK(TeradataFilter::Transform({2, 0}, &set, {"a", "b", "c"}) == "\"c\" = '1' AND \"a\" IS NULL");
}

TEST_CASE("Multi-column filters are rejected", "[filter]") {
	TableFilterSet set;
	set.PushMultiColumnFilter(make_uniq<ExpressionFilter>(
	    ColCmp(ExpressionType::COMPARE_EQUAL, 1), vector<ProjectionIndex> {ProjectionIndex(0), ProjectionIndex(1)}));
	REQUIRE_THROWS_AS(TeradataFilter::Transform({0}, &set, {"c"}), NotImplementedException);
	REQUIRE_THROWS_AS(TableFilterClause(set, {"c"}, {ColumnIndex(0)}, "DELETE"), NotImplementedException);
}

TEST_CASE("DELETE/UPDATE WHERE clauses come from the scan filters", "[filter]") {
	auto set = FiltersOn(0, ColCmp(ExpressionType::COMPARE_EQUAL, 7));
	const auto clause = TableFilterClause(set, {"id", "name"}, {ColumnIndex(0), ColumnIndex(1)}, "DELETE");
	CHECK(clause == "(id = 7)");

	TableFilterSet empty;
	CHECK(TableFilterClause(empty, {"id"}, {ColumnIndex(0)}, "DELETE").empty());

	auto rowid_filter = FiltersOn(0, ColCmp(ExpressionType::COMPARE_EQUAL, 7));
	REQUIRE_THROWS_AS(TableFilterClause(rowid_filter, {"id"}, {ColumnIndex(COLUMN_IDENTIFIER_ROW_ID)}, "DELETE"),
	                  NotImplementedException);
}

} // namespace duckdb
