#include <cocos2d.h>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCIMEDispatcher.hpp>
#include "Geode/cocos/text_input_node/CCIMEDelegate.h"
#include "Geode/platform/cplatform.h"
#include "platform/platform.hpp"
#include "DevTools.hpp"
#include "ImGui.hpp"
#include <array>

using namespace cocos2d;

// based off https://github.com/matcool/gd-imgui-cocos

static bool g_useNormalPos = false;

CCPoint getMousePos_H() {
    CCPoint mouse = cocos::getMousePos();
    const auto pos = toVec2(mouse);

    if (DevTools::get()->shouldUseGDWindow() && shouldPassEventsToGDButTransformed() && !g_useNormalPos) {
        auto win = ImGui::GetMainViewport()->Size;
        const auto gdRect = getGDWindowRect();

        auto relativePos = ImVec2(
            pos.x - gdRect.Min.x,
            pos.y - gdRect.Min.y
        );
        auto x = (relativePos.x / gdRect.GetWidth()) * win.x;
        auto y = (relativePos.y / gdRect.GetHeight()) * win.y;

        mouse = toCocos(ImVec2(x, y));
    }
    
    return mouse;
}

void DevTools::setupPlatform() {
    auto& io = ImGui::GetIO();

    io.BackendPlatformUserData = this;
    io.BackendPlatformName = "cocos2d-2.2.3 GD";
    // this is a lie hehe
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    // use static since imgui does not own the pointer!
    static const auto iniPath = (Mod::get()->getSaveDir() / "imgui.ini").u8string();
    io.IniFilename = reinterpret_cast<const char*>(iniPath.c_str());

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_fontTexture = new CCTexture2D;
    m_fontTexture->initWithData(pixels, kCCTexture2DPixelFormat_RGBA8888, width, height, CCSize(width, height));
    m_fontTexture->retain();

    io.Fonts->SetTexID(static_cast<ImTextureID>(m_fontTexture->getName()));

    // fixes getMousePos to be relative to the GD view
    #ifndef GEODE_IS_MOBILE
    (void) Mod::get()->hook(
        reinterpret_cast<void*>(addresser::getNonVirtual(&geode::cocos::getMousePos)),
        &getMousePos_H,
        "geode::cocos::getMousePos"
    );
    #endif
}

#ifdef GEODE_IS_MOBILE

class DevToolsIMEDelegate : public CCIMEDelegate {
protected:
    bool m_attached = false;
    std::string m_text;
public:
    bool attachWithIME() override {
        if (CCIMEDelegate::attachWithIME()) {
            // being anywhere but end of line ends up messing up the text, so this sends it to the end of the line
            #ifdef GEODE_IS_ANDROID
            ImGui::GetIO().AddKeyEvent(ImGuiKey_End, true);
            ImGui::GetIO().AddKeyEvent(ImGuiKey_End, false);
            #endif
            m_attached = true;
            CCEGLView::get()->setIMEKeyboardState(true);
            return true;
        }
        return false;
    }
    
    bool detachWithIME() override {
        if (CCIMEDelegate::detachWithIME()) {
            m_attached = false;
            CCEGLView::get()->setIMEKeyboardState(false);
            ImGui::ClearActiveID();
            return true;
        }
        return false;
    }

    bool canAttachWithIME() override {
        return true;
    }

    bool canDetachWithIME() override {
        return true;
    }

    char const* getContentText() override {
        m_text = "";
        for (auto str : ImGui::GetInputTextState(ImGui::GetFocusID())->TextA) {
            m_text += str;
        }
        return m_text.c_str();
    }

    bool isAttached() {
        return m_attached;
    }

    static DevToolsIMEDelegate* get() {
        static DevToolsIMEDelegate* instance = new DevToolsIMEDelegate();
        return instance;
    }
};

#endif

void DevTools::newFrame() {
    auto& io = ImGui::GetIO();

    auto* director = CCDirector::sharedDirector();
    const auto winSize = director->getWinSize();
    const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();

    // glfw new frame
    io.DisplaySize = ImVec2(frameSize.width, frameSize.height);
    io.DisplayFramebufferScale = ImVec2(
        winSize.width / frameSize.width,
        winSize.height / frameSize.height
    );
    io.DeltaTime = director->getDeltaTime();

#ifdef GEODE_IS_DESKTOP
    g_useNormalPos = true;
    const auto mousePos = toVec2(geode::cocos::getMousePos());
    g_useNormalPos = false;
    io.AddMousePosEvent(mousePos.x, mousePos.y);
#endif

    // TODO: text input

    auto* kb = director->getKeyboardDispatcher();
    io.KeyAlt = kb->getAltKeyPressed() || kb->getCommandKeyPressed(); // look
    io.KeyCtrl = kb->getControlKeyPressed();
    io.KeyShift = kb->getShiftKeyPressed();

#ifdef GEODE_IS_MOBILE
    auto ime = DevToolsIMEDelegate::get();
    if (io.WantTextInput && !ime->isAttached()) {
	    ime->attachWithIME();
    } else if (!io.WantTextInput && ime->isAttached()) {
	    ime->detachWithIME();
    }
#endif
}

