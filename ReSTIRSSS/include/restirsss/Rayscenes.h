#pragma once

#include "../../../VkRaven/raven/include/raven/core/GPUContext.h"
#include "../../../VkRaven/raven/include/raven/scene/RavenScene.h"
#include "../../../VkRaven/raven/include/raven/scene/loader/GLTFLoader.h"
#include "../../../VkRaven/raven/include/raven/util/Paths.h"

#include <random>

namespace raven {
    class Rayscenes {
    public:
        enum EScene {
            AJAX,
            AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
            AJAX_MANY_LIGHTS_W_LARGE_LIGHT,
            LTE_ORB,
            ASIAN_DRAGON,
            FLAT_TEST,
            UNDEFINED
        };

        static EScene sceneFromString(const std::string &scene) {
            if (scene == "ajax") {
                return AJAX;
            }
            if (scene == "ajax_manylights_wolargelight") {
                return AJAX_MANY_LIGHTS_WO_LARGE_LIGHT;
            }
            if (scene == "ajax_manylights_wlargelight") {
                return AJAX_MANY_LIGHTS_W_LARGE_LIGHT;
            }
            if (scene == "lteorb") {
                return LTE_ORB;
            }
            if (scene == "asiandragon") {
                return ASIAN_DRAGON;
            }
            if (scene == "flattest") {
                return FLAT_TEST;
            }
            return UNDEFINED;
        }

        struct SceneOptions {
            bool m_environmentMap = true;
            glm::vec2 m_environmentMapRotation = glm::vec2(0.f);
            glm::vec3 m_skyColor = glm::vec3(0);
        };
        SceneOptions m_sceneOptions{};

#define AJAX_POSITION 35.798f, 8.049f, -14.853f
#define AJAX_ORIENTATION 0.f, 1.978f, 1.818f
#define AJAX_TRAJECTORY_START 36.612f, 8.049f, -12.967f

#define ASIAN_DRAGON_POSITION 8.409f, 6.008f, -7.942f
#define ASIAN_DRAGON_ORIENTATION 0.f, 2.577f, 2.122f
#define ASIAN_DRAGON_TRAJECTORY_START 9.433f, 6.008f, -7.294f

#define LTE_ORB_POSITION -1.775f, 3.657f, -2.631f
#define LTE_ORB_ORIENTATION 0.f, 3.733f, 2.276f
#define LTE_ORB_TRAJECTORY_START -1.375f, 3.657f, -2.912f

