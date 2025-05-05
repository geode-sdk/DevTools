#include "../platform/platform.hpp"
#include "../DevTools.hpp"
#include "../ImGui.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJDropDownLayer.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>

void drawRowAxisArrow(
    ImDrawList& foreground,
    float x, float y,
    ImVec2 const& tmax, ImVec2 const& tmin,
    AxisAlignment align,
    bool reverse, auto color
) {
    float axisArrowStart = x;
    float axisArrowLength = (tmax.x - tmin.x) / 2;
    if (align == AxisAlignment::Even) {
        axisArrowLength = tmax.x - tmin.x;
    }
    if (align == AxisAlignment::End) {
        axisArrowStart -= axisArrowLength;
    }
    if (align == AxisAlignment::Center && reverse) {
        axisArrowStart -= axisArrowLength;
    }
    if (reverse) {
        foreground.AddLine(
            ImVec2(axisArrowStart + 15.f, y),
            ImVec2(axisArrowStart + axisArrowLength, y),
            color, 4.f
        );
        foreground.AddTriangleFilled(
            ImVec2(axisArrowStart + 21.f, y + 10.f),
            ImVec2(axisArrowStart + 21.f, y - 10.f),
            ImVec2(axisArrowStart + 6.f, y),
            color
        );
    }
    else {
        foreground.AddLine(
            ImVec2(axisArrowStart, y),
            ImVec2(axisArrowStart + axisArrowLength - 10.f, y),
            color, 4.f
        );
        foreground.AddTriangleFilled(
            ImVec2(axisArrowStart + axisArrowLength - 15.f, y + 10.f),
            ImVec2(axisArrowStart + axisArrowLength - 15.f, y - 10.f),
            ImVec2(axisArrowStart + axisArrowLength, y),
            color
        );
    }
}

void drawColAxisArrow(
    ImDrawList& foreground,
    float x, float y,
    ImVec2 const& tmax, ImVec2 const& tmin,
    AxisAlignment align,
    bool reverse, auto color
) {
    float crossArrowStart = y;
    float crossArrowLength = (tmax.y - tmin.y) / 2;
    if (align == AxisAlignment::Even) {
        crossArrowLength = tmax.y - tmin.y;
    }
    if (align == AxisAlignment::End) {
        crossArrowStart -= crossArrowLength;
    }
    if (align == AxisAlignment::Center && !reverse) {
        crossArrowStart -= crossArrowLength;
    }
    if (reverse) {
        foreground.AddLine(
            ImVec2(x, crossArrowStart),
            ImVec2(x, crossArrowStart + crossArrowLength + 5.f),
            color, 4.f
        );
        foreground.AddTriangleFilled(
            ImVec2(x + 10.f, crossArrowStart + crossArrowLength + 15.f),
            ImVec2(x - 10.f, crossArrowStart + crossArrowLength + 15.f),
            ImVec2(x, crossArrowStart + crossArrowLength),
            color
        );
    }
    else {
        crossArrowStart += crossArrowLength;
        foreground.AddLine(
            ImVec2(x, crossArrowStart),
            ImVec2(x, crossArrowStart - crossArrowLength - 10.f),
            color, 4.f
        );
        foreground.AddTriangleFilled(
            ImVec2(x + 10.f, crossArrowStart - crossArrowLength - 15.f),
            ImVec2(x - 10.f, crossArrowStart - crossArrowLength - 15.f),
            ImVec2(x, crossArrowStart - crossArrowLength),
            color
        );
    }
}