void DevTools::render(GLRenderCtx* ctx) {
    ccGLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->newFrame();

    ImGui::NewFrame();

    DevTools::get()->draw(ctx);

    ImGui::Render();

    this->renderDrawData(ImGui::GetDrawData());
}

bool DevTools::hasExtension(const std::string& ext) const {
    auto exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts == nullptr) {
        return false;
    }

    std::string extsStr(exts);
    return extsStr.find(ext) != std::string::npos;
}

namespace {
    static void drawTriangle(const std::array<CCPoint, 3>& poli, const std::array<ccColor4F, 3>& colors, const std::array<CCPoint, 3>& uvs) {
        auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
        shader->use();
        shader->setUniformsForBuiltins();

        ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);

        static_assert(sizeof(CCPoint) == sizeof(ccVertex2F), "so the cocos devs were right then");
        
        glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, poli.data());
        glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors.data());
        glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, uvs.data());

        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    }
}

void DevTools::renderDrawDataFallback(ImDrawData* draw_data) {
    glEnable(GL_SCISSOR_TEST);

    const auto clip_scale = draw_data->FramebufferScale;

    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        auto* list = draw_data->CmdLists[i];
        auto* idxBuffer = list->IdxBuffer.Data;
        auto* vtxBuffer = list->VtxBuffer.Data;
        for (auto& cmd : list->CmdBuffer) {
            ccGLBindTexture2D(static_cast<GLuint>(cmd.GetTexID()));

            const auto rect = cmd.ClipRect;
            const auto orig = toCocos(ImVec2(rect.x, rect.y));
            const auto end = toCocos(ImVec2(rect.z, rect.w));
            if (end.x <= orig.x || end.y >= orig.y)
                continue;
            CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

            for (unsigned int i = 0; i < cmd.ElemCount; i += 3) {
                const auto a = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 0]];
                const auto b = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 1]];
                const auto c = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 2]];
                std::array<CCPoint, 3> points = {
                    toCocos(a.pos),
                    toCocos(b.pos),
                    toCocos(c.pos),
                };
                static constexpr auto ccc4FromImColor = [](const ImColor color) {
                    // beautiful
                    return ccc4f(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
                };
                std::array<ccColor4F, 3> colors = {
                    ccc4FromImColor(a.col),
                    ccc4FromImColor(b.col),
                    ccc4FromImColor(c.col),
                };

                std::array<CCPoint, 3> uvs = {
                    ccp(a.uv.x, a.uv.y),
                    ccp(b.uv.x, b.uv.y),
                    ccp(c.uv.x, c.uv.y),
                };

                drawTriangle(points, colors, uvs);
            }
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

