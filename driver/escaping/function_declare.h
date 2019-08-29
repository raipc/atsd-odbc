
// https://docs.faircom.com/doc/sqlref/33391.htm
// https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/appendix-e-scalar-functions

// clang-format off

    // Numeric
    DECLARE2(ABS, "abs"),
    DECLARE2(ACOS, "acos"),
    DECLARE2(ASIN, "asin"),
    DECLARE2(ATAN, "atan"),
    // DECLARE2(ATAN2, ""),
    DECLARE2(CEILING, "ceil"),
    DECLARE2(COS, "cos"),
    // DECLARE2(COT, ""),
    // DECLARE2(DEGREES, ""),
    DECLARE2(EXP, "exp"),
    DECLARE2(FLOOR, "floor"),
    DECLARE2(LOG, "log"),
    DECLARE2(LOG10, "log10"),
    DECLARE2(MOD, "modulo"),
    DECLARE2(PI, "pi"),
    DECLARE2(POWER, "power"),
    // DECLARE2(RADIANS, ""),
    DECLARE2(RAND, "rand"),
    DECLARE2(ROUND, "round"),
    // DECLARE2(SIGN, ""),
    DECLARE2(SIN, "sin"),
    DECLARE2(SQRT, "sqrt"),
    DECLARE2(TAN, "tan"),
    DECLARE2(TRUNCATE, "trunc"),

    // String
    // ASCII
    // BIT_LENGTH
    // CHAR
    DECLARE2(CHAR_LENGTH, "lengthUTF8"),
    DECLARE2(CHARACTER_LENGTH, "lengthUTF8"),
    DECLARE2(CONCAT, "concat"),
    // DIFFERENCE
    // INSERT
    DECLARE2(LCASE, "lowerUTF8"),
    DECLARE2(LOWER, "lowerUTF8"),
    // LEFT  substring(s, 0, length)
    DECLARE2(LENGTH, "lengthUTF8"),
    //DECLARE2(LOCATE, "position"), // special
    // LTRIM
    DECLARE2(OCTET_LENGTH, "length"),
    // POSITION
    // REPEAT
    DECLARE2(REPLACE, "replaceAll"),
    // RIGHT
    // RTRIM
    // SOUNDEX
    // SPACE
    DECLARE2(SUBSTRING, "substringUTF8"),
    DECLARE2(UCASE, "upperUTF8"),
    DECLARE2(UPPER, "upperUTF8"),


    // Date
    DECLARE2(CURDATE, "today"),
    DECLARE2(CURRENT_DATE, "today"),
    DECLARE2(DAYOFMONTH, "DayOfMonth"),
    DECLARE2(DAYOFWEEK, "DayOfWeek"),
    DECLARE2(DAYOFYEAR, " DayOfYear"), 
    DECLARE2(EXTRACT, "EXTRACT"), // Do not touch extract inside {fn ... }
    DECLARE2(TIMESTAMPADD, "dateAdd"),
    //Consider deleting all of the below functions
    DECLARE2(HOUR, "Hour"),
    DECLARE2(MINUTE, "Minute"),
    DECLARE2(MONTH, "Month"),
    DECLARE2(NOW, "now"),
    DECLARE2(SECOND, "Second"),
    DECLARE2(TIMESTAMPDIFF, "dateDiff"),
    DECLARE2(WEEK, "WeekOfYear"),
    DECLARE2(SQL_TSI_QUARTER, "Quarter"),
    DECLARE2(YEAR, "Year"),

    // DECLARE2(DATABASE, ""),
    DECLARE2(IFNULL, "ifNull"),
    // DECLARE2(USER, ""),

    // TODO.
