#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define INPUT_FILE "unchair.un"
#define RESULTS_FILE "parser_test_results.txt"
#define OUTPUT_FILE "parser_current_output.txt"
#define PARSER_COMMAND "./main"

#define VALUE_SIZE 1024
#define LINE_SIZE 2048

typedef struct
{
    const char *name;
    const char *input;
    int should_succeed;
    const char *expected_config;
} TestCase;

typedef struct
{
    char key[256];
    char value[VALUE_SIZE];
} ParsedConfigEntry;

#define ACCEPT(name, input, config_str) \
    {name, input, 1, config_str}

#define REJECT(name, input) \
    {name, input, 0, NULL}

static const TestCase tests[] = {
    ACCEPT("01 - Basic single property config",
           "[\"work_minutes\" = 35]\n",
           "work_minutes:35|break_minutes:0|min_work_minutes:0|max_work_minutes:0|adjust_step:0|snooze_minutes:0|max_snoozes:0|water_minutes:0|song_seconds:0|song_order:(null)"),

    ACCEPT("02 - Standard full configuration",
           "[\n"
           "    \"work_minutes\" = 35,\n"
           "    \"break_minutes\" = 5,\n"
           "    \"min_work_minutes\" = 25,\n"
           "    \"max_work_minutes\" = 50,\n"
           "    \"adjust_step\" = 5,\n"
           "    \"snooze_minutes\" = 5,\n"
           "    \"max_snoozes\" = 2,\n"
           "    \"water_minutes\" = 60,\n"
           "    \"song_seconds\" = 30,\n"
           "    \"song_order\" = \"rotate\"\n"
           "]\n",
           "work_minutes:35|break_minutes:5|min_work_minutes:25|max_work_minutes:50|adjust_step:5|snooze_minutes:5|max_snoozes:2|water_minutes:60|song_seconds:30|song_order:rotate"),

    ACCEPT("03 - Compact single line config",
           "[\"work_minutes\" = 35, \"break_minutes\" = 5, \"song_order\" = \"shuffle\"]\n",
           "work_minutes:35|break_minutes:5|min_work_minutes:0|max_work_minutes:0|adjust_step:0|snooze_minutes:0|max_snoozes:0|water_minutes:0|song_seconds:0|song_order:shuffle"),

    // Test 04 expects rejection because "custom_setting" is not a recognized keyword in your `keys` array
    REJECT("04 - Custom property name with integer",
           "[\"custom_setting\" = 100]\n"),

    REJECT("05 - Missing opening bracket",
           "\"work_minutes\" = 35]\n"),

    REJECT("06 - Missing closing bracket",
           "[\"work_minutes\" = 35\n"),

    REJECT("07 - Missing equals sign",
           "[\"work_minutes\" 35]\n"),

    REJECT("08 - Missing value after equals",
           "[\"work_minutes\" = ]\n"),

    REJECT("09 - Missing comma between assignments",
           "[\"work_minutes\" = 35 \"break_minutes\" = 5]\n"),

    // Test 10 expects rejection for trailing comma based on your parser's strict checks
    REJECT("10 - Trailing comma with nothing after",
           "[\"work_minutes\" = 35,]\n"),

    REJECT("11 - Type mismatch: assigning string to integer field",
           "[\"work_minutes\" = \"thirty\"]\n"),

    REJECT("12 - Type mismatch: assigning integer to string field",
           "[\"song_order\" = 123]\n"),

    REJECT("13 - Unknown token or syntax error",
           "[\"work_minutes\" == 35]\n"),

    // Test 14 expects rejection for empty config blocks
    REJECT("14 - Empty configuration block",
           "[]\n")};

static int write_test_input(const char *input)
{
    FILE *fp = fopen(INPUT_FILE, "w");

    if (fp == NULL)
    {
        perror("Could not open input file");
        return 0;
    }

    if (fputs(input, fp) == EOF)
    {
        perror("Could not write test input");
        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0)
    {
        perror("Could not close input file");
        return 0;
    }

    return 1;
}

static void remove_newline(char *text)
{
    text[strcspn(text, "\r\n")] = '\0';
}

static int read_actual_config(
    ParsedConfigEntry entries[],
    size_t capacity,
    size_t *count)
{
    FILE *fp = fopen(OUTPUT_FILE, "r");

    if (fp == NULL)
    {
        perror("Could not read parser output");
        return 0;
    }

    char line[LINE_SIZE];
    *count = 0;
    int header_found = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        remove_newline(line);

        if (strstr(line, "--- Timer Configuration ---") != NULL)
        {
            header_found = 1;
            continue;
        }

        if (!header_found)
            continue;

        char *eq = strchr(line, '=');
        if (eq == NULL)
            continue;

        *eq = '\0';
        char *k = line;
        char *v = eq + 1;

        while (*k && (*k == ' ' || *k == '\t'))
            k++;
        size_t k_len = strlen(k);
        while (k_len > 0 && (k[k_len - 1] == ' ' || k[k_len - 1] == '\t'))
        {
            k[k_len - 1] = '\0';
            k_len--;
        }

        while (*v && (*v == ' ' || *v == '\t'))
            v++;
        size_t v_len = strlen(v);
        while (v_len > 0 && (v[v_len - 1] == ' ' || v[v_len - 1] == '\t'))
        {
            v[v_len - 1] = '\0';
            v_len--;
        }

        if (*count >= capacity)
        {
            fprintf(stderr, "Too many configuration fields captured.\n");
            fclose(fp);
            return 0;
        }

        // Safely copy key and value with strict bounds to prevent truncation warnings
        snprintf(entries[*count].key, sizeof(entries[*count].key), "%.*s",
                 (int)(sizeof(entries[*count].key) - 1), k);

        snprintf(entries[*count].value, sizeof(entries[*count].value), "%.*s",
                 (int)(sizeof(entries[*count].value) - 1), v);

        ++(*count);
    }

    fclose(fp);
    return header_found && (*count > 0);
}

