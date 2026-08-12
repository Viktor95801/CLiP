#define CLIP_IMPLEMENTATION
#include "../clip.h"

void my_usage(struct clip_Ctx *ctx) {
    fprintf(stderr, "Usage: %s [options]\n", ctx->_program_argv[0]);
    clip_default_Ctx_usage(ctx);
}

int main(int argc, char **argv) {
    clip_Ctx ctx = {0};
    clip_Ctx_init(&ctx, argc, argv, my_usage);
    char **name = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_STRING,
        .name = "name",
        .short_name = 'n',
        .desc = "Your name, used for greeting.",
    });
    bool *goodbye = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_FLAG,
        .name = "goodbye",
        .short_name = 'g',
        .desc = "Whether to say goodbye.",
    });
    bool *compliment = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_FLAG,
        .name = "compliment",
        .short_name = 'c',
        .desc = "Whether to compliment the user.",
    });
    int *age = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_INT,
        .name = "age",
        .short_name = 'a',
        .desc = "Your age.",
    });
    float *height = clip_Ctx_option(&ctx, (clip_Option){
        .type = vtyp_FLOAT,
        .name = "height",
        .short_name = 'h',
        .desc = "Your height in cm.",
    });

    clip_Result result = clip_Ctx_parse(&ctx);
    if(strlen(result.err) > 0) {
        fprintf(stderr, "ERROR: %s\n", result.err);
        return 1;
    }

    if (argc == 1) {
        ctx.usage_fn(&ctx);
        return 0;
    }

    if(*name) {
        printf("Hello, %s!\n", *name);
    }

    if(*goodbye) {
        printf("Goodbye!\n");
    }

    if(*compliment) {
        printf("Nah, you ugly af.\n");
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
