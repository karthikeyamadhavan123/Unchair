
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define INPUT_FILE       "unchair.un"
#define RESULTS_FILE     "lexer_test_results.txt"
#define OUTPUT_FILE      "lexer_current_output.txt"
#define LEXER_COMMAND    "./main"

#define MAX_TEST_TOKENS  256
#define VALUE_SIZE       1024
#define LINE_SIZE        2048

/*
Token type numbers from your lexer:

LBRACKET   = 0
RBRACKET   = 1
COMMA      = 2
EQUALS     = 3
DOUBLEQUOTE= 4
INTEGER    = 5
STRING     = 6
KEYWORD    = 7

Expected stream format:
"type:value|type:value|type:value"

Example:
"0:[|6:a|3:=|5:30|1:]"

Rejected cases have expected_stream = NULL.
*/

typedef struct {
    const char *name;
    const char *input;
    int should_succeed;
    const char *expected_stream;
} TestCase;

typedef struct {
    int type;
    char value[VALUE_SIZE];
} ActualToken;

#define ACCEPT(name, input, stream) \
    { name, input, 1, stream }

#define REJECT(name, input) \
    { name, input, 0, NULL }

static const TestCase tests[] = {
    ACCEPT("01 - Single integer property",
           "[\"a\" = 30]\n",
           "0:[|6:a|3:=|5:30|1:]"),

    ACCEPT("02 - Single string property",
           "[\"a\" = \"hello\"]\n",
           "0:[|6:a|3:=|6:hello|1:]"),

    ACCEPT("03 - Known property with integer",
           "[\"work_minutes\" = 355]\n",
           "0:[|7:work_minutes|3:=|5:355|1:]"),

    ACCEPT("04 - Known property with string",
           "[\"song_order\" = \"rotate\"]\n",
           "0:[|7:song_order|3:=|6:rotate|1:]"),

    ACCEPT("05 - No spaces around syntax",
           "[\"a\"=30]\n",
           "0:[|6:a|3:=|5:30|1:]"),

    ACCEPT("06 - Whitespace around brackets",
           "[ \"a\" = 30 ]\n",
           "0:[|6:a|3:=|5:30|1:]"),

    ACCEPT("07 - Two integer properties",
           "[\"a\" = 30, \"b\" = 5]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|3:=|5:5|1:]"),

    ACCEPT("08 - Integer and string values",
           "[\"a\" = 30, \"b\" = \"hello\"]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|3:=|6:hello|1:]"),

    ACCEPT("09 - Zero value",
           "[\"a\" = 0]\n",
           "0:[|6:a|3:=|5:0|1:]"),

    ACCEPT("10 - Large integer",
           "[\"a\" = 123456]\n",
           "0:[|6:a|3:=|5:123456|1:]"),

    ACCEPT("11 - Unknown but valid property name",
           "[\"custom_setting\" = 10]\n",
           "0:[|6:custom_setting|3:=|5:10|1:]"),

    ACCEPT("12 - Spaces inside strings",
           "[\"song order\" = \"rotate\"]\n",
           "0:[|6:song order|3:=|6:rotate|1:]"),

    ACCEPT("13 - Newlines and tabs between tokens",
           "[\n\t\"a\"\t=\t30\n]\n",
           "0:[|6:a|3:=|5:30|1:]"),

    ACCEPT("14 - Missing comma (parser responsibility)",
           "[\"a\" = 30 \"b\" = 5]\n",
           "0:[|6:a|3:=|5:30|6:b|3:=|5:5|1:]"),

    ACCEPT("15 - Trailing comma (parser responsibility)",
           "[\"a\" = 30,]\n",
           "0:[|6:a|3:=|5:30|2:,|1:]"),

    REJECT("16 - Digit in property name",
           "[\"work_minutes2\" = 30]\n"),

    REJECT("17 - Digit in property name",
           "[\"song30\" = \"rotate\"]\n"),

    REJECT("18 - Digit in string value",
           "[\"a\" = \"song30\"]\n"),

    REJECT("19 - Empty string",
           "[\"a\" = \"\"]\n"),

    REJECT("20 - Missing closing quote",
           "[\"a\" = \"hello]\n"),

    REJECT("21 - Unexpected text after closed string",
           "[\"a\" = \"hello\"world\"]\n"),

    REJECT("22 - At-sign inside string",
           "[\"a\" = \"hello@\"]\n"),

    REJECT("23 - Hash inside string",
           "[\"a\" = \"hello#\"]\n"),

    REJECT("24 - Negative integer",
           "[\"a\" = -30]\n"),

    REJECT("25 - Decimal number",
           "[\"a\" = 3.5]\n"),

    ACCEPT("26 - Whitespace splits digits: 3 0",
           "[\"a\" = 3 0]\n",
           "0:[|6:a|3:=|5:3|5:0|1:]"),

    ACCEPT("27 - Tab splits digits: 3<TAB>0",
           "[\"a\" = 3\t0]\n",
           "0:[|6:a|3:=|5:3|5:0|1:]"),

    REJECT("28 - Invalid character after integer",
           "[\"a\" = 30@]\n"),

    REJECT("29 - Invalid character before integer",
           "[\"a\" = @30]\n"),

    REJECT("30 - Letters attached to integer",
           "[\"a\" = 30abc]\n"),

    REJECT("31 - Multiple decimal points",
           "[\"a\" = 3..5]\n"),

    REJECT("32 - Standalone at-sign",
           "@\n"),

    REJECT("33 - Hash after integer",
           "[\"a\" = 30#]\n"),

    REJECT("34 - Colon is unsupported",
           "[\"a\" : 30]\n"),

    REJECT("35 - Semicolon is unsupported",
           "[\"a\" = 30;]\n"),

    REJECT("36 - Curly brace is unsupported",
           "{ \"a\" = 30 }\n"),

    REJECT("37 - Slash is unsupported",
           "[\"a\" = 30 / 2]\n"),

    REJECT("38 - Exclamation mark is unsupported",
           "[\"a\" = 30 !]\n"),

    ACCEPT("39 - Missing equals sign (parser responsibility)",
           "[\"a\" 30]\n",
           "0:[|6:a|5:30|1:]"),

    ACCEPT("40 - Equals without property name (parser responsibility)",
           "[= 30]\n",
           "0:[|3:=|5:30|1:]"),

    ACCEPT("41 - Missing value (parser responsibility)",
           "[\"a\" = ]\n",
           "0:[|6:a|3:=|1:]"),

    ACCEPT("42 - Property missing equals/value (parser responsibility)",
           "[\"a\" = 30, \"b\"]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|1:]"),

    ACCEPT("43 - Missing comma (parser responsibility)",
           "[\"a\" = 30 \"b\" = 5]\n",
           "0:[|6:a|3:=|5:30|6:b|3:=|5:5|1:]"),

    ACCEPT("44 - Trailing comma (parser responsibility)",
           "[\"a\" = 30,]\n",
           "0:[|6:a|3:=|5:30|2:,|1:]"),

    ACCEPT("45 - Duplicate comma (parser responsibility)",
           "[\"a\" = \"hello\",, \"b\" = 5]\n",
           "0:[|6:a|3:=|6:hello|2:,|2:,|6:b|3:=|5:5|1:]"),

    ACCEPT("46 - Missing closing bracket (parser responsibility)",
           "[\"a\" = 30\n",
           "0:[|6:a|3:=|5:30"),

    ACCEPT("47 - Missing opening bracket (parser responsibility)",
           "\"a\" = 30]\n",
           "6:a|3:=|5:30|1:]"),

    ACCEPT("48 - Duplicate property names (parser responsibility)",
           "[\"a\" = 30, \"a\" = 5]\n",
           "0:[|6:a|3:=|5:30|2:,|6:a|3:=|5:5|1:]"),

    ACCEPT("49 - Spaces before comma",
           "[ \"a\" = 30   , \"b\" = 49 ]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|3:=|5:49|1:]"),

    ACCEPT("50 - Multiple spaces split digits",
           "[\"a\" = 3   0]\n",
           "0:[|6:a|3:=|5:3|5:0|1:]"),

    ACCEPT("51 - Two integers without comma",
           "[\"a\" = 30 49]\n",
           "0:[|6:a|3:=|5:30|5:49|1:]"),

    ACCEPT("52 - Tabs around comma",
           "[\"a\" = 30\t,\t\"b\" = 49]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|3:=|5:49|1:]"),

    ACCEPT("53 - Newlines around comma",
           "[\"a\" = 30\n,\n\"b\" = 49]\n",
           "0:[|6:a|3:=|5:30|2:,|6:b|3:=|5:49|1:]"),

    REJECT("54 - Letters attached to integer (repeat)",
           "[\"a\" = 30abc]\n"),

    REJECT("55 - Invalid character after integer with spaces",
           "[\"a\" = 30 @  ]\n")
};

