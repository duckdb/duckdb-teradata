# duckdb-teradata-connector
Teradata Connector

# Usage

## Attaching to Teradata

Once the extension is loaded, you can attach DuckDB to a teradata system by executing the following SQL:

```sql
ATTACH '{logon string}' as {database name} (TYPE TERADATA);
```

This will mount the database corresponding to the username specified in the terdata "logon string" (on the form "host/username,password"). To attach to a specific teradata database, provide the optional `DATABASE '{name}'` parameter to the `ATTACH` command:

```sql
ATTACH '{logon string}' as {database name} (TYPE TERADATA, DATABASE '{teradata database name}');
```

Here's a complete example connecting to a teradata instance running on localhost as the `dbc` user (with password `dbc`) and attaching the database `my_db` as `td` from within duckdb:

```sql
ATTACH '127.0.0.1/dbc,dbc' as td (TYPE TERADATA, DATABASE 'my_db');
```

Additionally, parts of the logon string can be omitted from the string itself and instead given as parameters to the `ATTACH` command, e.g. `HOST`, `USER`, `PASSWORD`, with the parts present in the logon string taking priority.

The following table illustrates all available `ATTACH` options when attaching to a `TYPE TERADATA` database:

| Option      | Description                                                                 |
|-------------|-----------------------------------------------------------------------------|
| `HOST`      | The host to connect to, e.g. `127.0.0.1` or `localhost`.                    |
| `USER`      | The username to connect with, e.g. `dbc`.                                    |
| `PASSWORD`  | The password to use, e.g. `dbc`.                                             |
| `DATABASE`  | The teradata database to attach, e.g. `my_db`. This is optional and defaults to the user database. |
| `BUFFER_SIZE` | The size of the response buffer used to fetch data from Teradata. This can be used to tune performance. Defaults to 1MiB (1024 * 1024). | 

### Secret Management

## Querying data

- teradata_query
- schema.query()

## Type mapping

## Catalog operations

## Configuration Options

`SET teradata_use_primary_index = <bool>` - Default to to true
This option sets whether DuckDB should automatically add a "primary index" on the first column of tables created in Teradata.
This defaults to true, to follow the standard behavior in Teradata, but can be set to false if you want to create tables where the first column can contain duplicates.

# Building the Extension