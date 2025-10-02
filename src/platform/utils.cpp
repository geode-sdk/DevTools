#include "utils.hpp"

#if defined(GEODE_IS_MACOS)
#include <OpenGL/gl.h>
#elif defined(GEODE_IS_IOS)
#include <OpenGLES/ES2/gl.h>
#endif
#include <unordered_map>
#include <cocos2d.h>

using namespace cocos2d;

std::string formatAddressIntoOffset(uintptr_t addr, bool module) {
    static std::unordered_map<uintptr_t, std::pair<std::string, std::string>> formatted;
    auto it = formatted.find(addr);
    if (it != formatted.end()) {
        if(module) return it->second.first;
        else return it->second.second;
    } else {
        auto const txt = formatAddressIntoOffsetImpl(addr, true);
        auto const txtNoModule = formatAddressIntoOffsetImpl(addr, false);
        auto const pair = std::make_pair(txt, txtNoModule);
        formatted.insert({ addr, pair });
        if(module) return pair.first;
        else return pair.second;
    }
}

std::vector<uint8_t> renderToBytes(cocos2d::CCNode* node, int& width, int& height) {
    // Get scale from cocos2d units to opengl units
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    width = node->getContentSize().width * (viewport[2] / winSize.width);
    height = node->getContentSize().height * (viewport[3] / winSize.height);

    // Create Texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create Framebuffer Object
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Clear any data
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Flip Y when projecting
    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLPushMatrix();
    kmGLLoadIdentity();

    kmMat4 ortho;
    kmMat4OrthographicProjection(&ortho,
        0.0f, winSize.width,
        winSize.height, 0.0f,
        -1.0f, 1.0f
    );
    kmGLMultMatrix(&ortho);

    // Transform matrix so the node is drawn at 0,0
    kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPushMatrix();
    kmGLLoadIdentity();

    auto anchor = node->isIgnoreAnchorPointForPosition() ? ccp(0, 0) : node->getAnchorPointInPoints();
    kmGLTranslatef(
        anchor.x - node->getPositionX(),
        anchor.y - node->getPositionY() + (winSize.height - node->getContentSize().height),
        0
    );

    // Visit
    node->visit();              

    // Undo matrix transformations
    kmGLPopMatrix();
    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLPopMatrix();
    kmGLMatrixMode(KM_GL_MODELVIEW);

    // Read from Framebuffer
    std::vector<unsigned char> pixels(width * height * 4); // RGBA8
    glReadPixels(
        0, 0, width, height,
        GL_RGBA, GL_UNSIGNED_BYTE,
        pixels.data()
    );

    // Unbind Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Delete
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);

    return pixels;
}

void saveRenderToFile(std::vector<uint8_t> const& data, int width, int height, char const* filename) {
    auto img = new CCImage();
    img->initWithImageData((void*)data.data(), data.size(), CCImage::kFmtRawData, width, height, 8);
    img->saveToFile(filename);
}