static int write_test_input(const char *input)
{
    FILE *fp = fopen(INPUT_FILE, "w");

    if (fp == NULL) {
        perror("Could not open input file");
        return 0;
    }

    if (fputs(input, fp) == EOF) {
        perror("Could not write test input");
        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0) {
        perror("Could not close input file");
        return 0;
    }

    return 1;
}

static void remove_newline(char *text)
{
    text[strcspn(text, "\r\n")] = '\0';
}

/*
Reads the lexer's existing debug output:

Token type is : 6
Value is a

The lexer output format does not need to change.
*/
static int read_actual_tokens(
    ActualToken tokens[],
    size_t capacity,
    size_t *count)
{
    FILE *fp = fopen(OUTPUT_FILE, "r");

    if (fp == NULL) {
        perror("Could not read lexer output");
        return 0;
    }

    char line[LINE_SIZE];
    *count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        int type;

        if (sscanf(line, "Token type is : %d", &type) != 1)
            continue;

        if (*count >= capacity) {
            fprintf(stderr, "Too many tokens in lexer output.\n");
            fclose(fp);
            return 0;
        }

        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Missing token value in lexer output.\n");
            fclose(fp);
            return 0;
        }

        remove_newline(line);

        const char *prefix = "Value is ";
        size_t prefix_len = strlen(prefix);

        if (strncmp(line, prefix, prefix_len) != 0) {
            fprintf(stderr, "Unexpected token value line: %s\n", line);
            fclose(fp);
            return 0;
        }

        tokens[*count].type = type;

        snprintf(tokens[*count].value,
                 sizeof(tokens[*count].value),
                 "%s",
                 line + prefix_len);

        ++(*count);
    }

    fclose(fp);
    return 1;
}

