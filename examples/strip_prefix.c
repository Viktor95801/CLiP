#include <stdio.h>

#define CLIP_IMPLEMENTATION
#define CLIP_STRIP_PREFIX
#include "../clip.h"

int main(int argc, char **argv) {
    Ctx ctx;
    Ctx_init(&ctx, argc, argv, NULL);

    bool *verbose = Ctx_option(&ctx, (Option){
        .type = vtyp_FLAG,
        .name = "verbose",
        .short_name = 'v',
        .desc = "Enable verbose mode"
    });

    Result res = Ctx_parse(&ctx);
    if (res.err[0] != '\0') {
        fprintf(stderr, "Error: %s\n", res.err);
        return 1;
    }

    printf("Verbose: %s\n", *verbose ? "true" : "false");
    return 0;
}
