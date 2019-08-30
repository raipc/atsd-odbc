#include <Poco/NumberFormatter.h>
#include "connection.h"
#include "log/log.h"
#include "utils.h"
#include "win/version.h"

#if (ODBCVER >= 0x0380)
#    define SQL_DRIVER_AWARE_POOLING_CAPABLE 0x00000001L
#endif /* ODBCVER >= 0x0300 */

#define SQL_DRIVER_AWARE_POOLING_SUPPORTED 10024

#if (ODBCVER >= 0x0380)
#    define SQL_ASYNC_NOTIFICATION 10025
// Possible values for SQL_ASYNC_NOTIFICATION
#    define SQL_ASYNC_NOTIFICATION_NOT_CAPABLE 0x00000000L
#    define SQL_ASYNC_NOTIFICATION_CAPABLE 0x00000001L
#endif // ODBCVER >= 0x0380

#if defined(UNICODE)
#   define DRIVER_FILE_NAME "ATSDODBCW.DLL"
#else
#   define DRIVER_FILE_NAME "ATSDODBC.DLL"
#endif

extern "C" {

RETCODE SQL_API SQLGetInfo(
    HDBC connection_handle, SQLUSMALLINT info_type, PTR out_value, SQLSMALLINT out_value_max_length, SQLSMALLINT * out_value_length) {
    LOG(__FUNCTION__);
    LOG("GetInfo with info_type: " << info_type << ", out_value_max_length: " << out_value_max_length);

#if defined(UNICODE)
#    define CASE_STRING(NAME, VALUE)             \
        case NAME:                               \
            if ((out_value_max_length % 2) != 0) \
                return SQL_ERROR;                \
            return fillOutputPlatformString(VALUE, out_value, out_value_max_length, out_value_length);
#else
#    define CASE_STRING(NAME, VALUE) \
        case NAME:                   \
            return fillOutputPlatformString(VALUE, out_value, out_value_max_length, out_value_length);
#endif

    /** How are all these values selected?
      * Part of them provides true information about the capabilities of the DBMS.
      * But in most cases, the possibilities are declared "in reserve" to see,
      * what requests will be sent and what any software will do, meaning these features.
      */

    return doWith<Connection>(connection_handle, [&](Connection & connection) -> RETCODE {
        const char * name = nullptr;

        const auto mask_SQL_CONVERT_VARCHAR = SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT |
#if defined(SQL_CVT_GUID)
            SQL_CVT_GUID |
#endif
            SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT
            | SQL_CVT_INTEGER /*| SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME*/ | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR
            | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP | SQL_CVT_TINYINT | SQL_CVT_VARBINARY
            | SQL_CVT_VARCHAR;

        switch (info_type) {
            CASE_STRING(SQL_DRIVER_VER, VERSION_STRING)
            CASE_STRING(SQL_DRIVER_ODBC_VER, "03.80")
            CASE_STRING(SQL_DM_VER, "03.80.0000.0000")
            CASE_STRING(SQL_DRIVER_NAME, DRIVER_FILE_NAME)
            CASE_STRING(SQL_DBMS_NAME, "ATSD")
            CASE_STRING(SQL_DBMS_VER, "01.00.0000")
            CASE_STRING(SQL_SERVER_NAME, connection.server)
            CASE_STRING(SQL_DATA_SOURCE_NAME, connection.data_source)
            CASE_STRING(SQL_CATALOG_TERM, "catalog")
            CASE_STRING(SQL_COLLATION_SEQ, "UTF-8")
            CASE_STRING(SQL_DATABASE_NAME, connection.getDatabase())
            CASE_STRING(SQL_KEYWORDS, "AND,AS,ASC,BETWEEN,BY,CASE,CAST,CURRENTROW,DENSE_RANK,DESC,ELSE,ESCAPE,FROM,GROUP,HAVING,IN,INNER,INTERPOLATE,ISNULL,JOIN,LAG,LAST_TIME,LEAD,LIKE,LIMIT,LOOKUP,NOT,OFFSET,OPTION,OR,ORDER,OUTER,PERIOD,PRECEDING,RANK,REGEX,ROW_NUMBER,SELECT,THEN,USING,VALUE,WHEN,WHERE,WITH")
            CASE_STRING(SQL_PROCEDURE_TERM, "stored procedure")
            CASE_STRING(SQL_CATALOG_NAME_SEPARATOR, ".")
            CASE_STRING(SQL_IDENTIFIER_QUOTE_CHAR, "\"")
            CASE_STRING(SQL_SEARCH_PATTERN_ESCAPE, "\\")
            CASE_STRING(SQL_SCHEMA_TERM, "schema")
            CASE_STRING(SQL_TABLE_TERM, "table")
            CASE_STRING(SQL_SPECIAL_CHARACTERS, "")
            CASE_STRING(SQL_USER_NAME, connection.user)
            CASE_STRING(SQL_XOPEN_CLI_YEAR, "2015")

            CASE_FALLTHROUGH(SQL_DATA_SOURCE_READ_ONLY)
            CASE_FALLTHROUGH(SQL_ACCESSIBLE_PROCEDURES)
            CASE_FALLTHROUGH(SQL_ACCESSIBLE_TABLES)
            CASE_FALLTHROUGH(SQL_CATALOG_NAME)
            CASE_FALLTHROUGH(SQL_EXPRESSIONS_IN_ORDERBY)
            CASE_FALLTHROUGH(SQL_LIKE_ESCAPE_CLAUSE)
            CASE_FALLTHROUGH(SQL_MULTIPLE_ACTIVE_TXN)
            CASE_STRING(SQL_COLUMN_ALIAS, "Y")

            CASE_FALLTHROUGH(SQL_ORDER_BY_COLUMNS_IN_SELECT)
            CASE_FALLTHROUGH(SQL_INTEGRITY)
            CASE_FALLTHROUGH(SQL_MAX_ROW_SIZE_INCLUDES_LONG)
            CASE_FALLTHROUGH(SQL_MULT_RESULT_SETS)
            CASE_FALLTHROUGH(SQL_NEED_LONG_DATA_LEN)
            CASE_FALLTHROUGH(SQL_PROCEDURES)
            CASE_FALLTHROUGH(SQL_ROW_UPDATES)
            CASE_STRING(SQL_DESCRIBE_PARAMETER, "N")

            /// UINTEGER single values
            CASE_NUM(SQL_ODBC_INTERFACE_CONFORMANCE, SQLUINTEGER, SQL_OIC_CORE)
            CASE_NUM(SQL_ASYNC_MODE, SQLUINTEGER, SQL_AM_NONE)
#if defined(SQL_ASYNC_NOTIFICATION)
            CASE_NUM(SQL_ASYNC_NOTIFICATION, SQLUINTEGER, SQL_ASYNC_NOTIFICATION_NOT_CAPABLE)
#endif
            CASE_NUM(SQL_DEFAULT_TXN_ISOLATION, SQLUINTEGER, SQL_TXN_SERIALIZABLE)
#if defined(SQL_DRIVER_AWARE_POOLING_CAPABLE)
            CASE_NUM(SQL_DRIVER_AWARE_POOLING_SUPPORTED, SQLUINTEGER, SQL_DRIVER_AWARE_POOLING_CAPABLE)
#endif
            CASE_NUM(SQL_PARAM_ARRAY_ROW_COUNTS, SQLUINTEGER, SQL_PARC_NO_BATCH)
            CASE_NUM(SQL_PARAM_ARRAY_SELECTS, SQLUINTEGER, SQL_PAS_NO_SELECT)
            CASE_NUM(SQL_SQL_CONFORMANCE, SQLUINTEGER, SQL_SC_SQL92_ENTRY)

            /// USMALLINT single values
            CASE_NUM(SQL_ODBC_API_CONFORMANCE, SQLSMALLINT, SQL_OAC_LEVEL1);
            CASE_NUM(SQL_ODBC_SQL_CONFORMANCE, SQLSMALLINT, SQL_OSC_CORE);
            CASE_NUM(SQL_GROUP_BY, SQLUSMALLINT, SQL_GB_GROUP_BY_CONTAINS_SELECT)
            CASE_NUM(SQL_CATALOG_LOCATION, SQLUSMALLINT, SQL_CL_START)
            CASE_NUM(SQL_FILE_USAGE, SQLUSMALLINT, SQL_FILE_NOT_SUPPORTED)
            CASE_NUM(SQL_IDENTIFIER_CASE, SQLUSMALLINT, SQL_IC_SENSITIVE)
            CASE_NUM(SQL_QUOTED_IDENTIFIER_CASE, SQLUSMALLINT, SQL_IC_SENSITIVE)
            CASE_NUM(SQL_CONCAT_NULL_BEHAVIOR, SQLUSMALLINT, SQL_CB_NULL)
            CASE_NUM(SQL_CORRELATION_NAME, SQLUSMALLINT, SQL_CN_ANY)
            CASE_FALLTHROUGH(SQL_CURSOR_COMMIT_BEHAVIOR)
            CASE_NUM(SQL_CURSOR_ROLLBACK_BEHAVIOR, SQLUSMALLINT, SQL_CB_PRESERVE)
            CASE_NUM(SQL_CURSOR_SENSITIVITY, SQLUSMALLINT, SQL_INSENSITIVE)
            CASE_NUM(SQL_NON_NULLABLE_COLUMNS, SQLUSMALLINT, SQL_NNC_NON_NULL)
            CASE_NUM(SQL_NULL_COLLATION, SQLUSMALLINT, SQL_NC_END)
            CASE_NUM(SQL_TXN_CAPABLE, SQLUSMALLINT, SQL_TC_NONE)

            /// UINTEGER non-empty bitmasks
            CASE_NUM(SQL_CATALOG_USAGE, SQLUINTEGER, SQL_CU_DML_STATEMENTS | SQL_CU_TABLE_DEFINITION)
            CASE_NUM(SQL_AGGREGATE_FUNCTIONS,
                SQLUINTEGER,
                SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT /*| SQL_AF_DISTINCT*/ | SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM)
            CASE_NUM(SQL_ALTER_TABLE,
                SQLUINTEGER,
                SQL_AT_ADD_COLUMN_DEFAULT | SQL_AT_ADD_COLUMN_SINGLE | SQL_AT_DROP_COLUMN_DEFAULT | SQL_AT_SET_COLUMN_DEFAULT)
            CASE_NUM(SQL_CONVERT_FUNCTIONS, SQLUINTEGER, SQL_FN_CVT_CAST /*| SQL_FN_CVT_CONVERT*/)
            CASE_NUM(SQL_CREATE_TABLE, SQLUINTEGER, SQL_CT_CREATE_TABLE)
            CASE_NUM(SQL_CREATE_VIEW, SQLUINTEGER, SQL_CV_CREATE_VIEW)
            CASE_NUM(SQL_DROP_TABLE, SQLUINTEGER, SQL_DT_DROP_TABLE)
            CASE_NUM(SQL_DROP_VIEW, SQLUINTEGER, SQL_DV_DROP_VIEW)
            CASE_NUM(SQL_DATETIME_LITERALS, SQLUINTEGER, SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIMESTAMP)
            CASE_NUM(SQL_GETDATA_EXTENSIONS, SQLUINTEGER, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER | SQL_GD_BOUND)
            CASE_NUM(SQL_INDEX_KEYWORDS, SQLUINTEGER, SQL_IK_NONE)
            CASE_NUM(SQL_INSERT_STATEMENT, SQLUINTEGER, SQL_IS_INSERT_LITERALS)
            CASE_NUM(SQL_SCROLL_OPTIONS, SQLUINTEGER, SQL_SO_FORWARD_ONLY)
            CASE_NUM(SQL_SQL92_DATETIME_FUNCTIONS, SQLUINTEGER, SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIME | SQL_SDF_CURRENT_TIMESTAMP)

            CASE_FALLTHROUGH(SQL_CONVERT_BIGINT)
            CASE_FALLTHROUGH(SQL_CONVERT_BINARY)
            CASE_FALLTHROUGH(SQL_CONVERT_BIT)
            CASE_FALLTHROUGH(SQL_CONVERT_CHAR)
#if defined(SQL_CONVERT_GUID)
            CASE_FALLTHROUGH(SQL_CONVERT_GUID)
#endif
            CASE_FALLTHROUGH(SQL_CONVERT_DATE)
            CASE_FALLTHROUGH(SQL_CONVERT_DECIMAL)
            CASE_FALLTHROUGH(SQL_CONVERT_DOUBLE)
            CASE_FALLTHROUGH(SQL_CONVERT_FLOAT)
            CASE_FALLTHROUGH(SQL_CONVERT_INTEGER)
            CASE_FALLTHROUGH(SQL_CONVERT_INTERVAL_YEAR_MONTH)
            CASE_FALLTHROUGH(SQL_CONVERT_INTERVAL_DAY_TIME)
            CASE_FALLTHROUGH(SQL_CONVERT_LONGVARBINARY)
            CASE_FALLTHROUGH(SQL_CONVERT_LONGVARCHAR)
            CASE_FALLTHROUGH(SQL_CONVERT_NUMERIC)
            CASE_FALLTHROUGH(SQL_CONVERT_REAL)
            CASE_FALLTHROUGH(SQL_CONVERT_SMALLINT)
            CASE_FALLTHROUGH(SQL_CONVERT_TIME)
            CASE_FALLTHROUGH(SQL_CONVERT_TIMESTAMP)
            CASE_FALLTHROUGH(SQL_CONVERT_TINYINT)
            CASE_FALLTHROUGH(SQL_CONVERT_VARBINARY)

            CASE_NUM(SQL_CONVERT_VARCHAR, SQLUINTEGER, /*mask_SQL_CONVERT_VARCHAR*/ 0)

            CASE_NUM(SQL_NUMERIC_FUNCTIONS,
                SQLUINTEGER,
                SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING
                    | SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_DEGREES | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG
                    | SQL_FN_NUM_LOG10 | SQL_FN_NUM_MOD | SQL_FN_NUM_PI | SQL_FN_NUM_POWER | SQL_FN_NUM_RADIANS | SQL_FN_NUM_RAND
                    | SQL_FN_NUM_ROUND | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_TRUNCATE)

            CASE_NUM(SQL_OJ_CAPABILITIES,
                SQLUINTEGER,
                SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_INNER | SQL_OJ_FULL | SQL_OJ_NESTED | SQL_OJ_NOT_ORDERED | SQL_OJ_ALL_COMPARISON_OPS)

            CASE_NUM(SQL_SQL92_NUMERIC_VALUE_FUNCTIONS,
                SQLUINTEGER,
                SQL_SNVF_BIT_LENGTH | SQL_SNVF_CHAR_LENGTH | SQL_SNVF_CHARACTER_LENGTH | SQL_SNVF_EXTRACT | SQL_SNVF_OCTET_LENGTH
                    | SQL_SNVF_POSITION)

            CASE_NUM(SQL_SQL92_PREDICATES,
                SQLUINTEGER,
                SQL_SP_BETWEEN | SQL_SP_COMPARISON | SQL_SP_EXISTS | SQL_SP_IN | SQL_SP_ISNOTNULL | SQL_SP_ISNULL | SQL_SP_LIKE
                    | SQL_SP_MATCH_FULL | SQL_SP_MATCH_PARTIAL | SQL_SP_MATCH_UNIQUE_FULL | SQL_SP_MATCH_UNIQUE_PARTIAL | SQL_SP_OVERLAPS
                    | SQL_SP_QUANTIFIED_COMPARISON | SQL_SP_UNIQUE)

            CASE_NUM(SQL_SQL92_RELATIONAL_JOIN_OPERATORS,
                SQLUINTEGER,
                /*SQL_SRJO_CORRESPONDING_CLAUSE |*/ SQL_SRJO_CROSS_JOIN | /*SQL_SRJO_EXCEPT_JOIN |*/ SQL_SRJO_FULL_OUTER_JOIN
                    | SQL_SRJO_INNER_JOIN /*| SQL_SRJO_INTERSECT_JOIN |*/
                    /*SQL_SRJO_LEFT_OUTER_JOIN |*/ /*SQL_SRJO_NATURAL_JOIN |*/ /*SQL_SRJO_RIGHT_OUTER_JOIN*/ /*| SQL_SRJO_UNION_JOIN*/)

            CASE_NUM(SQL_SQL92_ROW_VALUE_CONSTRUCTOR,
                SQLUINTEGER,
                SQL_SRVC_VALUE_EXPRESSION | SQL_SRVC_NULL | SQL_SRVC_DEFAULT | SQL_SRVC_ROW_SUBQUERY)

            CASE_NUM(SQL_SQL92_STRING_FUNCTIONS,
                SQLUINTEGER,
                SQL_SSF_CONVERT | SQL_SSF_LOWER | SQL_SSF_UPPER | SQL_SSF_SUBSTRING | SQL_SSF_TRANSLATE | SQL_SSF_TRIM_BOTH
                    | SQL_SSF_TRIM_LEADING | SQL_SSF_TRIM_TRAILING)

            CASE_NUM(SQL_SQL92_VALUE_EXPRESSIONS, SQLUINTEGER, SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE | SQL_SVE_NULLIF)

            CASE_NUM(SQL_STANDARD_CLI_CONFORMANCE, SQLUINTEGER, SQL_SCC_XOPEN_CLI_VERSION1 | SQL_SCC_ISO92_CLI)

            CASE_NUM(SQL_STRING_FUNCTIONS,
                SQLUINTEGER,
                SQL_FN_STR_ASCII | SQL_FN_STR_BIT_LENGTH | SQL_FN_STR_CHAR | SQL_FN_STR_CHAR_LENGTH | SQL_FN_STR_CHARACTER_LENGTH
                    | SQL_FN_STR_CONCAT | SQL_FN_STR_DIFFERENCE | SQL_FN_STR_INSERT | SQL_FN_STR_LCASE | SQL_FN_STR_LEFT | SQL_FN_STR_LENGTH
                    | SQL_FN_STR_LOCATE | SQL_FN_STR_LTRIM | SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION | SQL_FN_STR_REPEAT
                    | SQL_FN_STR_REPLACE | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE
                    | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE)

            CASE_NUM(SQL_SUBQUERIES,
                SQLUINTEGER,
                /*SQL_SQ_CORRELATED_SUBQUERIES |*/ SQL_SQ_COMPARISON | SQL_SQ_EXISTS | SQL_SQ_IN | SQL_SQ_QUANTIFIED)

            CASE_NUM(SQL_TIMEDATE_ADD_INTERVALS,
                SQLUINTEGER,
                /*SQL_FN_TSI_FRAC_SECOND |*/ SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                    | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR)

            /*CASE_NUM(SQL_TIMEDATE_DIFF_INTERVALS,
                SQLUINTEGER,
                SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                    | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR)*/

            CASE_NUM(SQL_TIMEDATE_FUNCTIONS,
                SQLUINTEGER,
                SQL_FN_TD_CURRENT_DATE | SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | SQL_FN_TD_CURDATE | SQL_FN_TD_CURTIME
                    | SQL_FN_TD_DAYNAME | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_EXTRACT
                    | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME | SQL_FN_TD_NOW | SQL_FN_TD_QUARTER
                    | SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD | /*SQL_FN_TD_TIMESTAMPDIFF |*/ SQL_FN_TD_WEEK | SQL_FN_TD_YEAR)

            CASE_NUM(SQL_TXN_ISOLATION_OPTION, SQLUINTEGER, SQL_TXN_SERIALIZABLE)

            //CASE_NUM(SQL_UNION, SQLUINTEGER, SQL_U_UNION | SQL_U_UNION_ALL)

            // UINTEGER empty bitmasks
	    CASE_FALLTHROUGH(SQL_UNION) //UNION is not available in ATSD
            CASE_FALLTHROUGH(SQL_ALTER_DOMAIN)
            CASE_FALLTHROUGH(SQL_BATCH_ROW_COUNT)
            CASE_FALLTHROUGH(SQL_BATCH_SUPPORT)
            CASE_FALLTHROUGH(SQL_BOOKMARK_PERSISTENCE)
            CASE_FALLTHROUGH(SQL_CREATE_ASSERTION)
            CASE_FALLTHROUGH(SQL_CREATE_CHARACTER_SET)
            CASE_FALLTHROUGH(SQL_CREATE_COLLATION)
            CASE_FALLTHROUGH(SQL_CREATE_DOMAIN)
            CASE_FALLTHROUGH(SQL_CREATE_SCHEMA)
            CASE_FALLTHROUGH(SQL_CREATE_TRANSLATION)
            CASE_FALLTHROUGH(SQL_DROP_ASSERTION)
            CASE_FALLTHROUGH(SQL_DROP_CHARACTER_SET)
            CASE_FALLTHROUGH(SQL_DROP_COLLATION)
            CASE_FALLTHROUGH(SQL_DROP_DOMAIN)
            CASE_FALLTHROUGH(SQL_DROP_SCHEMA)
            CASE_FALLTHROUGH(SQL_DROP_TRANSLATION)
            CASE_FALLTHROUGH(SQL_DYNAMIC_CURSOR_ATTRIBUTES1)
            CASE_FALLTHROUGH(SQL_DYNAMIC_CURSOR_ATTRIBUTES2)
            CASE_FALLTHROUGH(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1)
            CASE_FALLTHROUGH(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2)
            CASE_FALLTHROUGH(SQL_KEYSET_CURSOR_ATTRIBUTES1)
            CASE_FALLTHROUGH(SQL_KEYSET_CURSOR_ATTRIBUTES2)
            CASE_FALLTHROUGH(SQL_STATIC_CURSOR_ATTRIBUTES1)
            CASE_FALLTHROUGH(SQL_STATIC_CURSOR_ATTRIBUTES2)
            CASE_FALLTHROUGH(SQL_INFO_SCHEMA_VIEWS)
            CASE_FALLTHROUGH(SQL_POS_OPERATIONS)
            CASE_FALLTHROUGH(SQL_SCHEMA_USAGE)
            CASE_FALLTHROUGH(SQL_SYSTEM_FUNCTIONS)
            CASE_FALLTHROUGH(SQL_SQL92_FOREIGN_KEY_DELETE_RULE)
            CASE_FALLTHROUGH(SQL_SQL92_FOREIGN_KEY_UPDATE_RULE)
            CASE_FALLTHROUGH(SQL_SQL92_GRANT)
            CASE_FALLTHROUGH(SQL_SQL92_REVOKE)
            CASE_FALLTHROUGH(SQL_STATIC_SENSITIVITY)
            CASE_FALLTHROUGH(SQL_LOCK_TYPES)
            CASE_FALLTHROUGH(SQL_SCROLL_CONCURRENCY)
            CASE_NUM(SQL_DDL_INDEX, SQLUINTEGER, 0)

            /// Limits on the maximum number, USMALLINT.
            CASE_FALLTHROUGH(SQL_ACTIVE_ENVIRONMENTS)
            CASE_FALLTHROUGH(SQL_MAX_COLUMNS_IN_GROUP_BY)
            CASE_FALLTHROUGH(SQL_MAX_COLUMNS_IN_INDEX)
            CASE_FALLTHROUGH(SQL_MAX_COLUMNS_IN_ORDER_BY)
            CASE_FALLTHROUGH(SQL_MAX_COLUMNS_IN_SELECT)
            CASE_FALLTHROUGH(SQL_MAX_COLUMNS_IN_TABLE)
            CASE_FALLTHROUGH(SQL_MAX_CONCURRENT_ACTIVITIES)
            CASE_FALLTHROUGH(SQL_MAX_DRIVER_CONNECTIONS)
            CASE_FALLTHROUGH(SQL_MAX_IDENTIFIER_LEN)
            CASE_FALLTHROUGH(SQL_MAX_PROCEDURE_NAME_LEN)
            CASE_FALLTHROUGH(SQL_MAX_TABLES_IN_SELECT)
            CASE_FALLTHROUGH(SQL_MAX_USER_NAME_LEN)
            CASE_FALLTHROUGH(SQL_MAX_COLUMN_NAME_LEN)
            CASE_FALLTHROUGH(SQL_MAX_CURSOR_NAME_LEN)
            CASE_FALLTHROUGH(SQL_MAX_SCHEMA_NAME_LEN)
            CASE_FALLTHROUGH(SQL_MAX_TABLE_NAME_LEN)
            CASE_NUM(SQL_MAX_CATALOG_NAME_LEN, SQLUSMALLINT, 0)

            /// Limitations on the maximum number, UINTEGER.
            CASE_FALLTHROUGH(SQL_MAX_ROW_SIZE)
            CASE_FALLTHROUGH(SQL_MAX_STATEMENT_LEN)
            CASE_FALLTHROUGH(SQL_MAX_BINARY_LITERAL_LEN)
            CASE_FALLTHROUGH(SQL_MAX_CHAR_LITERAL_LEN)
            CASE_FALLTHROUGH(SQL_MAX_INDEX_SIZE)
            CASE_NUM(SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, SQLUINTEGER, 0)

#if defined(SQL_ASYNC_DBC_FUNCTIONS)
            CASE_NUM(SQL_ASYNC_DBC_FUNCTIONS, SQLUINTEGER, 0)
#endif

            default:
                throw std::runtime_error("Unsupported info type: " + std::to_string(info_type));
        }
    });

#undef CASE_STRING
}
}
