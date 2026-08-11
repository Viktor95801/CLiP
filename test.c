// AI generated tests. im gonna test by hand... eventually

#define CLIP_IMPLEMENTATION
#include "clip.h"

// Helper macro for clean output
#define RUN_TEST(fn) do { \
    printf("Running %s...", #fn); \
    fn(); \
    printf(" PASSED\n"); \
} while(0)

// 1. Valid Flag and Positional Argument Interleaving
static void test_flags_and_positionals(void) {
    char *argv[] = {"app", "pos1", "-v", "pos2", "--debug", "pos3"};
    int argc = 6;

    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    bool *v = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "verbose", .short_name = 'v'});
    bool *d = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "debug", .short_name = 'd'});

    clip_Result res = clip_Ctx_parse(&ctx);

    assert(res.err[0] == '\0');
    assert(*v == true);
    assert(*d == true);
    assert(res.program_argc == 4); // "app", "pos1", "pos2", "pos3"
    assert(strcmp(res.program_argv[1], "pos1") == 0);
    assert(strcmp(res.program_argv[2], "pos2") == 0);
    assert(strcmp(res.program_argv[3], "pos3") == 0);
}

// 2. Short option syntax delimiters (= and :)
static void test_short_option_delimiters(void) {
    char *argv[] = {"app", "-c=42", "-f:3.14", "-s=hello"};
    int argc = 4;

    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    int *count = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_INT, .name = "count", .short_name = 'c'});
    float *factor = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLOAT, .name = "factor", .short_name = 'f'});
    char **str = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_STRING, .name = "str", .short_name = 's'});

    clip_Result res = clip_Ctx_parse(&ctx);

    assert(res.err[0] == '\0');
    assert(*count == 42);
    assert(*factor > 3.13f && *factor < 3.15f);
    assert(strcmp(*str, "hello") == 0);
}

// 3. Edge Case: Unexpected value assigned to a FLAG (e.g. -v=123 or --verbose=true)
static void test_flag_with_unexpected_value(void) {
    // Long flag with unexpected assignment
    {
        char *argv[] = {"app", "--verbose=true"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "verbose", .short_name = 'v'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "unexpected value") != NULL);
    }

    // Short flag with unexpected length (e.g., -v extra chars)
    {
        char *argv[] = {"app", "-vextra"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "verbose", .short_name = 'v'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "unexpected value") != NULL);
    }
}

// 4. Edge Case: Missing required value for INT/FLOAT/STRING options
static void test_missing_values(void) {
    // Long option without '=' delimiter
    {
        char *argv[] = {"app", "--count"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_INT, .name = "count", .short_name = 'c'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "requires a value") != NULL);
    }

    // Short option without value assignment delimiter
    {
        char *argv[] = {"app", "-c"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_INT, .name = "count", .short_name = 'c'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "requires a value") != NULL);
    }
}

// 5. Edge Case: Invalid Numeric Parsing (sscanf failure)
static void test_invalid_type_conversion(void) {
    // Invalid Integer
    {
        char *argv[] = {"app", "--count=not_a_number"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_INT, .name = "count", .short_name = 'c'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "requires an integer value") != NULL);
    }

    // Invalid Float
    {
        char *argv[] = {"app", "-f:abc"};
        clip_Ctx ctx;
        clip_Ctx_init(&ctx, 2, argv, NULL);
        clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLOAT, .name = "float", .short_name = 'f'});

        clip_Result res = clip_Ctx_parse(&ctx);
        assert(res.err[0] != '\0');
        assert(strstr(res.err, "requires a float value") != NULL);
    }
}

// 6. Edge Case: Double Dash Delimiter (`--`) Handling
static void test_double_dash_delimiter(void) {
    char *argv[] = {"app", "--verbose", "--", "--not-an-option", "-c"};
    int argc = 5;

    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    bool *verbose = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "verbose", .short_name = 'v'});

    clip_Result res = clip_Ctx_parse(&ctx); // Stops at "--" by default

    assert(res.err[0] == '\0');
    assert(*verbose == true);
    assert(res.unparsed_argc == 3); // "--", "--not-an-option", "-c"
    assert(strcmp(res.unparsed_argv[0], "--") == 0);
    assert(strcmp(res.unparsed_argv[1], "--not-an-option") == 0);
    assert(strcmp(res.unparsed_argv[2], "-c") == 0);
}

// 7. Edge Case: Empty or Only Executable Path in Argv
static void test_minimal_argv(void) {
    char *argv[] = {"myprogram"};
    clip_Ctx ctx;
    clip_Ctx_init(&ctx, 1, argv, NULL);

    bool *v = clip_Ctx_option(&ctx, (clip_Option){.type = vtyp_FLAG, .name = "verbose", .short_name = 'v'});

    clip_Result res = clip_Ctx_parse(&ctx);

    assert(res.err[0] == '\0');
    assert(*v == false);
    assert(res.program_argc == 1);
    assert(strcmp(res.program_argv[0], "myprogram") == 0);
    assert(res.unparsed_argc == 0);
}

// 8. Edge Case: Empty Long Option (`--=val` or `--`)
static void test_malformed_long_options(void) {
    char *argv[] = {"app", "--=value"};
    clip_Ctx ctx;
    clip_Ctx_init(&ctx, 2, argv, NULL);

    clip_Result res = clip_Ctx_parseUntilMany(&ctx, 0, NULL);

    assert(res.err[0] != '\0');
    assert(strstr(res.err, "missing name for long option") != NULL);
}

int main(void) {
    printf("=== Starting CLIP test suite ===\n");

    RUN_TEST(test_flags_and_positionals);
    RUN_TEST(test_short_option_delimiters);
    RUN_TEST(test_flag_with_unexpected_value);
    RUN_TEST(test_missing_values);
    RUN_TEST(test_invalid_type_conversion);
    RUN_TEST(test_double_dash_delimiter);
    RUN_TEST(test_minimal_argv);
    RUN_TEST(test_malformed_long_options);

    printf("=== All tests executed successfully! ===\n");
    return 0;
}
