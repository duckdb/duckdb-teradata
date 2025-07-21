# duckdb-teradata

This is a DuckDB extension for connecting to and "attach":ing Teradata databases as if they were part of the DuckDB catalog. It allows you to manipulate tables, query data, and execute raw SQL commands directly on the Teradata database using DuckDB, by either pushing part of queries down into Teradata or by pulling data up into DuckDB for further processing.

## Table of Contents
<!-- TOC -->
* [Usage](#usage)
  * [Attaching to Teradata](#attaching-to-teradata)
    * [Using Secrets](#using-secrets)
    * [Parameter Precedence](#parameter-precedence)
  * [Querying data](#querying-data)
    * [Sending raw SQL queries to Teradata](#sending-raw-sql-queries-to-teradata)
    * [Executing raw SQL commands](#executing-raw-sql-commands)
  * [Type mapping](#type-mapping)
  * [Configuration Options](#configuration-options)
* [Building the Extension](#building-the-extension)
<!-- TOC -->

# Usage

## Attaching to Teradata

Once the extension is loaded, you can attach DuckDB to a teradata system by executing the following SQL:

```sql
ATTACH
'{logon string}' as {database name} (TYPE TERADATA);
```

This will mount the database corresponding to the username specified in the terdata "logon string" (on the form "
host/username,password"). To attach to a specific teradata database, provide the optional `DATABASE '{name}'` parameter
to the `ATTACH` command:

```sql
ATTACH
'{logon string}' as {database name} (TYPE TERADATA, DATABASE '{teradata database name}');
```

Here's a complete example connecting to a teradata instance running on localhost as the `dbc` user (with password `dbc`)
and attaching the database `my_db` as `td` from within duckdb:

```sql
ATTACH
'127.0.0.1/dbc,dbc' as td (TYPE TERADATA, DATABASE 'my_db');
```

Additionally, parts of the logon string can be omitted from the string itself and instead given as parameters to the
`ATTACH` command, e.g. `HOST`, `USER`, `PASSWORD`, with the parts present in the logon string taking priority.

The following table illustrates all available `ATTACH` options when attaching to a `TYPE TERADATA` database:

| Option        | Description                                                                                                                             |
|---------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| `HOST`        | The host to connect to, e.g. `127.0.0.1` or `localhost`.                                                                                |
| `USER`        | The username to connect with, e.g. `dbc`.                                                                                               |
| `PASSWORD`    | The password to use, e.g. `dbc`.                                                                                                        |
| `DATABASE`    | The teradata database to attach, e.g. `my_db`. This is optional and defaults to the user database.                                      |
| `BUFFER_SIZE` | The size of the response buffer used to fetch data from Teradata. This can be used to tune performance. Defaults to 1MiB (1024 * 1024). | 

### Using Secrets

This extension also supports handling Teradata credentials through
DuckDBs [Secret Management](https://duckdb.org/docs/stable/configuration/secrets_manager.html) system. You can create a
temporary in-memory Teradata secret using the `CREATE SECRET` command, e.g.:

```sql
CREATE
SECRET my_secret (
	TYPE TERADATA,
	HOST '127.0.0.1',
    USER '{teradata username}',
	DATABASE '{teradata database name}',
	PASSWORD '{teradata password}'
);
```

> [!WARNING]
> You can also create a persistent secret using the `CREATE PERSISTENT SECRET` command instead, but
> note that persistent secrets in DuckDB are stored in an unencrypted binary format on disk.

You can then use the secret to provide the connection credentials when attaching to a Teradata database by using the
`ATTACH` command together with the `SECRET` keyword:

```sql
ATTACH
'' as td (TYPE TERADATA, SECRET 'my_secret');
```

### Parameter Precedence

When a secret is used, you do not need to provide a logon string, or any of the `HOST`, `USER`, `PASSWORD`, or
`DATABASE` parameters in the `ATTACH` command, as all information is gathered from the secret instead.

However, if you want to override any of the connection parameters, you can still do so by providing them in logon string
or the `ATTACH` command. Similarly, you can also exclude any of the parameters when creating the secret, in which case
they will have to be provided in the logon string or the `ATTACH` command.

Credential parts passed through the logon string takes precedence over those defined in the `ATTACH` command, and those
defined in the `ATTACH` command takes precedence over those defined in the secret.

## Querying data

Once a teradata database is attached, you can query data from its tables using standard SQL queries, e.g.:

```sql
-- Attach a teradata database using a secret
ATTACH
'' as td (TYPE TERADATA, SECRET my_secret);
   
-- Query data from a table in the teradata database
SELECT * FROM td.my_table WHERE id = 42;
```

Most standard SQL queries are supported, including `SELECT`, `INSERT`, `UPDATE`, and `DELETE`. These also support filter-pushdown for simple predicates, allowing you to filter data directly on the teradata side before it is returned to DuckDB.

However, some Teradata-specific features may not be fully supported, and not all catalog operations are currently implemented either.
Therefore, you may sometime want to send and execute a raw SQL string directly to teradata, using the `teradata_query`
function.

### Sending raw SQL queries to Teradata

The `teradata_query` function can be used to send raw SQL queries directly to the teradata database. This is useful for
executing queries that may not be fully supported by DuckDB's SQL parser or for using Teradata-specific features.

The `teradata_query` function takes two parameters:

- The name of the attached teradata database (e.g. `td`).
- The raw SQL query string to execute on the teradata database.

Example usage, executing a raw SQL query to select data from a table in the teradata database, instead of going through
DuckDB's SQL parser:

```sql
SELECT teradata_query('td', 'SEL * FROM my_table WHERE id = 42');
```

For convenience, the teradata extension will also automatically register a database-scoped macro in each attached
teradata database called `query()`, which can be used to execute raw SQL queries in a more concise way. The following
example is equivalent to the previous one, but uses the `query()` macro instead:

```sql
SELECT td.query('SEL * FROM my_table WHERE id = 42');
```

### Executing raw SQL commands

In addition to querying data, you can also execute raw SQL commands using the `teradata_execute` function. This is
useful for executing commands that do not return any results, such as `CREATE TABLE`, `DROP TABLE`, or other DDL
commands not currently implemented in the DuckDB Teradata extension.

The `teradata_execute` function works similarly to `teradata_query`, but it does not return any results. Instead, it
simply executes the command on the teradata database. It similarly takes two parameters:

- The name of the attached teradata database (e.g. `td`).
- The raw SQL command string to execute on the teradata database.

The following example shows how to use `teradata_execute` to create a new table in the teradata database, instead of
using the DuckDB `CREATE TABLE` statement:

```sql
SELECT teradata_execute('td', 'CREATE TABLE my_table (id INT, value INT)');
```

## Type mapping

The DuckDB Teradata extension attempts to automatically map Teradata data types to DuckDB data types as closely as possible when
querying data from Teradata tables or inserting from DuckDB. The following tables shows the default type mapping from Teradata types to DuckDB
types, and vice-versa. 

__Teradata -> DuckDB Type Mapping__

| Teradata Type  | DuckDB Type    | Notes                                                                   |
|----------------|----------------|-------------------------------------------------------------------------|
| `BYTEINT`      | `TINYINT`      |                                                                         |
| `SMALLINT`     | `SMALLINT`     |                                                                         |
| `INTEGER`      | `INTEGER`      |                                                                         |
| `BIGINT`       | `BIGINT`       |                                                                         |
| `DECIMAL(p,s)` | `DECIMAL(p,s)` | Always defaults to returning decimal precision 38.                      |
| `FLOAT`        | `DOUBLE`       | Teradata `FLOAT`s are 8 bytes (like DuckDB `DOUBLE`)                    |
| `VARCHAR(n)`   | `VARCHAR`      |                                                                         |
| `CHAR(n)`      | `VARCHAR`      |                                                                         |
| `CLOB(n)`      | `VARCHAR`      |                                                                         |
| `BYTE(n)`      | `BLOB`         |                                                                         |
| `BLOB(n)`      | `BLOB`         |                                                                         |
| `VARBYTE(n)`   | `BLOB`         |                                                                         |
| `DATE`         | `DATE`         |                                                                         |
| `TIMESTAMP`    | `TIMESTAMP_*`  | Mapped to the appropriate DuckDB timestamp type depending on precision. |
| `TIME(n)`      | `TIME`         | DuckDB rounds to microsecond precision                                  |
| `TIMESTAMP_TZ` | `TIMESTAMP_TZ` |                                                                         |
| `TIME_TZ`      | `TIME_TZ`      |                                                                         |
| `INTERVAL_*`   | `INTERVAL`     | Attempts to map all variants to DuckDBs single `INTERVAL` type.         |

__DuckDB -> Teradata Type Mapping__

| DuckDB Type      | Teradata Type     | Notes                                                    |
|------------------|-------------------|----------------------------------------------------------|
| `BOOLEAN`        | `BYTEINT`         |                                                          |
| `TINYINT`        | `BYTEINT`         |                                                          |
| `SMALLINT`       | `SMALLINT`        |                                                          |
| `INTEGER`        | `INTEGER`         |                                                          |
| `BIGINT`         | `BIGINT`          |                                                          |
| `DOUBLE`         | `FLOAT`           | Teradata `FLOAT`s are 8 bytes (like DuckDB `DOUBLE`)     |
| `DECIMAL(p,s)`   | `DECIMAL(p,s)`    |                                                          |
| `VARCHAR`        | `VARCHAR(64000)`  | DuckDB `VARCHAR` is mapped to Teradata `VARCHAR(64000)`. |
| `BLOB`           | `BLOB(64000)`     | DuckDB `BLOB` is mapped to Teradata `BLOB(64000)`        |
| `DATE`           | `DATE`            |                                                          |
| `TIME`           | `TIME(6)`         | DuckDB `TIME` is in microsecond precision                |
| `TIME_TZ`        | `TIME_TZ(6)`      | DuckDB `TIME_TZ` is in microsecond precision             |
| `TIMESTAMP`      | `TIMESTAMP(6)`    | DuckDB `TIMESTAMP` is in microsecond precision           |
| `TIMESTAMP_TZ`   | `TIMESTAMP_TZ(6)` | DuckDB `TIMESTAMP_TZ` is in microsecond precision        |
| `TIMESTAMP_NS`   | `TIMESTAMP(9)`    |                                                          |
| `TIMESTAMP_MS`   | `TIMESTAMP(3)`    |                                                          |
| `TIMESTAMP_SEC ` | `TIMESTAMP(0)`    |                                                          |

Because DuckDB does not support fixed-size `VARCHAR` or `BLOB` types, and Teradata does not support truly variable size types either, `VARCHAR` and `BLOB` are always mapped to the maximum size of `64000` bytes when creating or inserting into Teradata tables from within DuckDB.

Type conversion not listed in the above tables are not supported, and will result in an error if attempted.

## Configuration Options

The DuckDB Teradata extension supports the following configuration options that can be set using the `SET` command:

- `SET teradata_use_primary_index = <bool> (= true)`

This option controls whether DuckDB should automatically add a [primary index](https://docs.teradata.com/r/Enterprise_IntelliFlex_VMware/Database-Design/Primary-Index-Primary-AMP-Index-and-NoPI-Objects0) on the first column of tables created in Teradata.
This defaults to `true`, to follow the standard behavior in Teradata, but can be set to false if you want to create
Teradata tables from within DuckDB where the first column can contain duplicates.

# Building the Extension

The DuckDB Teradata extension is based on the [DuckDB Extension Template](https://github.com/duckdb/extension-template), but does not use `vcpkg` for dependency management.

To build the extension, you will need to have the following dependencies installed:
- A C++ compiler that supports C++11 or later.
- CMake 3.10 or later.
- Make or Ninja build system.

Additionally, you will need the Teradata CLIv2 Libraries installed on your system, which can be obtained from Teradata's official website. These need to be available in a location where the CMake build system can find them, e.g. by modifying the `TD_INCLUDE_DIR` and `TD_LIBRARY_DIR` variables in the `CMakeLists.txt` file to point to the correct include and library directories of the Teradata CLIv2 Libraries.

Once you have the dependencies installed, simply running `make debug` or `make release` in the root directory of the extension will build the extension in debug or release mode, and provide the following artifacts: 

- The resulting DuckDB CLI binary, `duckdb` in `build/debug/duckdb` or `build/release/duckdb`, which will have the Teradata extension statically linked built in. Can be used to quickly test the extension interactively. 

- The shared library of the extension itself, `teradata.duckdb_extension`, is placed in the `./build/debug/extension/teradata/` or `./build/release/extension/teradata` directory, depending on the build mode.

# Running tests

DuckDB provides a built-in test framework that can be used to run tests for the extension. However, the tests require a running Teradata instance to be available. Credentials to the Teradata instance needs to be provided through environment variables, that is: 
- `TD_LOGON`: the logon string to the Teradata instance, e.g. `127.0.0.1/dbc,dbc`
- `TD_DB`: the name of the Teradata database to use for testing, e.g. `my_db`.

You can set these environment variables in your shell before running the tests:

```bash
export TD_LOGON="127.0.0.1/dbc,dbc
export TD_DB="my_db"
```

After these enviornment variables are set, you can run the tests using the following command:

```bash
make test_debug
```

Or by invoking the DuckDB test runner directly:

```bash
./build/debug/test/unittest --test-dir . test/sql/teradata/
```

If you want to run the tests in release mode, use `make test` instead of `make test_debug`.