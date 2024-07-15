#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "raven/RavenInclude.h"
#if defined(WIN32)
// #include <libloaderapi.h>
#include <windows.h>
#undef MemoryBarrier
#endif

#include "SPIRV-Reflect/spirv_reflect.h"
#include "Uniform.h"
#include "raven/scene/loader/GLTFLoader.h"

namespace raven {
    class Shader {
    public:
        struct DescriptorSetLayoutData {
            uint32_t set_number{};
            vk::DescriptorSetLayoutCreateInfo create_info{};
            std::vector<vk::DescriptorSetLayoutBinding> bindings;
            std::map<uint32_t, uint32_t> bindingToIndex;

            void generateBindingToIndexMap() {
                for (uint32_t i = 0; i < bindings.size(); i++) {
                    const auto &binding = bindings[i];
                    bindingToIndex[binding.binding] = i;
                }
            }
        };

        Shader(GPUContext *gpuContext, const std::string &inputPath, const std::string &fileName, vk::ShaderStageFlagBits shaderStage, vk::Extent3D workGroupSize,
               std::vector<vk::SpecializationMapEntry> specializationMapEntries, std::vector<uint32_t> specializationData) :
            m_gpuContext(gpuContext), m_shaderStage(shaderStage), m_specializationMapEntries(std::move(specializationMapEntries)),
            m_specializationData(std::move(specializationData)), m_workGroupSize(workGroupSize) {
            std::cout << "[Shader] Compiling " << inputPath << "/" << fileName << std::endl;

#if defined(WIN32)
            wchar_t path[FILENAME_MAX] = {0};
            GetModuleFileNameW(nullptr, path, FILENAME_MAX);
            auto exe = std::filesystem::path(path).parent_path();
#else
            auto exe = std::filesystem::canonical("/proc/self/exe").remove_filename();
#endif

            const auto outputPath = std::filesystem::path(exe).append("resources/shaders");
            compileShader(inputPath, outputPath.string(), fileName);
            const std::vector<char> code = readFile(outputPath.string() + "/" + fileName + ".spv");

            reflect(code);

            m_shaderModule = createShaderModule(code, m_gpuContext->m_device);
            gpuContext->getDebug()->setName(m_shaderModule, fileName);
        }

        ~Shader() {
            release();
        }

        void release() {
            if (m_shaderModule) {
                vkDestroyShaderModule(m_gpuContext->m_device, m_shaderModule, nullptr);
            }
            m_shaderModule = nullptr;
        }

        void updateSpecializationConstants(std::vector<vk::SpecializationMapEntry> specializationMapEntries, std::vector<uint32_t> specializationData) {
            m_specializationMapEntries = std::move(specializationMapEntries);
            m_specializationData = std::move(specializationData);
        }

        vk::PipelineShaderStageCreateInfo generateShaderStageCreateInfo() {
            assert(m_workGroupSize.width > 0 && m_workGroupSize.height > 0 && m_workGroupSize.depth > 0 && "Workgroup size not set!");

            m_specializationInfo = vk::SpecializationInfo{static_cast<uint32_t>(m_specializationMapEntries.size()), m_specializationMapEntries.data(), m_specializationData.size() * sizeof(uint32_t), m_specializationData.data()};

            vk::PipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.stage = m_shaderStage;
            shaderStageInfo.module = m_shaderModule;
            shaderStageInfo.pName = "main";
            shaderStageInfo.pSpecializationInfo = &m_specializationInfo;
            return shaderStageInfo;
        }

        std::map<uint32_t, DescriptorSetLayoutData> *getDescriptorSetLayoutData() {
            return &m_descriptorSetLayoutData;
        }

        std::map<uint32_t, std::vector<std::shared_ptr<Uniform>>> *getUniforms() {
            return &m_uniforms;
        }

        [[nodiscard]] vk::Extent3D getWorkGroupSize() const {
            return m_workGroupSize;
        }

    private:
        GPUContext *m_gpuContext;
        vk::ShaderModule m_shaderModule;
        vk::ShaderStageFlagBits m_shaderStage;

        std::map<uint32_t, DescriptorSetLayoutData> m_descriptorSetLayoutData;
        std::map<uint32_t, std::vector<std::shared_ptr<Uniform>>> m_uniforms;