void drawLayoutArrows(
    ImDrawList& foreground,
    AxisLayout* layout,
    ImVec2 const& tmax, ImVec2 const& tmin
) {
    auto row = layout->getAxis() == Axis::Row;

    float x;
    float y;
    switch (row ? layout->getAxisAlignment() : layout->getCrossAxisAlignment()) {
        case AxisAlignment::Start:
        case AxisAlignment::Between:
        case AxisAlignment::Even: x = tmin.x; break;
        case AxisAlignment::Center: x = tmin.x + (tmax.x - tmin.x) / 2; break;
        case AxisAlignment::End: x = tmax.x; break;
    }
    switch (row ? layout->getCrossAxisAlignment() : layout->getAxisAlignment()) {
        case AxisAlignment::Start:
        case AxisAlignment::Between:
        case AxisAlignment::Even: y = tmin.y; break;
        case AxisAlignment::Center: y = tmin.y + (tmax.y - tmin.y) / 2; break;
        case AxisAlignment::End: y = tmax.y; break;
    }

    if (row) {
        drawRowAxisArrow(
            foreground,
            x, y, tmax, tmin,
            layout->getAxisAlignment(),
            layout->getAxisReverse(),
            IM_COL32(255, 55, 55, 255)
        );
        if (layout->getGrowCrossAxis()) {
            drawColAxisArrow(
                foreground,
                x, y, tmax, tmin,
                layout->getCrossAxisAlignment(),
                layout->getCrossAxisReverse(),
                IM_COL32(55, 55, 255, 255)
            );
        }
    }
    else {
        drawColAxisArrow(
            foreground,
            x, y, tmax, tmin,
            layout->getAxisAlignment(),
            !layout->getAxisReverse(),
            IM_COL32(255, 55, 55, 255)
        );
        if (layout->getGrowCrossAxis()) {
            drawRowAxisArrow(
                foreground,
                x, y, tmax, tmin,
                layout->getCrossAxisAlignment(),
                !layout->getCrossAxisReverse(),
                IM_COL32(55, 55, 255, 255)
            );
        }
    }
    
    foreground.AddCircleFilled(
        ImVec2(x, y), 6.f, IM_COL32(255, 55, 55, 255)
    );
}

AxisAlignment translateToAxisAlignment(const MainAxisAlignment& alignment) {
    switch (alignment) {
        case MainAxisAlignment::Start: return AxisAlignment::Start;
        case MainAxisAlignment::Between: return AxisAlignment::Between;
        case MainAxisAlignment::Even: return AxisAlignment::Even;
        case MainAxisAlignment::Center: return AxisAlignment::Center;
        case MainAxisAlignment::Around:
        case MainAxisAlignment::End: return AxisAlignment::End;
    }
    return AxisAlignment::Start;
}

AxisAlignment translateToAxisAlignment(const CrossAxisAlignment& alignment) {
    switch (alignment) {
        case CrossAxisAlignment::Start: return AxisAlignment::Start;
        case CrossAxisAlignment::Center: return AxisAlignment::Center;
        case CrossAxisAlignment::End: return AxisAlignment::End;
    }
    return AxisAlignment::Start;
}

void drawLayoutArrows(
    ImDrawList& foreground,
    SimpleAxisLayout* layout,
    ImVec2 const& tmax, ImVec2 const& tmin
) {
    auto row = layout->getAxis() == Axis::Row;

    float x;
    float y;

    // cheating
    AxisAlignment mainAxisAlignment = translateToAxisAlignment(layout->getMainAxisAlignment());
    AxisAlignment crossAxisAlignment = translateToAxisAlignment(layout->getCrossAxisAlignment());

    switch (row ? mainAxisAlignment : crossAxisAlignment) {
        case AxisAlignment::Start:
        case AxisAlignment::Between:
        case AxisAlignment::Even: x = tmin.x; break;
        case AxisAlignment::Center: x = tmin.x + (tmax.x - tmin.x) / 2; break;
        case AxisAlignment::End: x = tmax.x; break;
    }
    switch (row ? crossAxisAlignment : mainAxisAlignment) {
        case AxisAlignment::Start:
        case AxisAlignment::Between:
        case AxisAlignment::Even: y = tmin.y; break;
        case AxisAlignment::Center: y = tmin.y + (tmax.y - tmin.y) / 2; break;
        case AxisAlignment::End: y = tmax.y; break;
    }

    bool mainReverse = layout->getMainAxisDirection() == static_cast<AxisDirection>(1);
    bool crossReverse = layout->getCrossAxisDirection() == static_cast<AxisDirection>(1);

    bool growCrossAxis = layout->getCrossAxisScaling() == AxisScaling::Grow;

    if (row) {
        drawRowAxisArrow(
            foreground,
            x, y, tmax, tmin,
            mainAxisAlignment,
            mainReverse,
            IM_COL32(255, 55, 55, 255)
        );
        if (growCrossAxis) {
            drawColAxisArrow(
                foreground,
                x, y, tmax, tmin,
                crossAxisAlignment,
                crossReverse,
                IM_COL32(55, 55, 255, 255)
            );
        }
    }
    else {
        drawColAxisArrow(
            foreground,
            x, y, tmax, tmin,
            mainAxisAlignment,
            !mainReverse,
            IM_COL32(255, 55, 55, 255)
        );
        if (growCrossAxis) {
            drawRowAxisArrow(
                foreground,
                x, y, tmax, tmin,
                crossAxisAlignment,
                !crossReverse,
                IM_COL32(55, 55, 255, 255)
            );
        }
    }
    
    foreground.AddCircleFilled(
        ImVec2(x, y), 6.f, IM_COL32(255, 55, 55, 255)
    );
}

