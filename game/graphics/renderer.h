#pragma once

#include <Tempest/RenderPipeline>
#include <Tempest/CommandBuffer>
#include <Tempest/Matrix4x4>
#include <Tempest/Widget>
#include <Tempest/Device>
#include <Tempest/UniformBuffer>
#include <Tempest/VectorImage>

#include "worldview.h"
#include "shaders.h"

class Camera;
class InventoryMenu;
class VideoWidget;

class Renderer final {
  public:
    Renderer(Tempest::Swapchain& swapchain);
    ~Renderer();

    void resetSwapchain();
    void onWorldChanged();

    void draw(Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t cmdId, size_t imgId,
              Tempest::VectorImage::Mesh& uiLayer, Tempest::VectorImage::Mesh& numOverlay,
              InventoryMenu &inventory, VideoWidget& video);

    void dbgDraw(Tempest::Painter& painter);

    Tempest::Attachment       screenshoot(uint8_t frameId);

  private:
    void updateCamera(const Camera &camera);
    void prepareUniforms();
    void setupTlas(const Tempest::AccelerationStructure* tlas);

    void prepareSky       (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void prepareSSAO      (Tempest::Encoder<Tempest::CommandBuffer>& cmd);
    void prepareFog       (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void prepareIrradiance(Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId);

    void drawHiZ          (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawGBuffer      (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawGWater       (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawShadowMap    (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawShadowResolve(Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, const WorldView& view);
    void drawLights       (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawSky          (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId, WorldView& view);
    void drawAmbient      (Tempest::Encoder<Tempest::CommandBuffer>& cmd, const WorldView& view);
    void draw             (Tempest::Attachment& result, Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId);
    void drawTonemapping  (Tempest::Encoder<Tempest::CommandBuffer>& cmd);
    void drawReflections  (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId);
    void drawUnderwater   (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId);
    void stashSceneAux    (Tempest::Encoder<Tempest::CommandBuffer>& cmd, uint8_t fId);
    void initSettings();

    struct Settings {
      const uint32_t shadowResolution   = 2048;
      bool           zEnvMappingEnabled = false;
      bool           zCloudShadowScale  = false;

      float          zVidBrightness     = 0.5;
      float          zVidContrast       = 0.5;
      float          zVidGamma          = 0.5;
      } settings;

    Frustrum                  frustrum[SceneGlobals::V_Count];
    Tempest::Swapchain&       swapchain;
    Tempest::Matrix4x4        proj, viewProj, viewProjLwc;
    Tempest::Matrix4x4        shadowMatrix[Resources::ShadowLayers];
    Tempest::Vec3             clipInfo;

    Tempest::Attachment       sceneLinear;
    Tempest::ZBuffer          zbuffer, shadowMap[Resources::ShadowLayers];

    Tempest::Attachment       sceneOpaque;
    Tempest::Attachment       sceneDepth;

    Tempest::Attachment       gbufDiffuse;
    Tempest::Attachment       gbufNormal;

    struct Shadow {
      Tempest::RenderPipeline* composePso = nullptr;
      Tempest::DescriptorSet   ubo;
    } shadow;

    struct Water {
      Tempest::RenderPipeline* reflectionsPso = nullptr;
      Tempest::DescriptorSet   ubo;
      Tempest::DescriptorSet   underUbo;
    } water;

    struct SSAO {
      Tempest::TextureFormat    aoFormat = Tempest::TextureFormat::R8;
      Tempest::StorageImage     ssaoBuf;

      Tempest::ComputePipeline* ssaoPso = nullptr;
      Tempest::DescriptorSet    uboSsao;

      Tempest::RenderPipeline*  ambientComposePso = nullptr;
      Tempest::DescriptorSet    uboCompose;
    } ssao;

    struct Irradiance {
      Tempest::StorageImage     lut;

      Tempest::ComputePipeline* pso = nullptr;
      Tempest::DescriptorSet    ubo;
      } irradiance;

    struct Tonemapping {
      Tempest::RenderPipeline* pso = nullptr;
      Tempest::DescriptorSet   uboTone;
    } tonemapping;

    Tempest::TextureFormat    shadowFormat  = Tempest::TextureFormat::Depth16;
    Tempest::TextureFormat    zBufferFormat = Tempest::TextureFormat::Depth16;

    Tempest::StorageImage     hiZ;
    Tempest::StorageBuffer    hiZRaw;

    Tempest::DescriptorSet    uboHiZRaw, uboHiZPot;
    std::vector<Tempest::DescriptorSet> uboZMip;

    Tempest::DescriptorSet    uboStash;

    Shaders                   stor;
  };
