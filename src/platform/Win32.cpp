
#include "../GLEW/glew.h"
#include <Geode/DefaultInclude.hpp>

#ifdef GEODE_IS_WINDOWS

#include "platform.hpp"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include <cocos2d.h>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCDirector.hpp>
#include "../DevTools.hpp"
#include <windowsx.h>
#include "../ImGui.hpp"

using namespace cocos2d;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

void DevTools::setupPlatform() {
    // mmm....
    auto hwnd = WindowFromDC(
        *reinterpret_cast<HDC*>(
            reinterpret_cast<uintptr_t>(CCEGLView::get()->getWindow()) + 0x244
        )
    );
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init();
}

void DevTools::render(GLRenderCtx* ctx) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DevTools::get()->draw(ctx);

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glFlush();
}

class $modify(CCEGLView) {
    void updateWindow(int width, int height) {
        shouldUpdateGDRenderBuffer() = true;
        CCEGLView::updateWindow(width, height);
    }

    void pollEvents() {
        auto& io = ImGui::GetIO();

        bool blockInput = false;
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);

            if (io.WantCaptureMouse) {
                switch(msg.message) {
                    case WM_LBUTTONDBLCLK:
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                    case WM_MBUTTONDBLCLK:
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                    case WM_MOUSEACTIVATE:
                    case WM_MOUSEHOVER:
                    case WM_MOUSEHWHEEL:
                    case WM_MOUSELEAVE:
                    case WM_MOUSEMOVE:
                    case WM_MOUSEWHEEL:
                    case WM_NCLBUTTONDBLCLK:
                    case WM_NCLBUTTONDOWN:
                    case WM_NCLBUTTONUP:
                    case WM_NCMBUTTONDBLCLK:
                    case WM_NCMBUTTONDOWN:
                    case WM_NCMBUTTONUP:
                    case WM_NCMOUSEHOVER:
                    case WM_NCMOUSELEAVE:
                    case WM_NCMOUSEMOVE:
                    case WM_NCRBUTTONDBLCLK:
                    case WM_NCRBUTTONDOWN:
                    case WM_NCRBUTTONUP:
                    case WM_NCXBUTTONDBLCLK:
                    case WM_NCXBUTTONDOWN:
                    case WM_NCXBUTTONUP:
                    case WM_RBUTTONDBLCLK:
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                    case WM_XBUTTONDBLCLK:
                    case WM_XBUTTONDOWN:
                    case WM_XBUTTONUP:
                        blockInput = true;
                }
            }

            auto msgToGD = msg;
            if (shouldPassEventsToGDButTransformed() && msg.message == WM_MOUSEMOVE) {
                auto win = ImGui::GetMainViewport()->Size;
                auto mpos = ImVec2(
                    GET_X_LPARAM(msg.lParam) - getGDWindowRect().Min.x,
                    GET_Y_LPARAM(msg.lParam) - getGDWindowRect().Min.y
                );
                auto x = (mpos.x / getGDWindowRect().GetWidth()) * win.x;
                auto y = (mpos.y / getGDWindowRect().GetHeight()) * win.y;
                msgToGD.lParam = MAKELPARAM(x, y);
            }

            if (io.WantCaptureKeyboard) {
                switch(msg.message) {
                    case WM_HOTKEY:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_KILLFOCUS:
                    case WM_SETFOCUS:
                    case WM_SYSKEYDOWN:
                    case WM_SYSKEYUP:
                        blockInput = true;
                }
            }

            if (shouldPassEventsToGDButTransformed()) {
                blockInput = false;
            }
            // prevent clicks being passed to GD when clicking the black void 
            // if no imgui window is in front 
            else if (DevTools::get()->shouldPopGame()) {
                blockInput = ImRect(
                    { 0, 0 }, ImGui::GetMainViewport()->Size
                ).Contains(ImGui::GetMousePos());
            }

            if (!blockInput) {
                DispatchMessage(&msgToGD);
            }
            if (
                !shouldPassEventsToGDButTransformed() ||
                msg.message != WM_LBUTTONDOWN
            ) {
                ImGui_ImplWin32_WndProcHandler(
                    msg.hwnd, msg.message, msg.wParam, msg.lParam
                );
            }
        }

        CCEGLView::pollEvents();
    }
};

#endif