void DevTools::drawHighlight(CCNode* node, HighlightMode mode) {
	auto& foreground = *ImGui::GetWindowDrawList();
	auto parent = node->getParent();
	auto bounding_box = node->boundingBox();
	CCPoint bb_min(bounding_box.getMinX(), bounding_box.getMinY());
	CCPoint bb_max(bounding_box.getMaxX(), bounding_box.getMaxY());

#ifdef GEODE_IS_WINDOWS
	auto cameraParent = node;
	while (cameraParent) {
		auto camera = cameraParent->getCamera();

		float off_x, off_y, off_z;
		camera->getEyeXYZ(&off_x, &off_y, &off_z);
		const CCPoint offset(off_x, off_y);
		bb_min -= offset;
		bb_max -= offset;

		cameraParent = cameraParent->getParent();
	}
#endif

	auto min = toVec2(parent ? parent->convertToWorldSpace(bb_min) : bb_min);
	auto max = toVec2(parent ? parent->convertToWorldSpace(bb_max) : bb_max);

    auto wsize = ImGui::GetMainViewport()->Size;
    auto rect = getGDWindowRect();

    auto tmin = ImVec2(
        min.x / wsize.x * rect.GetWidth() + rect.Min.x,
        min.y / wsize.y * rect.GetHeight() + rect.Min.y
    );
    auto tmax = ImVec2(
        max.x / wsize.x * rect.GetWidth() + rect.Min.x,
        max.y / wsize.y * rect.GetHeight() + rect.Min.y
    );

    if (
        isnan(tmax.x) ||
        isnan(tmax.y) ||
        isnan(tmin.x) ||
        isnan(tmin.y)
    ) {
        return;
    }

    auto anchor = ImVec2(
        node->getAnchorPoint().x * (tmax.x - tmin.x) + tmin.x,
        node->getAnchorPoint().y * (tmax.y - tmin.y) + tmin.y
    );

    switch (mode) {
        case HighlightMode::Selected: {
            foreground.AddRect(
                tmin, tmax, IM_COL32(0, 255, 55, 155),
                0.f, 0, 3.f
            );
            foreground.AddCircleFilled(
                anchor, 7.5f, IM_COL32(255, 75, 105, 255)
            );
            foreground.AddCircleFilled(
                anchor, 5.f, IM_COL32(255, 255, 255, 255)
            );
            if (auto layout = typeinfo_cast<AxisLayout*>(node->getLayout())) {
                drawLayoutArrows(foreground, layout, tmax, tmin);
            }
            if (auto layout = typeinfo_cast<SimpleAxisLayout*>(node->getLayout())) {
                drawLayoutArrows(foreground, layout, tmax, tmin);
            }
        } break;

        case HighlightMode::Layout: {
            foreground.AddRect(
                tmin, tmax, IM_COL32(255, 155, 55, 255),
                0.f, 0, 4.f
            );
            // built-in Geode layouts get special extra markings
            if (auto layout = typeinfo_cast<AxisLayout*>(node->getLayout())) {
                drawLayoutArrows(foreground, layout, tmax, tmin);
            }
            if (auto layout = typeinfo_cast<SimpleAxisLayout*>(node->getLayout())) {
                drawLayoutArrows(foreground, layout, tmax, tmin);
            }
        } break;

        default:
        case HighlightMode::Hovered: {
            foreground.AddRectFilled(
                tmin, tmax, IM_COL32(0, 255, 55, 70)
            );
        } break;
    }
}

