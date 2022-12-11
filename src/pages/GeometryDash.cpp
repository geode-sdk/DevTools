#include "../platform/platform.hpp"
#include "../DevTools.hpp"
#include "../ImGui.hpp"

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
        } break;

        default:
        case HighlightMode::Hovered: {
            foreground.AddRectFilled(
                tmin, tmax, IM_COL32(0, 255, 55, 70)
            );
        } break;
    }
}

void DevTools::drawGD(GLRenderCtx* gdCtx) {
    if (gdCtx) {
        if (ImGui::Begin("Geometry Dash")) {
            auto list = ImGui::GetWindowDrawList();
            auto ratio = gdCtx->size().x / gdCtx->size().y;

            auto pad = ImGui::GetStyle().FramePadding.x;

            auto winPos = ImGui::GetWindowPos() +
                ImGui::GetWindowContentRegionMin();
            
            auto winSize = ImGui::GetWindowContentRegionMax() -
                ImGui::GetWindowContentRegionMin();
            
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
            
            for (auto& [node, mode] : m_toHighlight) {
                this->drawHighlight(node, mode);
            }
            m_toHighlight.clear();
        }
        ImGui::End();
    }
}