/*
Compares the actual tokens against the compact expected stream.

Returns 1 if every token type, value, and the total token count match.
Returns 0 on any mismatch.
*/
static int compare_token_stream(
    const char *expected_stream,
    const ActualToken actual[],
    size_t actual_count,
    const char *test_name)
{
    char expected_copy[LINE_SIZE * 2];

    if (strlen(expected_stream) >= sizeof(expected_copy)) {
        fprintf(stderr, "%s: Expected stream is too long.\n", test_name);
        return 0;
    }

    strcpy(expected_copy, expected_stream);

    size_t expected_count = 0;
    char *cursor = expected_copy;

    while (*cursor != '\0') {
        char *end = strchr(cursor, '|');

        if (end != NULL)
            *end = '\0';

        char *separator = strchr(cursor, ':');

        if (separator == NULL) {
            fprintf(stderr,
                    "%s: Invalid expected token format: %s\n",
                    test_name, cursor);
            return 0;
        }

        *separator = '\0';

        char *type_end;
        long expected_type = strtol(cursor, &type_end, 10);

        if (*cursor == '\0' || *type_end != '\0') {
            fprintf(stderr,
                    "%s: Invalid expected token type.\n",
                    test_name);
            return 0;
        }

        const char *expected_value = separator + 1;

        if (expected_count >= actual_count) {
            fprintf(stderr,
                    "FAIL %s: Token %zu is missing.\n"
                    "  Expected: type=%ld, value=\"%s\"\n",
                    test_name, expected_count,
                    expected_type, expected_value);
            return 0;
        }

        if (actual[expected_count].type != expected_type ||
            strcmp(actual[expected_count].value, expected_value) != 0) {
            fprintf(stderr,
                    "FAIL %s: Token mismatch at index %zu.\n"
                    "  Expected: type=%ld, value=\"%s\"\n"
                    "  Actual:   type=%d, value=\"%s\"\n",
                    test_name, expected_count,
                    expected_type, expected_value,
                    actual[expected_count].type,
                    actual[expected_count].value);
            return 0;
        }

        ++expected_count;

        if (end == NULL)
            break;

        cursor = end + 1;

        if (*cursor == '\0') {
            fprintf(stderr,
                    "%s: Expected stream has a trailing separator.\n",
                    test_name);
            return 0;
        }
    }

    if (expected_count != actual_count) {
        fprintf(stderr,
                "FAIL %s: Token count mismatch.\n"
                "  Expected token count: %zu\n"
                "  Actual token count:   %zu\n",
                test_name, expected_count, actual_count);
        return 0;
    }

    return 1;
}