static int compare_config_stream(
    const char *expected_config,
    const ParsedConfigEntry actual[],
    size_t actual_count,
    const char *test_name)
{
    char expected_copy[LINE_SIZE * 2];

    if (strlen(expected_config) >= sizeof(expected_copy))
    {
        fprintf(stderr, "%s: Expected config string is too long.\n", test_name);
        return 0;
    }

    strcpy(expected_copy, expected_config);

    size_t expected_count = 0;
    char *cursor = expected_copy;

    while (*cursor != '\0')
    {
        char *end = strchr(cursor, '|');

        if (end != NULL)
            *end = '\0';

        char *separator = strchr(cursor, ':');

        if (separator == NULL)
        {
            fprintf(stderr,
                    "%s: Invalid expected config format: %s\n",
                    test_name, cursor);
            return 0;
        }

        *separator = '\0';
        const char *expected_key = cursor;
        const char *expected_value = separator + 1;

        if (expected_count >= actual_count)
        {
            fprintf(stderr,
                    "FAIL %s: Property '%s' is missing from configuration output.\n"
                    "  Expected: %s=%s\n",
                    test_name, expected_key,
                    expected_key, expected_value);
            return 0;
        }

        if (strcmp(actual[expected_count].key, expected_key) != 0 ||
            strcmp(actual[expected_count].value, expected_value) != 0)
        {
            fprintf(stderr,
                    "FAIL %s: Configuration mismatch at index %zu.\n"
                    "  Expected: property='%s', value='%s'\n"
                    "  Actual:   property='%s', value='%s'\n",
                    test_name, expected_count,
                    expected_key, expected_value,
                    actual[expected_count].key,
                    actual[expected_count].value);
            return 0;
        }

        ++expected_count;

        if (end == NULL)
            break;

        cursor = end + 1;

        if (*cursor == '\0')
        {
            fprintf(stderr,
                    "%s: Expected config string has a trailing separator.\n",
                    test_name);
            return 0;
        }
    }

    if (expected_count != actual_count)
    {
        fprintf(stderr,
                "FAIL %s: Configuration property count mismatch.\n"
                "  Expected count: %zu\n"
                "  Actual count:   %zu\n",
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

    if (results == NULL)
    {
        perror("Could not create results file");
        return EXIT_FAILURE;
    }

    fprintf(results,
            "UNCHAIR PARSER TEST RESULTS\n"
            "Tests: %zu\n"
            "Accepted inputs check exact parsed configuration struct values.\n"
            "Rejected inputs check nonzero exit status.\n",
            test_count);

    fclose(results);

    for (size_t i = 0; i < test_count; ++i)
    {
        const TestCase *test = &tests[i];

        if (!write_test_input(test->input))
            return EXIT_FAILURE;

        char command[256];

        snprintf(command, sizeof(command),
                 PARSER_COMMAND " > " OUTPUT_FILE " 2>&1");

        int system_status = system(command);

        int exited_normally =
            system_status != -1 && WIFEXITED(system_status);

        int exit_code =
            exited_normally ? WEXITSTATUS(system_status) : -1;

        int succeeded = exited_normally && exit_code == 0;

        int test_passed = (succeeded == test->should_succeed);

        ParsedConfigEntry actual[32];
        size_t actual_count = 0;

        if (test_passed && test->should_succeed)
        {
            if (!read_actual_config(actual, 32, &actual_count))
            {
                test_passed = 0;
            }
            else if (!compare_config_stream(test->expected_config,
                                            actual,
                                            actual_count,
                                            test->name))
            {
                test_passed = 0;
            }
        }

        results = fopen(RESULTS_FILE, "a");

        if (results == NULL)
        {
            perror("Could not append results");
            return EXIT_FAILURE;
        }

        fprintf(results,
                "\n============================================================\n"
                "%s\n"
                "Expected: %s\n"
                "Input:\n%s"
                "Expected config string: %s\n"
                "Parser output:\n",
                test->name,
                test->should_succeed
                    ? "ACCEPT with exact parsed values"
                    : "REJECT with nonzero exit",
                test->input,
                test->expected_config != NULL
                    ? test->expected_config
                    : "(not applicable)");

        fclose(results);

        FILE *current = fopen(OUTPUT_FILE, "r");

        results = fopen(RESULTS_FILE, "a");

        if (results == NULL)
        {
            if (current != NULL)
                fclose(current);

            perror("Could not append results");
            return EXIT_FAILURE;
        }

        if (current != NULL)
        {
            char line[LINE_SIZE];

            while (fgets(line, sizeof(line), current) != NULL)
                fputs(line, results);

            fclose(current);
        }
        else
        {
            fprintf(results, "[Could not read parser output file]\n");
        }

        fprintf(results,
                "[exit code: %d] %s\n",
                exit_code,
                test_passed ? "PASS" : "FAIL");

        fclose(results);

        if (test_passed)
        {
            ++passed;
            printf("PASS %02zu - %s\n", i + 1, test->name);
        }
        else
        {
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