void DevTools::renderDrawData(ImDrawData* draw_data) {
    static bool hasVaos = this->hasExtension("GL_ARB_vertex_array_object");
    if (!hasVaos) {
        return this->renderDrawDataFallback(draw_data);
    }

    glEnable(GL_SCISSOR_TEST);

    GLuint vao = 0;
    GLuint vbos[2] = {0, 0};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, &vbos[0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);

    glEnableVertexAttribArray(kCCVertexAttrib_Position);
    glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));

    glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));

    glEnableVertexAttribArray(kCCVertexAttrib_Color);
    glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

    auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
    shader->use();
    shader->setUniformsForBuiltins();

    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        auto* list = draw_data->CmdLists[i];

        // convert vertex coords to cocos space
        for(int j = 0; j < list->VtxBuffer.size(); j++) {
            auto point = toCocos(list->VtxBuffer[j].pos);
            list->VtxBuffer[j].pos = ImVec2(point.x, point.y);
        }

        glBufferData(GL_ARRAY_BUFFER, list->VtxBuffer.Size * sizeof(ImDrawVert), list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, list->IdxBuffer.Size * sizeof(ImDrawIdx), list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (auto& cmd : list->CmdBuffer) {
            ccGLBindTexture2D(static_cast<GLuint>(cmd.GetTexID()));

            const auto rect = cmd.ClipRect;
            const auto orig = toCocos(ImVec2(rect.x, rect.y));
            const auto end = toCocos(ImVec2(rect.z, rect.w));
            if (end.x <= orig.x || end.y >= orig.y)
                continue;
            CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

            glDrawElements(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, (GLvoid*)(cmd.IdxOffset * sizeof(ImDrawIdx)));
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(2, &vbos[0]);
    glDeleteVertexArrays(1, &vao);

    glDisable(GL_SCISSOR_TEST);
}

static float SCROLL_SENSITIVITY = 10;

#ifndef GEODE_IS_IOS
class $modify(CCMouseDispatcher) {
    bool dispatchScrollMSG(float y, float x) {
        if(!DevTools::get()->isSetup()) return true;

        auto& io = ImGui::GetIO();
        io.AddMouseWheelEvent(x / SCROLL_SENSITIVITY, -y / SCROLL_SENSITIVITY);

        if (!io.WantCaptureMouse || shouldPassEventsToGDButTransformed()) {
            return CCMouseDispatcher::dispatchScrollMSG(y, x);
        }

        return true;
    }
};
#endif

class $modify(CCTouchDispatcher) {
    static void onModify(auto& self) {
        /* 
         * some mods hook this instead of using normal touch delegates for some reason 
         * for example QOLMod, even in the rewrite
         * so i added hook priority
         */
        Result<> res = self.setHookPriorityPre("cocos2d::CCTouchDispatcher::touches", Priority::First);
        if (!res) {
            geode::log::warn("Failed to set hook priority for CCTouchDispatcher::touches: {}", res.unwrapErr());
        }
    }

    void touches(CCSet* touches, CCEvent* event, unsigned int type) {
        auto& io = ImGui::GetIO();
        auto* touch = static_cast<CCTouch*>(touches->anyObject());

        // for some reason mac can filter out out of touches i think?
        if (touch == nullptr) {
            // i am very lazy to find ccset count
            // i don't even know if the std set in gnustl and libc++ are the same struct
            return;
        }

        const auto pos = toVec2(touch->getLocation());
        GEODE_MOBILE(io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);)
        io.AddMousePosEvent(pos.x, pos.y);
        if (io.WantCaptureMouse) {
            bool didGDSwallow = false;

            if (DevTools::get()->shouldUseGDWindow() && shouldPassEventsToGDButTransformed()) {
                auto win = ImGui::GetMainViewport()->Size;
                const auto gdRect = getGDWindowRect();
                if (gdRect.Contains(pos) && !DevTools::get()->pausedGame()) {
                    auto relativePos = ImVec2(
                        pos.x - gdRect.Min.x,
                        pos.y - gdRect.Min.y
                    );
                    auto x = (relativePos.x / gdRect.GetWidth()) * win.x;
                    auto y = (1.f - relativePos.y / gdRect.GetHeight()) * win.y;

                    auto pos = toCocos(ImVec2(x, y));
                    // setTouchInfo messes up the previous location (causes issues like texturer loader's draggable nodes breaking)
                    touch->m_point = pos;
                    if (type == CCTOUCHBEGAN) {
                        // makes the start location in the touch correct
                        touch->m_startPoint = pos;
                    }
                    CCTouchDispatcher::touches(touches, event, type);

                    ImGui::SetWindowFocus("Geometry Dash");
                    didGDSwallow = true;
                    io.AddMouseButtonEvent(0, false);
                }
            }

            // TODO: dragging out of gd makes it click in imgui
            if (!didGDSwallow) {
                if (type == CCTOUCHBEGAN || type == CCTOUCHMOVED) {
                    GEODE_MOBILE(io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);)
                    io.AddMouseButtonEvent(0, true);
                }
                else {
                    io.AddMouseButtonEvent(0, false);
                }
            }
        }
        else {
            if (type != CCTOUCHMOVED) {
                io.AddMouseButtonEvent(0, false);
            }
            if (!DevTools::get()->shouldUseGDWindow() || !DevTools::get()->shouldPopGame()) {
                CCTouchDispatcher::touches(touches, event, type);
            }
        }
    }
};

