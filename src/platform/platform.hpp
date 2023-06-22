#pragma once

#include <imgui.h>
#include <imgui_internal.h>

ImRect& getGDWindowRect();
bool& shouldPassEventsToGDButTransformed();
bool& shouldUpdateGDRenderBuffer();

class GLRenderCtx final {
private:
    GLuint m_buffer  = 0;
    GLuint m_texture = 0;
    GLuint m_depth   = 0;
    ImVec2 m_size;

    void cleanup();

public:
    GLRenderCtx(ImVec2 const& size);
    ~GLRenderCtx();

    ImTextureID texture() const;
    ImVec2 size() const;

    bool begin();
    void end();
};
