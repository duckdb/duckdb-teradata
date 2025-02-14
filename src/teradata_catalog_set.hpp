#pragma once

#include "duckdb/transaction/transaction.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/common/mutex.hpp"
#include "duckdb/common/shared_ptr.hpp"

namespace duckdb {

class Catalog;
class ClientContext;
class CatalogEntry;
class TeradataSchemaEntry;

//----------------------------------------------------------------------------------------------------------------------
// TeradataCatalogSet
//----------------------------------------------------------------------------------------------------------------------
// Base class for sets of catalog entries
// Takes care of caching/reloading the entries

class TeradataCatalogSet {
public:
    explicit TeradataCatalogSet(Catalog &catalog);
    virtual ~TeradataCatalogSet() = default;

    // Scan all entries in this set
    void Scan(ClientContext &context, const std::function<void(CatalogEntry &)> &callback);

protected:
    // Needs to be implemented by subclasses: method to load the entries of this set
    virtual void LoadEntries(ClientContext &context) = 0;

    Catalog &catalog;
    unordered_map<string, unique_ptr<CatalogEntry>> entries = {};

private:
    void TryLoadEntries(ClientContext &context);
    mutex load_lock;
    bool is_loaded = false;
};

//----------------------------------------------------------------------------------------------------------------------
// TeradataInSchemaSet
//----------------------------------------------------------------------------------------------------------------------
// Base class for sets of entries within a schema
class TeradataInSchemaSet : public TeradataCatalogSet {
public:
    explicit TeradataInSchemaSet(TeradataSchemaEntry &schema);
protected:
    TeradataSchemaEntry &schema;
};

} // namespace duckdb