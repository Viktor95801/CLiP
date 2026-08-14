#ifndef CLIP_H
#define CLIP_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLIP_assert(cond, msg) assert(cond && msg)

#define CLIP__STRINGIFICATE_(x) CLIP__STRINGIFICATE(x)
#define CLIP__STRINGIFICATE(x) #x
#define CLIP__LINE CLIP__STRINGIFICATE(__LINE__)

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

    clip_Value *_handler; // internal variable for handling the option's value
} clip_Option;

typedef struct {
    char err[CLIP_ERROR_LEN];
    int unparsed_argi; // index of the first unparsed argument
    int unparsed_argc;
    char **unparsed_argv; // stuff after the parseUntil delimeter
    int program_argc;
    char **program_argv; // the program's arguments (e.g. files to a compiler in `cc main.c clip.c`). program_argv[0] is the program's name or subcommand.
} clip_Result;

//TODO: dynamic _optv and stuff instead of fixed-size arrays. maybe pass in an allocator/freer function
typedef struct clip_Ctx {
    int _argc;
    char **_argv;

    void (*usage_fn)(struct clip_Ctx *ctx);
    int _optc;
    clip_Option _optv[CLIP_MAX_OPTIONS];
    clip_Value _values[CLIP_MAX_OPTIONS];
    int _program_argc;
    const char *_program_argv[CLIP_MAX_PROGRAM_ARGS];
} clip_Ctx;

const char *clip__type_hint(clip_Value_Type type);
void clip__Ctx_print_options(clip_Ctx *ctx);
void clip_default_Ctx_usage(clip_Ctx *ctx);

void clip_Ctx_init(clip_Ctx *ctx, int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx));
void *clip_Ctx_option(clip_Ctx *ctx, clip_Option opt);

// Wraps `clip_parseUntil` with "--" as `until`.
clip_Result clip_Ctx_parse(clip_Ctx *ctx);
// Dis but a mere wrapper around `clip_parseUntilMany` with a single `until` string.
clip_Result clip_Ctx_parseUntil(clip_Ctx *ctx, const char *until);
// Stops parsing at the first occurrence of any of the `until` strings.
clip_Result clip_Ctx_parseUntilMany(clip_Ctx *ctx, int n, const char *until[]);

#endif // CLIP_H

//#define CLIP_IMPLEMENTATION
#ifdef CLIP_IMPLEMENTATION

const char *clip__type_hint(clip_Value_Type type) {
    switch (type) {
    case vtyp_INT:    return " <INT>";
    case vtyp_FLOAT:  return " <FLOAT>";
    case vtyp_STRING: return " <STR>";
    case vtyp_FLAG:   return "";
    }
    return "";
}

void clip__Ctx_print_options(clip_Ctx *ctx) {
    int max_len = 0;
    for (int i = 0; i < ctx->_optc; ++i) {
        clip_Option opt = ctx->_optv[i];
        int len = 4; // base indent "  --"
        len += strlen(opt.name);

        if (opt.short_name != 0) {
            len += 4; // ", -X"
        }

        len += strlen(clip__type_hint(opt.type));

        if (len > max_len) {
            max_len = len;
        }
    }

    for (int i = 0; i < ctx->_optc; ++i) {
        clip_Option opt = ctx->_optv[i];
        char left_buf[256];

        if (opt.short_name != 0) {
            snprintf(left_buf, sizeof(left_buf), "  --%s, -%c%s",
                     opt.name, opt.short_name, clip__type_hint(opt.type));
        } else {
            snprintf(left_buf, sizeof(left_buf), "  --%s%s",
                     opt.name, clip__type_hint(opt.type));
        }

        fprintf(stderr, "%-*s  %s\n", max_len, left_buf, opt.desc ? opt.desc : "");
    }
}

void clip_default_Ctx_usage(clip_Ctx *ctx) {
    const char *prog_name = (ctx->_argc > 0 && ctx->_argv[0]) ? ctx->_argv[0] : "program";
    fprintf(stderr, "Usage: %s [OPTIONS]\n\n", prog_name);
    fprintf(stderr, "Options:\n");
    clip__Ctx_print_options(ctx);
}

void clip_Ctx_init(clip_Ctx *ctx, int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx)) {
    memset(ctx, 0, sizeof(clip_Ctx));
    ctx->_argc = argc;
    ctx->_argv = argv;
    if(usage_fn == NULL) {
        ctx->usage_fn = clip_default_Ctx_usage;
    } else {
        ctx->usage_fn = usage_fn;
    }
}

void *clip_Ctx_option(clip_Ctx *ctx, clip_Option opt) {
    CLIP_assert(ctx->_optc + 1 <= CLIP_MAX_OPTIONS, "CLIP_MAX_OPTIONS is too small");
    CLIP_assert(opt.name != NULL && strlen(opt.name) > 0, "option name is empty");
    ctx->_optv[ctx->_optc] = opt;
    ctx->_values[ctx->_optc] = (clip_Value){
        .type = opt.type,
        .as = {0},
    };
    ctx->_optv[ctx->_optc]._handler = &ctx->_values[ctx->_optc];
    return &ctx->_values[ctx->_optc++].as;
}

inline clip_Result clip_Ctx_parse(clip_Ctx *ctx) {
    return clip_Ctx_parseUntil(ctx, "--");
}

