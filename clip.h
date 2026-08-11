#ifndef CLIP_H
#define CLIP_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLIP_ERROR_LEN
#define CLIP_ERROR_LEN 512
#endif

#ifndef CLIP_MAX_OPTIONS
#define CLIP_MAX_OPTIONS 64
#endif

#ifndef CLIP_MAX_PROGRAM_ARGS
#define CLIP_MAX_PROGRAM_ARGS 64
#endif

typedef enum {
    vtyp_STRING,
    vtyp_INT,
    vtyp_FLOAT,
    vtyp_FLAG,
} clip_Value_Type;

typedef struct {
    clip_Value_Type type;
    union {
        char *string;
        int integer;
        float floating;
        bool flag;
    } as;
} clip_Value;

typedef struct {
    clip_Value_Type type;
    const char *name;
    char short_name;
    const char *desc;
    const char *short_desc;

    clip_Value *_handler;
} clip_Option;

typedef struct {
    char err[CLIP_ERROR_LEN];
    int unparsed_argi; // index of the first unparsed argument
    int unparsed_argc;
    char **unparsed_argv; // stuff after the parseUntil delimeter
    int program_argc;
    char **program_argv; // the program's arguments (e.g. files to a compiler in `cc main.c clip.c`). program_argv[0] is the program's name or subcommand.
} clip_Result;

typedef struct clip_Ctx {
    int _argc;
    char **_argv;
    int _argi;

    void (*usage_fn)(struct clip_Ctx *ctx);
    int _optc;
    clip_Option _optv[CLIP_MAX_OPTIONS];
    clip_Value _values[CLIP_MAX_OPTIONS];
    int _program_argc;
    const char *_program_argv[CLIP_MAX_PROGRAM_ARGS];
} clip_Ctx;
void clip__default_Ctx_usage(clip_Ctx *ctx);

bool clip_Ctx_init(clip_Ctx *ctx, int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx));
void *clip_Ctx_option(clip_Ctx *ctx, clip_Option opt);

// Wraps `clip_parseUntil` with "--" as `until`.
clip_Result clip_Ctx_parse(clip_Ctx *ctx);
// Dis but a mere wrapper around `clip_parseUntilMany` with a single `until` string.
clip_Result clip_Ctx_parseUntil(clip_Ctx *ctx, const char *until);
// Stops parsing at the first occurrence of any of the `until` strings.
clip_Result clip_Ctx_parseUntilMany(clip_Ctx *ctx, int n, const char *until[]);

#ifdef __cplusplus
}
#endif

//#define CLIP_IMPLEMENTATION
#ifdef CLIP_IMPLEMENTATION

void clip__default_Ctx_usage(struct clip_Ctx *ctx) {
    int maxlen = 0;
    for (int i = 0; i < ctx->_optc; ++i) {
        int len = strlen(ctx->_optv[i].name) + 2;
        if (ctx->_optv[i].short_name != '\0') {
            len += 3; // for ", X"
        }
        if (len > maxlen) maxlen = len;
    }

    for(int i = 0; i < ctx->_optc; ++i) {
        clip_Option opt = ctx->_optv[i];
        fprintf(stderr, "  --%s", opt.name);
        if(opt.short_name != '\0') {
            fprintf(stderr, ", -%c", opt.short_name);
        }
        int printed_len = (opt.short_name != '\0')
            ? (int)strlen(opt.name) + 5  // "name, x"
            : (int)strlen(opt.name) + 2;
        int padding = maxlen - printed_len;
        fprintf(stderr, "%-*s", padding, "\0");
        if(opt.short_desc != NULL) {
            fprintf(stderr, " : %s\n", opt.short_desc);
        } else {
            fprintf(stderr, " : %s\n", opt.desc);
        }
    }
}

bool clip_Ctx_init(clip_Ctx *ctx, int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx)) {
    memset(ctx, 0, sizeof(clip_Ctx));
    ctx->_argc = argc;
    ctx->_argv = argv;
    if(usage_fn == NULL) {
        ctx->usage_fn = clip__default_Ctx_usage;
    } else {
        ctx->usage_fn = usage_fn;
    }
    return true;
}

void *clip_Ctx_option(clip_Ctx *ctx, clip_Option opt) {
    assert(ctx->_optc + 1 <= CLIP_MAX_OPTIONS);
    ctx->_optv[ctx->_optc] = opt;
    ctx->_values[ctx->_optc] = (clip_Value){
        .type = opt.type,
        .as = {0},
    };
    ctx->_optv[ctx->_optc]._handler = &ctx->_values[ctx->_optc];
    return &ctx->_values[ctx->_optc++].as;
}

clip_Result clip_Ctx_parse(clip_Ctx *ctx) {
    return clip_Ctx_parseUntil(ctx, "--");
}

clip_Result clip_Ctx_parseUntil(clip_Ctx *ctx, const char *until) {
    return clip_Ctx_parseUntilMany(ctx, 1, &until);
}

