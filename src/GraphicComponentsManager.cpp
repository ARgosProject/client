#include "GraphicComponentsManager.h"
#include "GraphicComponent.h"
#include "RenderToTextureComponent.h"
#include "LineComponent.h"
#include "RectangleComponent.h"
#include "TextComponent.h"
#include "ImageComponent.h"
#include "VideoStreamComponent.h"
#include "VideoComponent.h"
#include "Log.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace argosClient {

  GraphicComponentsManager::GraphicComponentsManager()
    : _projectionMatrix(glm::mat4(1.0f)), _imagesPath(""), _videosPath(""), _fontsPath("") {

  }

  GraphicComponentsManager::~GraphicComponentsManager() {
    Log::info("Releasing all graphic collections...");
    _gcCollections.clear();
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::getGCCollection(const std::string& name) {
    if(_gcCollections.find(name) == _gcCollections.end()) {
      Log::error("Could not get the GCCollection named '" + name + "'");
      exit(1);
    }

    return _gcCollections[name];
  }

  void GraphicComponentsManager::removeGCCollection(const std::string& name) {
    auto it = _gcCollections.find(name);
    _gcCollections.erase(it);
  }

  void GraphicComponentsManager::showGCCollection(const std::string& name, bool show) {
    if(_gcCollections.find(name) == _gcCollections.end()) {
      Log::error("Could not show the GCCollection named '" + name + "'");
    }

    _gcCollections[name]->show(show);
  }

  void GraphicComponentsManager::render(const std::string& name) {
    _gcCollections[name]->render();
  }

  void GraphicComponentsManager::renderAll() {
    for(auto& gcc : _gcCollections) {
      gcc.second->render();
    }
  }

  void GraphicComponentsManager::update(const glm::mat4& modelViewMatrix) {
    for(auto& gcc : _gcCollections) {
      gcc.second->update(modelViewMatrix);
    }
  }

  void GraphicComponentsManager::setProjectionMatrix(const glm::mat4& projectionMatrix) {
    _projectionMatrix = projectionMatrix;
  }

  void GraphicComponentsManager::setImagesPath(const std::string& path) {
    _imagesPath = path;
  }

  void GraphicComponentsManager::setVideosPath(const std::string& path) {
    _videosPath = path;
  }

  void GraphicComponentsManager::setFontsPath(const std::string& path) {
    _fontsPath = path;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createImageFromFile(const std::string& name, const std::string& file_name,
                                                                                          const glm::vec3& pos, const glm::vec2& size) {
    std::shared_ptr<ImageComponent> imageComponent = std::make_shared<ImageComponent>(_imagesPath + file_name, size.x, size.y);
    imageComponent->setPosition(pos);

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");
    gcc->add(imageComponent);
    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createVideoFromFile(const std::string& name, const std::string& file_name,
                                                                                          const glm::vec3& pos, const glm::vec2& size) {
    std::shared_ptr<VideoComponent> videoComponent = std::make_shared<VideoComponent>(_videosPath + file_name, size.x, size.y);
    videoComponent->setPosition(pos);
    videoComponent->setProjectionMatrix(_projectionMatrix);

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");
    gcc->add(videoComponent);
    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createCorners(const std::string& name, float length, float wide,
                                                                                    const glm::vec4& colour, const glm::vec2& size) {
    // Positions
    float width = size.x;
    float height = size.y;

    // Corners
    std::vector<std::shared_ptr<GraphicComponent>> corners = {
      // Top-Left
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(length, 0, 0), wide),   // 0
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, -length, 0), wide),  // 1

      // Top-Right
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(-length, 0, 0), wide),  // 2
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, -length, 0), wide),  // 3

      // Bot-Left
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(length, 0, 0), wide),   // 4
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, length, 0), wide),   // 5

      // Bot-Right
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(-length, 0, 0), wide),  // 6
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, length, 0), wide)    // 7
    };

    glm::vec3 topLeftCorner(-width / 2.0f, height / 2.0f, 0.0f);
    glm::vec3 topRightCorner(width / 2.0f, height / 2.0f, 0.0f);
    glm::vec3 botLeftCorner(-width / 2.0f, -height / 2.0f, 0.0f);
    glm::vec3 botRightCorner(width / 2.0f, -height / 2.0f, 0.0f);

    corners[0]->setPosition(topLeftCorner);
    corners[1]->setPosition(topLeftCorner);
    corners[2]->setPosition(topRightCorner);
    corners[3]->setPosition(topRightCorner);
    corners[4]->setPosition(botLeftCorner);
    corners[5]->setPosition(botLeftCorner);
    corners[6]->setPosition(botRightCorner);
    corners[7]->setPosition(botRightCorner);

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");

    for(int i = 0; i < 8; ++i) {
      corners[i]->setColor(colour.r, colour.g, colour.b, colour.a);
      corners[i]->setProjectionMatrix(_projectionMatrix);
      gcc->add(corners[i]);
    }

    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createAxis(const std::string& name, float length, float wide,
                                                                                 const glm::vec3& pos) {
    // Lines
    std::vector<std::shared_ptr<GraphicComponent>> lines = {
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(length, 0, 0), wide),
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, length, 0), wide),
      std::make_shared<LineComponent>(glm::vec3(0, 0, 0), glm::vec3(0, 0, length), wide)
    };

    lines[0]->setColor(1, 0, 0, 1);
    lines[1]->setColor(0, 1, 0, 1);
    lines[2]->setColor(0, 0, 1, 1);

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");

    for(int i = 0; i < 3; ++i) {
      lines[i]->setPosition(pos);
      lines[i]->setProjectionMatrix(_projectionMatrix);
      gcc->add(lines[i]);
    }

    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createVideoStream(const std::string& name, const std::string& bg_file,
                                                                                        const glm::vec2& size, int port) {
    std::shared_ptr<ImageComponent> bg = std::make_shared<ImageComponent>(_imagesPath + "videoconference.jpg", size.x, size.y);
    bg->setProjectionMatrix(_projectionMatrix);

    std::shared_ptr<VideoStreamComponent> videoStream = std::make_shared<VideoStreamComponent>(size.y / 1.77, size.x / 1.77);
    videoStream->startReceivingVideo(port);
    videoStream->setPosition(glm::vec3(0.0f, 5.6f, 0.0f));
    videoStream->setScale(glm::vec3(1.055f, 0.85f, 1.0f));

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");
    gcc->add(bg);
    gcc->add(videoStream);
    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createTextPanel(const std::string& name, const glm::vec4& colour,
                                                                                      const std::string& text, const glm::vec3& pos) {
    // TODO
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createHighlight(const std::string& name, const glm::vec4& colour,
                                                                                      const glm::vec3& pos, const glm::vec3& scale) {
    // TODO
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createButton(const std::string& name, const glm::vec4& colour,
                                                                                   const std::string& text, const glm::vec3& pos) {
    // TODO
  }

  GraphicComponentsManager::GCCollectionPtr GraphicComponentsManager::createFactureHint(const std::string& name, const glm::vec3& pos, const glm::vec2& size,
                                                                                        const glm::vec4& colour, const std::wstring& title,
                                                                                        const std::vector<std::pair<std::wstring, glm::vec3>>& textBlocks) {
    float scaleFactor = 0.008f;
    std::shared_ptr<RenderToTextureComponent> rtt = std::make_shared<RenderToTextureComponent>(size.x, size.y);
    rtt->setScale(glm::vec3(-scaleFactor, scaleFactor, scaleFactor));
    rtt->setPosition(pos);
    rtt->setProjectionMatrix(_projectionMatrix);

    RectangleComponent* bg = new RectangleComponent(size.x, size.y);
    bg->setColor(colour.r, colour.g, colour.b, colour.a);
    rtt->addGraphicComponent(bg);

    TextComponent* tcTitle = new TextComponent(_fontsPath + "ProximaNova-Bold.ttf", 72);
    tcTitle->setScale(glm::vec3(1.0f, -1.0f, 1.0f));
    tcTitle->setPosition(glm::vec3(50.0f, 50.0f, 0.0f));
    tcTitle->setText(title);
    rtt->addGraphicComponent(tcTitle);

    for(auto& block : textBlocks) {
      TextComponent* textComponent = new TextComponent(_fontsPath + "ProximaNova-Bold.ttf", 54);
      textComponent->setScale(glm::vec3(1.0f, -1.0f, 1.0f));
      textComponent->setPosition(block.second);
      textComponent->setText(block.first);
      rtt->addGraphicComponent(textComponent);
    }

    GCCollectionPtr gcc = std::make_shared<GCCollection>(name + "_Collection");
    gcc->add(rtt);
    gcc->show(false);
    _gcCollections[name] = gcc;

    return gcc;
  }

}