inline clip_Result clip_Ctx_parseUntil(clip_Ctx *ctx, const char *until) {
    return clip_Ctx_parseUntilMany(ctx, 1, &until);
}

#define CLIP__parseArg(arg) do {                                                                   \
    switch(opt->type) {                                                                            \
    case vtyp_FLAG: CLIP_assert(0, ("UNREACHABLE: " __FILE__ ":" CLIP__LINE));                     \
    case vtyp_STRING:                                                                              \
        opt->_handler->as.string = (arg);                                                          \
        break;                                                                                     \
    case vtyp_INT:                                                                                 \
        if(sscanf((arg), "%d", &opt->_handler->as.integer) != 1) {                                 \
            clip_Result res = {0};                                                                 \
            snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires an integer value", opt->name); \
            return res;                                                                            \
        }                                                                                          \
        break;                                                                                     \
    case vtyp_FLOAT:                                                                               \
        if(sscanf((arg), "%f", &opt->_handler->as.floating) != 1) {                                \
            clip_Result res = {0};                                                                 \
            snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a float value", opt->name);    \
            return res;                                                                            \
        }                                                                                          \
        break;                                                                                     \
    }                                                                                              \
} while(0)
#define CLIP__findShortOpt(i) do {             \
    for(int i = 0; i < ctx->_optc; ++i) {      \
        if(ctx->_optv[i].short_name == *arg) { \
            opt = &ctx->_optv[i];              \
            break;                             \
        }                                      \
    }                                          \
} while(0)
clip_Result clip_Ctx_parseUntilMany(clip_Ctx *ctx, int n, const char *until[]) {
    int i = 0;
    for(char *arg = ctx->_argv[i]; i < ctx->_argc; ++i, arg = ctx->_argv[i]) {
        for(int j = 0; j < n; ++j) {
            if(strcmp(arg, until[j]) == 0) {
                goto clip_Ctx_parseUntilMany_DONE;
            }
        }

        if(*arg != '-') {
            ctx->_program_argv[ctx->_program_argc++] = arg;
            continue;
        }
        ++arg;

        if(*arg != '-') {
            clip_Option *opt = NULL;
            CLIP__findShortOpt(j);
            if(opt == NULL) {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "unknown short option: %s", ctx->_argv[i]);
                return res;
            }

            if(opt->type == vtyp_FLAG) {
                while(*arg != 0) {
                    if(*arg == '=' || *arg == ':') {
                        clip_Result res = {0};
                        snprintf(res.err, CLIP_ERROR_LEN, "option '%s' received an unexpected value (flags don't take values): %s", opt->name, ctx->_argv[i]);
                        return res;
                    }
                    CLIP__findShortOpt(k);
                    if(opt == NULL) {
                        clip_Result res = {0};
                        snprintf(res.err, CLIP_ERROR_LEN, "unknown short option: %s", ctx->_argv[i]);
                        return res;
                    }
                    if(opt->type != vtyp_FLAG) {
                        clip_Result res = {0};
                        snprintf(res.err, CLIP_ERROR_LEN, "option '%s' is not a flag: %s", opt->name, ctx->_argv[i]);
                        return res;
                    }
                    opt->_handler->as.flag = true;
                    ++arg;
                }
                continue;
            }

            char *arg_value = arg + 1;
            if(strlen(arg) <= 1 && ctx->_argc > i + 1) {
                arg_value = ctx->_argv[i + 1];
                ++i;
            } else if(strlen(arg) > 1 && (arg[1] == '=' || arg[1] == ':')) {
                ++arg_value;
            } else if(strlen(arg) <= 1) {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a value: %s", opt->name, ctx->_argv[i]);
                return res;
            }
            CLIP__parseArg(arg_value);
            continue;
        }
        ++arg;

        size_t arg_len = 0;
        char *arg_assign = NULL;
        if((arg_assign = strchr(arg, '=')) != NULL || (arg_assign = strchr(arg, ':')) != NULL) {
            arg_len = arg_assign - arg;
        } else {
            arg_len = strlen(arg);
        }

        if(arg_len <= 0) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "missing name for long option '%s'", ctx->_argv[i]);
            return res;
        }

        clip_Option *opt = NULL;
        for(int j = 0; j < ctx->_optc; ++j) {
            if(strlen(ctx->_optv[j].name) == arg_len && strncmp(arg, ctx->_optv[j].name, arg_len) == 0) {
                opt = &ctx->_optv[j];
                break;
            }
        }
        if(opt == NULL) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "unknown long option '%s'", ctx->_argv[i]);
            return res;
        }

        if(opt->type == vtyp_FLAG) {
            if(strlen(arg) > arg_len) {
                clip_Result res = {0};
                snprintf(res.err, CLIP_ERROR_LEN, "option '%s' received an unexpected value (flags don't take values): %s", opt->name, ctx->_argv[i]);
                return res;
            }
            opt->_handler->as.flag = true;
            continue;
        }

        char *arg_value = NULL;
        if(arg_assign != NULL) {
            arg_value = arg_assign + 1;
        } else if(arg_assign == NULL && ctx->_argc > i + 1) {
            arg_value = ctx->_argv[++i];
        } else if(arg_assign == NULL) {
            clip_Result res = {0};
            snprintf(res.err, CLIP_ERROR_LEN, "option '%s' requires a value: %s", opt->name, ctx->_argv[i]);
            return res;
        }
        CLIP__parseArg(arg_value);
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
#undef CLIP__parseArg

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
