#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cocos2d.h>
#if defined(GEODE_IS_MACOS)
#include <OpenGL/gl.h>
#elif defined(GEODE_IS_IOS)
#include <OpenGLES/ES2/gl.h>
#endif

ImRect& getGDWindowRect();
bool& shouldPassEventsToGDButTransformed();
bool& shouldUpdateGDRenderBuffer();

class GLRenderCtx final {
private:
    GLuint m_buffer = 0;
    GLuint m_texture = 0;
    GLuint m_depthStencil = 0;
    ImVec2 m_size;

    GLint m_prevDrawBuffer = 0;
    GLint m_prevReadBuffer = 0;

    void cleanup();

public:
    GLRenderCtx(ImVec2 const& size);
    ~GLRenderCtx();

    ImTextureID texture() const;
    ImVec2 size() const;

    bool begin();
    void end();
};