class $modify(CCIMEDispatcher) {
    void dispatchInsertText(const char* text, int len, enumKeyCodes key) {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            CCIMEDispatcher::dispatchInsertText(text, len, key);
        }
        std::string str(text, len);
        io.AddInputCharactersUTF8(str.c_str());
    }

    void dispatchDeleteBackward() {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            CCIMEDispatcher::dispatchDeleteBackward();
        }
        // is this really how youre supposed to do this
        io.AddKeyEvent(ImGuiKey_Backspace, true);
        io.AddKeyEvent(ImGuiKey_Backspace, false);
    }
};

ImGuiKey cocosToImGuiKey(cocos2d::enumKeyCodes key) {
	if (key >= KEY_A && key <= KEY_Z) {
		return static_cast<ImGuiKey>(ImGuiKey_A + (key - KEY_A));
	}
	if (key >= KEY_Zero && key <= KEY_Nine) {
		return static_cast<ImGuiKey>(ImGuiKey_0 + (key - KEY_Zero));
	}
	switch (key) {
		case KEY_Up: return ImGuiKey_UpArrow;
		case KEY_Down: return ImGuiKey_DownArrow;
		case KEY_Left: return ImGuiKey_LeftArrow;
		case KEY_Right: return ImGuiKey_RightArrow;

		case KEY_Control: return ImGuiKey_ModCtrl;
        case KEY_LeftWindowsKey: return ImGuiKey_ModSuper;
		case KEY_Shift: return ImGuiKey_ModShift;
		case KEY_Alt: return ImGuiKey_ModAlt;
		case KEY_Enter: return ImGuiKey_Enter;

		case KEY_Home: return ImGuiKey_Home;
		case KEY_End: return ImGuiKey_End;
        // macos uses delete instead of backspace for some reason
        #ifndef GEODE_IS_MACOS
		case KEY_Delete: return ImGuiKey_Delete;
        #endif
		case KEY_Escape: return ImGuiKey_Escape;

        // KEY_Control and KEY_Shift aren't called on android like windows or mac
        #ifdef GEODE_IS_ANDROID
        case KEY_LeftControl: return ImGuiKey_ModCtrl;
        case KEY_RightContol: return ImGuiKey_ModCtrl;
        case KEY_LeftShift: return ImGuiKey_ModShift;
        case KEY_RightShift: return ImGuiKey_ModShift;
        #endif

		default: return ImGuiKey_None;
	}
}

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double a4) {
        if(!DevTools::get()->isSetup()) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, a4);

		auto& io = ImGui::GetIO();
		const auto imKey = cocosToImGuiKey(key);
		if (imKey != ImGuiKey_None) {
			io.AddKeyEvent(imKey, down);
		}

        // CCIMEDispatcher stuff only gets called on mobile if the virtual keyboard would be up.
        // Similarly, CCKeyboardDispatcher doesn't get called if the virtual keyboard would be up.
        #ifdef GEODE_IS_MOBILE
        if (down) {
            char c = 0;
            if (key >= KEY_A && key <= KEY_Z) {
                c = static_cast<char>(key);
                if (!io.KeyShift) {
                    c = static_cast<char>(tolower(c));
                }
            } else if (key >= KEY_Zero && key <= KEY_Nine) {
                c = static_cast<char>('0' + (key - KEY_Zero));
            } else if (key == KEY_Space) {
                c = ' ';
            }

            if (c != 0) {
                std::string str(1, c);
                io.AddInputCharactersUTF8(str.c_str());
            }
        }
        if (key == KEY_Backspace) {
            io.AddKeyEvent(ImGuiKey_Backspace, true);
            io.AddKeyEvent(ImGuiKey_Backspace, false);
        }
        #endif

		if (io.WantCaptureKeyboard) {
			return false;
		} else {
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, a4);
		}
    }

    #if defined(GEODE_IS_MACOS) || defined(GEODE_IS_IOS)
    static void onModify(auto& self) {
        Result<> res = self.setHookPriorityBeforePre("CCKeyboardDispatcher::updateModifierKeys", "geode.custom-keybinds");
        if (!res) {
            geode::log::warn("Failed to set hook priority for CCKeyboardDispatcher::updateModifierKeys: {}", res.unwrapErr());
        }
    }

    void updateModifierKeys(bool shft, bool ctrl, bool alt, bool cmd) {
        auto& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_ModShift, shft);
        io.AddKeyEvent(ImGuiKey_ModCtrl, ctrl);
        io.AddKeyEvent(ImGuiKey_ModAlt, alt);
        io.AddKeyEvent(ImGuiKey_ModSuper, cmd);
        CCKeyboardDispatcher::updateModifierKeys(shft, ctrl, alt, cmd);
    }
    #endif
};
