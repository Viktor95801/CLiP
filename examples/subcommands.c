#define CLIP_IMPLEMENTATION
#include "../clip.h"

// Subcommand: clone
int handle_clone(bool usage, int argc, char **argv) {
    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    int *depth = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_INT,
        .name = "depth",
        .short_name = 'd',
        .desc = "Create a shallow clone with a history truncated to the specified number of commits",
    });

    char **branch = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_STRING,
        .name = "branch",
        .short_name = 'b',
        .desc = "Point HEAD to the specified branch instead of default",
    });

    if(usage) {
        fprintf(stderr, "Usage: app [--verbose] clone [options] <repository>\n");
        clip__Ctx_print_options(&ctx);
        return 0;
    }

    clip_Result res = clip_Ctx_parse(&ctx);
    if (res.err[0] != '\0') {
        fprintf(stderr, "Error [clone]: %s\n", res.err);
        clip_default_Ctx_usage(&ctx);
        return 1;
    }

    // res.program_argv[0] is "clone"
    if (res.program_argc < 2) {
        fprintf(stderr, "Error: missing repository URL for clone command.\n");
        return 1;
    }

    printf("Executing 'clone':\n");
    printf("  Repository: %s\n", res.program_argv[1]);
    printf("  Depth:      %d\n", *depth);
    printf("  Branch:     %s\n", *branch ? *branch : "default");

    return 0;
}

// Subcommand: commit
int handle_commit(bool usage, int argc, char **argv) {
    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    char **message = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_STRING,
        .name = "message",
        .short_name = 'm',
        .desc = "Commit message",
    });

    bool *all = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_FLAG,
        .name = "all",
        .short_name = 'a',
        .desc = "Stage all modified files automatically",
    });

    if(usage) {
        fprintf(stderr, "Usage: app [--verbose] commit [options]\n");
        clip__Ctx_print_options(&ctx);
        return 0;
    }

    clip_Result res = clip_Ctx_parse(&ctx);
    if (res.err[0] != '\0') {
        fprintf(stderr, "Error [commit]: %s\n", res.err);
        return 1;
    }

    if (!*message || strlen(*message) == 0) {
        fprintf(stderr, "Error: commit message cannot be empty (-m / --message required).\n");
        return 1;
    }

    printf("Executing 'commit':\n");
    printf("  Message:    \"%s\"\n", *message);
    printf("  Stage all:  %s\n", *all ? "true" : "false");

    return 0;
}
int main(int argc, char **argv) {
    clip_Ctx ctx;
    clip_Ctx_init(&ctx, argc, argv, NULL);

    bool *verbose = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_FLAG,
        .name = "verbose",
        .short_name = 'v',
        .desc = "Enable global verbose logging",
    });

    // Subcommands table
    const char *subcommands[] = {"clone", "commit", "help"};
    int num_subcommands = sizeof(subcommands) / sizeof(subcommands[0]);

    // Parse global flags until one of the subcommands is reached
    clip_Result res = clip_Ctx_parseUntilMany(&ctx, num_subcommands, subcommands);

    if (res.err[0] != '\0') {
        fprintf(stderr, "Global options error: %s\n", res.err);
        return 1;
    }

    if (*verbose) {
        printf("[Global Log] Verbose mode enabled.\n");
    }

    // If no subcommand was reached
    if (res.unparsed_argc == 0) {
        fprintf(stderr, "Usage: app [--verbose] <subcommand> [options]\n");
        ctx.usage_fn(&ctx);
        fprintf(stderr, "\n");
        handle_commit(true, 0, NULL);
        fprintf(stderr, "\n");
        handle_clone(true, 0, NULL);
        return 1;
    }

    const char *subcmd = res.unparsed_argv[0];

    if (strcmp(subcmd, "clone") == 0) {
        return handle_clone(false, res.unparsed_argc, res.unparsed_argv);
    } else if (strcmp(subcmd, "commit") == 0) {
        return handle_commit(false, res.unparsed_argc, res.unparsed_argv);
    } else if (strcmp(subcmd, "help") == 0) {
        fprintf(stderr, "Usage: app [--verbose] <subcommand> [options]\n");
        ctx.usage_fn(&ctx);
        fprintf(stderr, "\n");
        handle_commit(true, 0, NULL);
        fprintf(stderr, "\n");
        handle_clone(true, 0, NULL);
        return 0;
    } else {
        fprintf(stderr, "Unknown subcommand: %s\n", subcmd);
        return 1;
    }
}
