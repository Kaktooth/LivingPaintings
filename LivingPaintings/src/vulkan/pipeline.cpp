// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "pipeline.h"

using namespace std;

void Pipeline::create(VkDevice& device, VkRenderPass& renderPass,
    VkDescriptorSetLayout& descriptorSetLayout,
    const VkExtent2D extent, VkSampleCountFlagBits samples)
{
    this->device = device;
    this->renderPass = renderPass;
    this->descriptorSetLayout = descriptorSetLayout;
    this->extent = extent;
    this->samples = samples;

    try {
        shaderManager.createShaderModules(device);
    } catch (format_error er) {
        cout << "[Graphics Pipeline] Fail graphics pipeline creation due to failed shader compilation.";
        return;
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderModules {};
    std::vector<VkPipelineShaderStageCreateInfo> computeShaderModules {};
    for (const std::pair<VkShaderStageFlagBits, VkShaderModule_T*>& shaderModule : shaderManager.getShaderModules()) {
        if (shaderModule.first == VK_SHADER_STAGE_COMPUTE_BIT) {
            VkPipelineShaderStageCreateInfo computeShaderModuleInfo {};
            computeShaderModuleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderModuleInfo.stage = shaderModule.first;
            computeShaderModuleInfo.module = shaderModule.second;
            computeShaderModuleInfo.pName = "main";
            computeShaderModuleInfo.pSpecializationInfo = nullptr;
            computeShaderModules.push_back(computeShaderModuleInfo);
        } else {
            VkPipelineShaderStageCreateInfo shaderModuleInfo {};
            shaderModuleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderModuleInfo.stage = shaderModule.first;
            shaderModuleInfo.module = shaderModule.second;
            shaderModuleInfo.pName = "main";
            shaderModuleInfo.pSpecializationInfo = nullptr;
            shaderModules.push_back(shaderModuleInfo);
        }
    }

    // Graphics Pipeline Creation
    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

    const vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportInfo {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    // colorBlending.blendConstants[0] = 0.0f;

    const VkVertexInputBindingDescription bindingDescription = Data::GraphicsObject::Vertex::getBindingDescription();
    const std::vector<VkVertexInputAttributeDescription> attributeDescription = Data::GraphicsObject::Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInfo {};
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.vertexBindingDescriptionCount = 1;
    vertexInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInfo.vertexAttributeDescriptionCount = attributeDescription.size();
    vertexInfo.pVertexAttributeDescriptions = attributeDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = .1f;
    rasterizer.depthBiasClamp = .1f;
    rasterizer.depthBiasSlopeFactor = .1f;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = samples;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = .1f;
    // multisampling.pSampleMask = nullptr;
    // multisampling.alphaToCoverageEnable = VK_FALSE;
    // multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    // pipelineLayoutInfo.pushConstantRangeCount = 0;
    // pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw runtime_error("Failed to create pipeline layout.");
    }
    layouts.push_back(layout);

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.stageCount = shaderModules.size();
    graphicsPipelineInfo.pStages = shaderModules.data();
    graphicsPipelineInfo.pViewportState = &viewportInfo;
    graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
    graphicsPipelineInfo.pColorBlendState = &colorBlending;
    graphicsPipelineInfo.pDepthStencilState = &depthStencil;
    graphicsPipelineInfo.pVertexInputState = &vertexInfo;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineInfo.pRasterizationState = &rasterizer;
    graphicsPipelineInfo.pMultisampleState = &multisampling;
    graphicsPipelineInfo.layout = layout;
    graphicsPipelineInfo.renderPass = renderPass;
    graphicsPipelineInfo.subpass = 0;
    graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineInfo.basePipelineIndex = -1;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw runtime_error("Failed to create graphics pipeline.");
    }
    graphicsPipelines.push_back(graphicsPipeline);

    // Compute Pipeline Creation
    for (size_t i = 0; i < computeShaderModules.size(); i++) {

        VkPipelineLayoutCreateInfo computePipelineLayoutInfo {};
        computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        computePipelineLayoutInfo.setLayoutCount = 1;
        computePipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        VkPipelineLayout computePipelineLayout;
        if (vkCreatePipelineLayout(device, &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            throw runtime_error("Failed to create compute pipeline layout.");
        }
        computePipelineLayouts.push_back(computePipelineLayout);

        VkComputePipelineCreateInfo computePipelineInfo {};
        computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineInfo.layout = computePipelineLayouts[i];
        computePipelineInfo.stage = computeShaderModules[i];

        VkPipeline computePipeline;
        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
                &computePipelineInfo, nullptr,
                &computePipeline)) {
            throw runtime_error("Failed to create compute pipeline.");
        }

        computePipelines.push_back(computePipeline);
    }

    shaderManager.destroyShaderModules();
}

void Pipeline::destroy()
{
    for (size_t i = 0; i < layouts.size(); i++) {
        vkDestroyPipeline(device, graphicsPipelines[i], nullptr);
        vkDestroyPipelineLayout(device, layouts[i], nullptr);
        vkDestroyPipeline(device, computePipelines[i], nullptr);
        vkDestroyPipelineLayout(device, computePipelineLayouts[i], nullptr);
    }
}

bool Pipeline::recreateifShadersChanged()
{
    if (ShaderManager::recreateGraphicsPipeline) {
        Pipeline::create(device, renderPass, descriptorSetLayout, extent,
            samples);
        ShaderManager::recreateGraphicsPipeline = false;
        return true;
    }

    return false;
}

void Pipeline::bind(VkCommandBuffer& cmdCompute, VkDescriptorSet& computeDescriptorSet)
{
    for (size_t i = 0; i < computePipelines.size(); i++) {
        vkCmdBindDescriptorSets(cmdCompute, VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayouts[i], 0, 1,
            &computeDescriptorSet, 0, 0);
        vkCmdBindPipeline(cmdCompute, VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelines[i]);
    }
}

void Pipeline::updateExtent(VkExtent2D& extent)
{
    this->extent = extent;
}

VkPipelineLayout& Pipeline::getLastLayout()
{
    return layouts[layouts.size() - 1];
}

VkPipeline& Pipeline::getLast()
{
    return graphicsPipelines[layouts.size() - 1];
}

VkPipelineLayout& Pipeline::getLayout(const size_t index)
{
    return layouts[index];
}

VkPipeline& Pipeline::get(const size_t index)
{
    return graphicsPipelines[index];
}

size_t Pipeline::getPipelineHistorySize() { return graphicsPipelines.size(); }
