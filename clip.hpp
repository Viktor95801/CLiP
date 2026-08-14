#ifndef CLIP_HPP
#define CLIP_HPP

extern "C" {
    #include "clip.h"
}

class ClipCtx {
public:
    ClipCtx(int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx) = nullptr);
    ~ClipCtx();

    void Init(int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx) = nullptr);
    template <typename T>
    T* Option(clip_Option opt);
    void *OptionRaw(clip_Option opt);

    clip_Result Parse();
    clip_Result ParseUntil(const char *until);
    clip_Result ParseUntilMany(int n, const char *until[]);

    clip_Ctx *Raw();
private:
    clip_Ctx m_ctx{};
};


//#define CLIP_IMPLEMENTATION
#ifdef CLIP_IMPLEMENTATION

ClipCtx::ClipCtx(int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx)) {
    Init(argc, argv, usage_fn);
}

ClipCtx::~ClipCtx() {
}

void ClipCtx::Init(int argc, char **argv, void (*usage_fn)(struct clip_Ctx *ctx)) {
    clip_Ctx_init(&this->m_ctx, argc, argv, usage_fn);
}

template <typename T>
T* ClipCtx::Option(clip_Option opt) {
    return static_cast<T*>(clip_Ctx_option(&this->m_ctx, opt));
}

void *ClipCtx::OptionRaw(clip_Option opt) {
    return clip_Ctx_option(&this->m_ctx, opt);
}

clip_Result ClipCtx::Parse() {
    return clip_Ctx_parse(&this->m_ctx);
}

clip_Result ClipCtx::ParseUntil(const char *until) {
    return clip_Ctx_parseUntil(&this->m_ctx, until);
}

clip_Result ClipCtx::ParseUntilMany(int n, const char *until[]) {
    return clip_Ctx_parseUntilMany(&this->m_ctx, n, until);
}

clip_Ctx *ClipCtx::Raw() {
    return &this->m_ctx;
}

#endif // CLIP_IMPLEMENTATION

#endif // CLIP_HPP