int main(void)
{
    const size_t test_count = sizeof(tests) / sizeof(tests[0]);

    size_t passed = 0;
    size_t failed = 0;

    FILE *results = fopen(RESULTS_FILE, "w");

    if (results == NULL) {
        perror("Could not create results file");
        return EXIT_FAILURE;
    }

    fprintf(results,
            "UNCHAlR LEXER TEST RESULTS\n"
            "Tests: %zu\n"
            "Accepted inputs check exact token types, values, and count.\n"
            "Rejected inputs check nonzero exit status.\n",
            test_count);

    fclose(results);

    for (size_t i = 0; i < test_count; ++i) {
        const TestCase *test = &tests[i];

        if (!write_test_input(test->input))
            return EXIT_FAILURE;

        char command[256];

        snprintf(command, sizeof(command),
                 LEXER_COMMAND " > " OUTPUT_FILE " 2>&1");

        int system_status = system(command);

        int exited_normally =
            system_status != -1 && WIFEXITED(system_status);

        int exit_code =
            exited_normally ? WEXITSTATUS(system_status) : -1;

        int succeeded = exited_normally && exit_code == 0;

        int test_passed = (succeeded == test->should_succeed);

        ActualToken actual[MAX_TEST_TOKENS];
        size_t actual_count = 0;

        if (test_passed && test->should_succeed) {
            if (!read_actual_tokens(actual,
                                    MAX_TEST_TOKENS,
                                    &actual_count)) {
                test_passed = 0;
            } else if (!compare_token_stream(test->expected_stream,
                                             actual,
                                             actual_count,
                                             test->name)) {
                test_passed = 0;
            }
        }

        results = fopen(RESULTS_FILE, "a");

        if (results == NULL) {
            perror("Could not append results");
            return EXIT_FAILURE;
        }

        fprintf(results,
                "\n============================================================\n"
                "%s\n"
                "Expected: %s\n"
                "Input:\n%s"
                "Expected token stream: %s\n"
                "Lexer output:\n",
                test->name,
                test->should_succeed
                    ? "ACCEPT with exact token stream"
                    : "REJECT with nonzero exit",
                test->input,
                test->expected_stream != NULL
                    ? test->expected_stream
                    : "(not applicable)");

        fclose(results);

        FILE *current = fopen(OUTPUT_FILE, "r");

        results = fopen(RESULTS_FILE, "a");

        if (results == NULL) {
            if (current != NULL)
                fclose(current);

            perror("Could not append results");
            return EXIT_FAILURE;
        }

        if (current != NULL) {
            char line[LINE_SIZE];

            while (fgets(line, sizeof(line), current) != NULL)
                fputs(line, results);

            fclose(current);
        } else {
            fprintf(results, "[Could not read lexer output file]\n");
        }

        fprintf(results,
                "[exit code: %d] %s\n",
                exit_code,
                test_passed ? "PASS" : "FAIL");

        fclose(results);

        if (test_passed) {
            ++passed;
            printf("PASS %02zu - %s\n", i + 1, test->name);
        } else {
            ++failed;
            printf("FAIL %02zu - %s\n", i + 1, test->name);
        }
    }

    printf("\nSummary: %zu passed, %zu failed, %zu total.\n",
           passed, failed, test_count);

    printf("Detailed output: %s\n", RESULTS_FILE);
    printf("Last test input: %s\n", INPUT_FILE);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
