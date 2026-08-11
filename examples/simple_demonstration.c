#define CLIP_IMPLEMENTATION
#include "../clip.h"

void my_usage(struct clip_Ctx *ctx) {
    fprintf(stderr, "Usage: %s [options]\n", ctx->_program_argv[0]);
    clip__default_Ctx_usage(ctx);
}

int main(int argc, char **argv) {
    clip_Ctx ctx = {0};
    clip_Ctx_init(&ctx, argc, argv, my_usage);
    char **name = clip_Ctx_option(&ctx, (clip_Option){
        vtyp_STRING,
        "name",
        'n',
        "Your name, used for greeting.",
    });
    bool *goodbye = clip_Ctx_option(&ctx, (clip_Option){
        vtyp_FLAG,
        "goodbye",
        'g',
        "Whether to say goodbye.",
    });
    int *age = clip_Ctx_option(&ctx, (clip_Option){
        vtyp_INT,
        "age",
        'a',
        "Your age.",
    });
    float *height = clip_Ctx_option(&ctx, (clip_Option){
        vtyp_FLOAT,
        "height",
        'h',
        "Your height in cm.",
    });

    if (argc == 1) {
        ctx.usage_fn(&ctx);
        return 0;
    }

    clip_Result result = clip_Ctx_parse(&ctx);
    if(strlen(result.err) > 0) {
        fprintf(stderr, "ERROR: %s\n", result.err);
        return 1;
    }

    if(*name) {
        printf("Hello, %s!\n", *name);
    }

    if(*goodbye) {
        printf("Goodbye!\n");
    }

    if(*age) {
        printf("Wow... Just wow. You're %d? You're getting old.\n", *age);
    }

    if(*height) {
        printf("HAHA. Only %.2f meters tall? You're a shortie LOL.\n", *height/100.0);
    }

    for(int i = 0; i < result.program_argc; i++) {
        printf("program_arg[%d] = %s\n", i, result.program_argv[i]);
    }

    for(int i = 0; i < result.unparsed_argc; i++) {
        printf("unparsed_arg[%d] = %s\n", i, result.unparsed_argv[i]);
    }

    return 0;
}