        std::vector<vk::SpecializationMapEntry> m_specializationMapEntries;
        std::vector<uint32_t> m_specializationData;
        vk::SpecializationInfo m_specializationInfo;
        vk::Extent3D m_workGroupSize = {0, 0, 0};

        static vk::ShaderModule createShaderModule(const std::vector<char> &code, vk::Device device) {
            vk::ShaderModuleCreateInfo createInfo{};
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

            vk::ShaderModule shaderModule;
            if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create shader module!");
            }

            return shaderModule;
        }

        static std::vector<char> readFile(const std::string &filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file!");
            }
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
            file.close();
            return buffer;
        }

        static void compileShader(const std::string &inputPath, const std::string &outputPath, const std::string &fileName) {
            std::filesystem::create_directories(outputPath);

            std::stringstream cmd;
            //            cmd << "glslc --target-spv=spv1.6 " << inputPath << "/" << fileName << " -o " << outputPath << "/" << fileName << "_compiled.spv";
            cmd << "glslangValidator -V --target-env spirv1.6 " << inputPath << "/" << fileName << " -o " << outputPath << "/" << fileName << ".spv";

            std::string cmd_output;
            char read_buffer[1024];
#ifdef WIN32
            FILE *cmd_stream = _popen(cmd.str().c_str(), "r");
#else
            FILE *cmd_stream = popen(cmd.str().c_str(), "r");
#endif
            while (fgets(read_buffer, sizeof(read_buffer), cmd_stream))
                cmd_output += read_buffer;
#ifdef WIN32
            int cmd_ret = _pclose(cmd_stream);
#else
            int cmd_ret = pclose(cmd_stream);
#endif

            if (cmd_ret != 0) {
                //                std::cerr << cmd_output << std::endl;
                throw std::runtime_error("[Shader] Compile error: " + cmd_output);
            }
        }

        void reflect(const std::vector<char> &code) {
            SpvReflectShaderModule module = {};
            SpvReflectResult result =
                    spvReflectCreateShaderModule(sizeof(code[0]) * code.size(), code.data(), &module);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            reflectDescriptorSetLayout(module);
            // reflectWorkGroupSize(module);

            spvReflectDestroyShaderModule(&module);
        }

        void reflectDescriptorSetLayout(const SpvReflectShaderModule &module) {
            uint32_t count = 0;
            SpvReflectResult result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorSet *> sets(count);
            result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            for (const auto &reflectedSet: sets) {
                m_descriptorSetLayoutData[reflectedSet->set] = {};
                DescriptorSetLayoutData &layout = m_descriptorSetLayoutData[reflectedSet->set];

                layout.bindings.resize(reflectedSet->binding_count);
                for (uint32_t i_binding = 0; i_binding < reflectedSet->binding_count; ++i_binding) {
                    const SpvReflectDescriptorBinding &refl_binding = *(reflectedSet->bindings[i_binding]);
                    vk::DescriptorSetLayoutBinding &layout_binding = layout.bindings[i_binding];
                    layout_binding.binding = refl_binding.binding;
                    layout_binding.descriptorType = static_cast<vk::DescriptorType>(refl_binding.descriptor_type);
                    layout_binding.descriptorCount = 1;
                    for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                        layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                    }
                    layout_binding.stageFlags = static_cast<vk::ShaderStageFlagBits>(module.shader_stage);

                    if (layout_binding.descriptorType == vk::DescriptorType::eUniformBuffer) {
                        // reflect uniform
                        m_uniforms[reflectedSet->set].push_back(Uniform::reflect(m_gpuContext, refl_binding));
                    }
                }
                layout.set_number = reflectedSet->set;
                layout.create_info.bindingCount = reflectedSet->binding_count;
                layout.create_info.pBindings = layout.bindings.data();
            }
        }

        // void reflectWorkGroupSize(const SpvReflectShaderModule &module) {
        //     auto entryPoint = spvReflectGetEntryPoint(&module, "main");
        //     if (entryPoint != nullptr) {
        //         m_workGroupSize = vk::Extent3D{entryPoint->local_size.x, entryPoint->local_size.y, entryPoint->local_size.z};
        //     }
        // }
    };
} // namespace raven