        void sceneAjax(GPUContext *gpuContext, RavenScene &scene) {
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/ajax";
                const std::string SCENE_FILE = "ajax.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 1.f;
                transformation[1][1] = 1.f;
                transformation[2][2] = 1.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                scene.getMaterial("ajax")->m_material.baseColorFactor = glm::vec4(0.83f, 0.79f, 0.75f, 0.f);
                scene.getMaterial("ajax")->m_material.subsurface = 0.5f;
                //            scene.getMaterial("ajax")->m_material.mfpFromVolumetricCoefficients(glm::vec3(2.19f, 2.62f, 3.f), glm::vec3(0.0021f, 0.0041f, 0.0071f)); // marble
                scene.getMaterial("ajax")->m_material.meanFreePath = glm::vec3(1.f, 0.2f, 0.1f); // skin
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/background";
                const std::string SCENE_FILE = "background.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 42.f; // 512.f;
                transformation[1][1] = 42.f; // 512.f;
                transformation[2][2] = 42.f; // 512.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = -20.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 3.f, glm::vec3(0, 1, 0)));
                scene.loadScene(&sceneLoader);
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane_emissive.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 16.f;
                transformation[1][1] = 16.f;
                transformation[2][2] = 16.f;
                transformation[3][0] = -14.f;
                transformation[3][1] = 16.f;
                transformation[3][2] = -33.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 4.f, glm::vec3(1, 0, 0)));
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane_emissive")->setEmission(glm::vec3(1.f));
            }
            m_sceneOptions.m_skyColor = glm::vec3(0.f);
            m_sceneOptions.m_environmentMap = false;
        }

        void sceneAjaxManyLights(GPUContext *gpuContext, RavenScene &scene, bool largeLight) {
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/ajax";
                const std::string SCENE_FILE = "ajax.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 1.f;
                transformation[1][1] = 1.f;
                transformation[2][2] = 1.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                scene.getMaterial("ajax")->m_material.baseColorFactor = glm::vec4(0.83f, 0.79f, 0.75f, 0.f);
                scene.getMaterial("ajax")->m_material.subsurface = 0.5f;
                //            scene.getMaterial("ajax")->m_material.mfpFromVolumetricCoefficients(glm::vec3(2.19f, 2.62f, 3.f), glm::vec3(0.0021f, 0.0041f, 0.0071f)); // marble
                scene.getMaterial("ajax")->m_material.meanFreePath = glm::vec3(1.f, 0.2f, 0.1f); // skin
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/background";
                const std::string SCENE_FILE = "background.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 42.f;
                transformation[1][1] = 42.f;
                transformation[2][2] = 42.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = -20.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 3.f, glm::vec3(0, 1, 0)));
                scene.loadScene(&sceneLoader);
            }
            if (largeLight) {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane_emissive.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 16.f;
                transformation[1][1] = 16.f;
                transformation[2][2] = 16.f;
                transformation[3][0] = -14.f;
                transformation[3][1] = 16.f;
                transformation[3][2] = -33.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 4.f, glm::vec3(1, 0, 0)));
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane_emissive")->setEmission(glm::vec3(1.f));
            }
            {
                std::array<glm::vec3, 7> colors{
                        glm::vec3(1, 0, 0),
                        glm::vec3(0, 1, 0),
                        glm::vec3(0, 0, 1),
                        glm::vec3(1, 1, 0),
                        glm::vec3(1, 0, 1),
                        glm::vec3(0, 1, 1),
                        glm::vec3(1, 1, 1)};

                std::mt19937 rng(0);
                std::uniform_real_distribution<float> dis(0.f, 1.f); // [0,1)

                for (uint32_t i = 0; i < 512; i++) {
                    const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                    const std::string SCENE_FILE = "plane_emissive.gltf";
                    auto transformation = glm::mat4(1.f);
                    transformation[0][0] = glm::clamp(dis(rng), 0.05f, 1.f);
                    transformation[1][1] = glm::clamp(dis(rng), 0.05f, 1.f);
                    transformation[2][2] = glm::clamp(dis(rng), 0.05f, 1.f);
                    transformation[3][0] = -20.f + 40.f * dis(rng);
                    transformation[3][1] = -20.f + 40.f * dis(rng);
                    transformation[3][2] = -20.f + 40.f * dis(rng);
                    GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), dis(rng) * glm::pi<float>(), glm::normalize(glm::vec3(dis(rng), dis(rng), dis(rng)))), i + 1);
                    scene.loadScene(&sceneLoader);

                    std::string key = "plane_emissive";
                    if (largeLight) {
                        key += std::to_string(i);
                    } else if (i > 0) {
                        key += std::to_string(i - 1);
                    }
                    scene.getMaterial(key)->setEmission({colors[glm::clamp(static_cast<int>(glm::round(dis(rng) * 7.f)), 0, 6)]}, dis(rng) * 48.f);
                }
            }
            m_sceneOptions.m_skyColor = glm::vec3(0.f);
            m_sceneOptions.m_environmentMap = false;
        }

        void sceneFlatTest(GPUContext *gpuContext, RavenScene &scene) {
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 50.f;
                transformation[1][1] = 1.f;
                transformation[2][2] = 50.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane")->m_material.baseColorFactor = glm::vec4(1.f);
                scene.getMaterial("plane")->m_baseColorTexture = "undefined";

                scene.getMaterial("plane")->m_material.subsurface = 0.5f;
                scene.getMaterial("plane")->m_material.meanFreePath = glm::vec3(1.f, 0.2f, 0.1f); // skin
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane_emissive.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 0.1f;
                transformation[1][1] = 0.1f;
                transformation[2][2] = 0.1f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.1f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane_emissive")->setEmission(glm::vec3(1.f));
            }
            m_sceneOptions.m_skyColor = glm::vec3(0.f);
            m_sceneOptions.m_environmentMap = false;
        }

        void sceneLTEOrb(GPUContext *gpuContext, RavenScene &scene, const uint32_t sceneMode) {
            // https://github.com/lighttransport/lighttransportequation-orb (TestObjPoly.blend)
            // https://github.com/mmp/pbrt-v4-scenes?tab=readme-ov-file#lte-orb
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/lte-orb";
                const std::string SCENE_FILE = "lte-orb.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 10.f;
                transformation[1][1] = 10.f;
                transformation[2][2] = 10.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                switch (sceneMode) {
                    case 0:
                        // apple
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.846f, 0.841f, 0.528f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(6.96f, 6.4f, 1.9f);
                        break;
                    case 1:
                        // chicken1
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.314f, 0.156f, 0.126f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(11.61f, 3.88f, 1.75f);
                        break;
                    case 2:
                        // chicken2
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.321f, 0.160f, 0.108f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(9.44f, 3.35f, 1.79f);
                        break;
                    case 3:
                        // cream
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.976f, 0.900f, 0.725f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(15.03f, 4.66f, 2.54f);
                        break;
                    case 4:
                        // ketchup
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.164f, 0.006f, 0.002f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(4.76f, 0.58f, 0.39f);
                        break;
                    case 5:
                        // marble
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.830f, 0.791f, 0.753f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(8.51f, 5.57f, 3.95f);
                        break;
                    case 6:
                        // potato
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.764f, 0.613f, 0.213f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(14.27f, 7.23f, 2.04f);
                        break;
                    case 7:
                        // skimmilk
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.815f, 0.813f, 0.682f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(18.42f, 10.44f, 3.50f);
                        break;
                    case 8:
                        // skin1
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.436f, 0.227f, 0.131f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(3.67f, 1.37f, 0.68f);
                        break;
                    case 9:
                        // skin2
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.623f, 0.433f, 0.343f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(4.82f, 1.69f, 1.09f);
                        break;
                    case 10:
                        // wholemilk
                        scene.getMaterial("lteorb_OuterMat")->m_material.baseColorFactor = glm::vec4(0.908f, 0.881f, 0.759f, 0.f);
                        scene.getMaterial("lteorb_OuterMat")->m_material.subsurface = 0.01f; // mfp is in mm but we scale the model by 10
                        scene.getMaterial("lteorb_OuterMat")->m_material.meanFreePath = glm::vec3(10.90f, 6.58f, 2.51f);
                    default:
                        break;
                }
            }
            m_sceneOptions.m_skyColor = glm::vec3(0.f);
            m_sceneOptions.m_environmentMap = false;
        }

        void sceneAsianDragon(GPUContext *gpuContext, RavenScene &scene, const uint32_t sceneMode) {
            // http://graphics.stanford.edu/data/3Dscanrep/
            // https://github.com/mmp/pbrt-v4-scenes?tab=readme-ov-file#sss-dragon
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/asiandragon";
                const std::string SCENE_FILE = "dragon.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = .1f;
                transformation[1][1] = .1f;
                transformation[2][2] = .1f;
                transformation[3][0] = 0.f;
                transformation[3][1] = 0.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation);
                scene.loadScene(&sceneLoader);

                // skin1
                scene.getMaterial("dragon")->m_material.baseColorFactor = glm::vec4(0.436f, 0.227f, 0.131f, 0.f);
                switch (sceneMode) {
                    case 0:
                        scene.getMaterial("dragon")->m_material.subsurface = 0.01; // mfp is in mm but we scale the model by 10
                        break;
                    case 1:
                        scene.getMaterial("dragon")->m_material.subsurface = 0.025; // mfp is in mm but we scale the model by 10
                        break;
                    case 2:
                        scene.getMaterial("dragon")->m_material.subsurface = 0.05; // mfp is in mm but we scale the model by 10
                        break;
                    default:
                        scene.getMaterial("dragon")->m_material.subsurface = 0.1; // mfp is in mm but we scale the model by 10
                        break;
                }
                scene.getMaterial("dragon")->m_material.meanFreePath = glm::vec3(3.67f, 1.37f, 0.68f);
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/background";
                const std::string SCENE_FILE = "background.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 42.f; // 512.f;
                transformation[1][1] = 42.f; // 512.f;
                transformation[2][2] = 42.f; // 512.f;
                transformation[3][0] = 0.f;
                transformation[3][1] = -4.f;
                transformation[3][2] = 0.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 6.f, glm::vec3(0, 1, 0)));
                scene.loadScene(&sceneLoader);
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane_emissive.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 16.f;
                transformation[1][1] = 16.f;
                transformation[2][2] = 16.f;
                transformation[3][0] = -14.f;
                transformation[3][1] = 16.f;
                transformation[3][2] = -33.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), -glm::pi<float>() / 4.f, glm::vec3(1, 0, 0)));
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane_emissive")->setEmission(glm::vec3(1.f), 1.f);
            }
            {
                const std::string SCENE_DIRECTORY = Paths::m_resourceDirectoryPath + "/assets/plane";
                const std::string SCENE_FILE = "plane_emissive.gltf";
                auto transformation = glm::mat4(1.f);
                transformation[0][0] = 16.f;
                transformation[1][1] = 16.f;
                transformation[2][2] = 16.f;
                transformation[3][0] = -11.f;
                transformation[3][1] = 16.f;
                transformation[3][2] = 32.f;
                GLTFLoader sceneLoader(SCENE_DIRECTORY, SCENE_FILE, transformation * glm::rotate(glm::identity<glm::mat4>(), glm::pi<float>() / 4.f, glm::vec3(1, 0, 0)), 1);
                scene.loadScene(&sceneLoader);

                scene.getMaterial("plane_emissive0")->setEmission(glm::vec3(1.f), 16.5f);
            }
            m_sceneOptions.m_skyColor = glm::vec3(0.f);
            m_sceneOptions.m_environmentMap = false;
        }
    };
} // namespace raven