clip_Result clip_Ctx_parseUntilMany(clip_Ctx *ctx, int n, const char *until[]) {
    int i = 0;
    for(char *arg = ctx->_argv[i]; i < ctx->_argc; ++i, arg = ctx->_argv[i]) {
        for(int j = 0; j < n; ++j) {
            if(strcmp(arg, until[j]) == 0) {
                goto clip_Ctx_parseUntilMany_DONE;
            }
        }

        if(arg[0] != '-') {
            ctx->_program_argv[ctx->_program_argc++] = arg;
            continue;
        }

        if(arg[1] != '-') {
            clip_Option *opt = NULL;
            for(int j = 0; j < ctx->_optc; ++j) {
                if(ctx->_optv[j].short_name == arg[1]) {
                    opt = &ctx->_optv[j];
                    break;
                }
            }
            if(opt == NULL) {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "unknown short option: %s", arg);
                return res;
            }

            if(opt->type == vtyp_FLAG) {
                if(strlen(arg) > 2) {
                    clip_Result res = {0};
                    snprintf(res.err, CLIP_ERROR_LEN, "option '%s' received an unexpected value (flags don't take values): %s", opt->name, arg);
                    return res;
                }
                opt->_handler->as.flag = true;
                continue;
            }

            if(arg[2] != '=' && arg[2] != ':') {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a value", opt->name);
                return res;
            }
            switch(opt->type) {
            case vtyp_FLAG: break;
            case vtyp_STRING:
                opt->_handler->as.string = arg + 3;
                break;
            case vtyp_INT:
                if(sscanf(arg + 3, "%d", &opt->_handler->as.integer) != 1) {
                    clip_Result res = {0};
                    snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires an integer value", opt->name);
                    return res;
                }
                break;
            case vtyp_FLOAT:
                if(sscanf(arg + 3, "%f", &opt->_handler->as.floating) != 1) {
                    clip_Result res = {0};
                    snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a float value", opt->name);
                    return res;
                }
                break;
            }
            continue;
        }

        int arg_len = 0;
        char *arg_assign = NULL;
        if((arg_assign = strchr(arg + 2, '=')) != NULL || (arg_assign = strchr(arg + 2, ':')) != NULL) {
            arg_len = arg_assign - arg;
        } else {
            arg_len = strlen(arg);
        }

        struct {
            char *name;
            int len;
        } arg_info = {
            .name = arg + 2,
            .len = arg_len - 2,
        };

        if(arg_info.len <= 0) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "missing name for long option '%s'", arg);
            return res;
        }

        clip_Option *opt = NULL;
        for(int j = 0; j < ctx->_optc; ++j) {
            if(strlen(ctx->_optv[j].name) == arg_info.len && strncmp(arg_info.name, ctx->_optv[j].name, arg_info.len) == 0) {
                opt = &ctx->_optv[j];
                break;
            }
        }
        if(opt == NULL) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "unknown long option '%s'", arg);
            return res;
        }

        if(opt->type == vtyp_FLAG) {
            if(strlen(arg) > arg_len + 2) {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "option '%s' received an unexpected value (flags don't take values): %s", opt->name, arg);
                return res;
            }
            opt->_handler->as.flag = true;
            continue;
        }

        if(arg_assign == NULL) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a value", opt->name);
            return res;
        }
        switch(opt->type) {
            case vtyp_STRING:
                opt->_handler->as.string = arg_assign+1;
                break;
            case vtyp_FLOAT:
                if(sscanf(arg_assign + 1, "%f", &opt->_handler->as.floating) != 1) {
                    clip_Result res = {0};
                    snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a float value", opt->name);
                    return res;
                }
                break;
            case vtyp_INT:
                if(sscanf(arg_assign + 1, "%d", &opt->_handler->as.integer) != 1) {
                    clip_Result res = {0};
                    snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires an integer value", opt->name);
                    return res;
                }
                break;
            case vtyp_FLAG:
                break;
        }
    }

clip_Ctx_parseUntilMany_DONE:
    return (clip_Result){
        .unparsed_argi = i,
        .unparsed_argc = ctx->_argc - i,
        .unparsed_argv = &ctx->_argv[i],
        .program_argc = ctx->_program_argc,
        .program_argv = (char**)ctx->_program_argv,
    };
}

#endif // CLIP_IMPLEMENTATION

#ifdef CLIP_STRIP_PREFIX

// Types
#define Value_Type clip_Value_Type
#define Value clip_Value
#define Option clip_Option
#define Result clip_Result
#define Ctx clip_Ctx

// Functions
#define Ctx_init clip_Ctx_init
#define Ctx_option clip_Ctx_option
#define Ctx_parse clip_Ctx_parse
#define Ctx_parseUntil clip_Ctx_parseUntil
#define Ctx_parseUntilMany clip_Ctx_parseUntilMany

#endif // CLIP_STRIP_PREFIX

#endif // CLIP_H
