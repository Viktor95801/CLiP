// Ignore all the warnings. you're compiling C code with a C++ compiler
// I suck at cpp. i tried my best :sob:

#define CLIP_IMPLEMENTATION
#include "../clip.hpp"

int main(int argc, char **argv) {
    ClipCtx ctx(argc, argv);

    auto name = ctx.Option<char*>({
        .name = "Name",
        .short_name = 'n',
        .desc = "Your name",
    });

    auto res = ctx.Parse();
    if(strlen(res.err) > 0) {
        fprintf(stderr, "ERROR: %s\n", res.err);
        return 1;
    }

    if(*name) {
        printf("Hello, %s!\n", *name);
    } else {
        printf("Hello, World!\n");
    }

    return 0;
}
