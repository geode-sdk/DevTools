#include "../platform/platform.hpp"
#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../ImGui.hpp"
#include <Geode/loader/Log.hpp>

using namespace geode::prelude;

void DevTools::drawNodePreview(CCNode* node) {
    // TODO: this creates a new texture every frame and kills my gpu :sob:
    return;
    auto list = ImGui::GetWindowDrawList();
    auto winPos = ImGui::GetCursorScreenPos();
    auto winSize = ImGui::GetContentRegionAvail();
    
    auto pad = ImGui::GetStyle().FramePadding.x;

	auto parent = node->getParent();
	auto bounding_box = node->boundingBox();
	CCPoint bb_min(bounding_box.getMinX(), bounding_box.getMinY());
	CCPoint bb_max(bounding_box.getMaxX(), bounding_box.getMaxY());

#ifdef GEODE_IS_WINDOWS
    // TODO: define CCCamera::getEyeXYZ on mac
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

	auto min = parent ? parent->convertToWorldSpace(bb_min) : bb_min;
	auto max = parent ? parent->convertToWorldSpace(bb_max) : bb_max;

    ImVec2 size {
        winSize.x - pad,
        (winSize.x - pad) / ((max.x - min.x) / (max.y - min.y))
    };
    if (winSize.y - pad < size.y) {
        size = {
            (winSize.y - pad) / ((max.y - min.y) / (max.x - min.x)),
            winSize.y - pad
        };
    }
    auto pos = winPos + winSize / 2 - size / 2;

    list->AddRectFilled(
        pos, pos + size,
        IM_COL32(11, 11, 11, 255)
    );

    auto nodeSize = ImVec2 {
        (max.x - min.x) * CCEGLView::get()->getScaleX(),
        (max.y - min.y) * CCEGLView::get()->getScaleY()
    };
    auto oldPos = node->getPosition();
    auto oldAnchor = node->getAnchorPoint();
    auto oldVisiblity = node->isVisible();
    // unfortunately visit() calls glTranslate so we can't use that to draw the 
    // node at 0,0. instead we will just move the node temporarily
    node->setPosition(
        (parent ? parent->convertToNodeSpace(CCPointZero) : -min) + 
        node->getPosition() - min
    );
    node->setVisible(true);

    auto ctx = new GLRenderCtx(nodeSize);

    ctx->begin();
    node->visit();
    ctx->end();

    // restore original position
    node->setPosition(oldPos);
    node->setVisible(oldVisiblity);

    list->AddImage(ctx->texture(), pos, pos + size, { 0, 1 }, { 1, 0 });

    // make sure to clean up memory used by ctx, but only after the image 
    // texture has been rendered on screen
    list->AddCallback(+[](ImDrawList const* list, ImDrawCmd const* cmd) {
        delete reinterpret_cast<GLRenderCtx*>(cmd->UserCallbackData);
    }, ctx);
}

void DevTools::drawPreview() {
    if (!m_selectedNode) {
        ImGui::TextWrapped("Select a Node to Edit in the Scene or Tree");
    } else {
        this->drawNodePreview(m_selectedNode);
    }
}