void DevTools::drawLayoutHighlights(CCNode* node) {
    // TODO: undo later
    #if 0
    for (auto child : ranges::reverse(CCArrayExt<CCNode*>(node->getChildren()))) {
        if (!child->isVisible()) continue;
        if (child->getLayout()) {
            this->drawHighlight(child, HighlightMode::Layout);
        }
        this->drawLayoutHighlights(child);
        if (
            typeinfo_cast<FLAlertLayer*>(child) || 
            typeinfo_cast<GJDropDownLayer*>(child) ||
            typeinfo_cast<EditorPauseLayer*>(child)
        ) {
            break;
        }
    }
    #endif
}

void DevTools::drawGD(GLRenderCtx* gdCtx) {
    if (gdCtx) {
        auto winSize = CCDirector::get()->getWinSize();
        auto title = fmt::format(
            "Geometry Dash ({}x{})###devtools/geometry-dash",
            winSize.width, winSize.height
        );
        if (ImGui::Begin(title.c_str())) {
            auto list = ImGui::GetWindowDrawList();
            auto ratio = gdCtx->size().x / gdCtx->size().y;

            auto pad = ImGui::GetStyle().FramePadding.x;

            auto winPos = ImGui::GetCursorScreenPos();
            auto winSize = ImGui::GetContentRegionAvail();
            
            ImVec2 imgSize = {
                (winSize.y - pad * 2) * ratio,
                (winSize.y - pad * 2)
            };
            if (winSize.x - pad * 2 < imgSize.x) {
                imgSize = {
                    (winSize.x - pad * 2),
                    (winSize.x - pad * 2) / ratio
                };
            }
            auto imgPos = winPos + winSize / 2 - imgSize / 2;
            list->AddImage(
                gdCtx->texture(),
                imgPos, imgPos + imgSize,
                { 0, 1 }, { 1, 0 }
            );
            getGDWindowRect() = {
                imgPos.x, imgPos.y,
                imgPos.x + imgSize.x,
                imgPos.y + imgSize.y
            };
            shouldPassEventsToGDButTransformed() = 
                // ensure that the some other window isn't on top
                ImGui::IsWindowHovered() &&
                getGDWindowRect().Contains(ImGui::GetMousePos());
            
            if (m_settings.highlightLayouts) {
                this->drawLayoutHighlights(CCDirector::get()->getRunningScene());
            }

            for (auto& [node, mode] : m_toHighlight) {
                this->drawHighlight(node, mode);
            }
            m_toHighlight.clear();
        }
        ImGui::End();
    }
}

// cheaty way to detect resize and update the render
class ResizeWatcher : public CCObject {
private:
    int m_lastWidth;
    int m_lastHeight;
    float m_resizeTimer;
    bool m_hasPendingResize;
public:
    static ResizeWatcher* create() {
        auto ret = new ResizeWatcher();
        ret->autorelease();
        return ret;
    }

    void update(float dt) {
        auto view = CCDirector::sharedDirector()->getOpenGLView();
        int w = view->getFrameSize().width;
        int h = view->getFrameSize().height;

        if (w != m_lastWidth || h != m_lastHeight) {
            m_lastWidth = w;
            m_lastHeight = h;
            m_resizeTimer = 0.0f;
            m_hasPendingResize = true;
            return;
        }

        if (m_hasPendingResize) {
            m_resizeTimer += dt;
            if (m_resizeTimer > 0.2f) {
                m_hasPendingResize = false;
                shouldUpdateGDRenderBuffer() = true;
            }
        }
    }
};

$execute {
    Loader::get()->queueInMainThread([]{
        CCScheduler::get()->scheduleUpdateForTarget(ResizeWatcher::create(), INT_MAX, false);
    });
}