/*
 *      ===~----------------------------------------------~===
 *  ----| 10DAL - The "10 Dollars A License" license. (v3.0) |----
 *      ===~----------------------------------------------~===
 *  Copyright (c) 2026 Viktor Hugo
 *  _______
 *  Summary
 *  -------
 *  Do whatever you want, be a nice person.
 *  ___________
 *  Definitions
 *  -----------
 *  1. "The Software" means any version of this software and associated files.
 *  2. "Military Purposes" means direct use of the The Software in
 *  the planning, execution or support of military operations.
 *  3. "Harmful Intent" means deliberate use of The Software with the aim of causing
 *  significant physical, moral or mental harm* to people or property.
 *  * Examples of "harm" include: harassment, anything that may hurt a person
 *      physically - including torture, human trafficking, genocide,
 *      creation of weapons, etc.
 *  __________
 *  Permission
 *  ----------
 *  The Software is licensed under the 10DAL license, which is
 *  subject to the conditions below, permission is granted, free of charge, to any
 *  person obtaining a copy of the The Software, to - but not limited to, even if you are
 *  Richard M. StallMan - use the The Software, to modify the The Software, to distribute the
 *  The Software, to sell the The Software, and/or to relicense the The Software.
 *  _________________________________________________________________________
 *  Optional Good-Practice Suggestions to Become a Better Human (non-binding)
 *  -------------------------------------------------------------------------
 *  1. Be kind to people. Donate money, give gifts, help them.
 *  2. Sign for volunteer work and/or donate 10 dollars monthly to your
 *  local charity.
 *  3. Do NOT waste anything that comes from natural non-renewable resources.
 *  4. Be tolerant. Everything in humanity, including the device, be it physical or
 *  digital, that you are using to read this right now, came from cooperation.
 *  5. Do NOT insult other people without a prior reason to it.
 *  6. Study upon topics you consider yourself ignorant about.
 *  7. If you use the The Software with Harmful Intent or with Military Purposes,
 *  you are specially encouraged to make donations and volunteer for people
 *  in your area.
 *  ___________________
 *  Warranty Disclaimer
 *  -------------------
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

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
