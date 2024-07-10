// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "graphics_pipeline.h"

using namespace std;

void GraphicsPipeline::create(VkDevice& device, VkRenderPass& renderPass,
    VkDescriptorSetLayout& descriptorSetLayout,
    const VkExtent2D extent)
{
    this->device = device;
    this->renderPass = renderPass;
    this->descriptorSetLayout = descriptorSetLayout;
    this->extent = extent;

    try {
        shaderManager.createShaderModules(device);
    } catch (format_error er) {
        cout << "[Graphics Pipeline] Fail graphics pipeline creation due to failed shader compilation.";
        return;
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderModuleInfos {};
    for (const std::pair<VkShaderStageFlagBits, VkShaderModule>& shaderModule : shaderManager.getShaderModules()) {
        VkPipelineShaderStageCreateInfo shaderModuleInfo {};
        shaderModuleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderModuleInfo.stage = shaderModule.first;
        shaderModuleInfo.module = shaderModule.second;
        shaderModuleInfo.pName = "main";
        shaderModuleInfo.pSpecializationInfo = nullptr;
        shaderModuleInfos.push_back(shaderModuleInfo);
    }

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    /*rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;*/

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // multisampling.minSampleShading = 1.0f;
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
    graphicsPipelineInfo.stageCount = shaderModuleInfos.size();
    graphicsPipelineInfo.pStages = shaderModuleInfos.data();
    graphicsPipelineInfo.pViewportState = &viewportInfo;
    graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
    graphicsPipelineInfo.pColorBlendState = &colorBlending;
    graphicsPipelineInfo.pVertexInputState = &vertexInfo;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineInfo.pRasterizationState = &rasterizer;
    graphicsPipelineInfo.pMultisampleState = &multisampling;
    graphicsPipelineInfo.pDepthStencilState = nullptr;
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

    shaderManager.destroyShaderModules();
}

void GraphicsPipeline::destroy()
{
    for (int i = 0; i < layouts.size(); i++) {
        vkDestroyPipeline(device, graphicsPipelines[i], nullptr);
        vkDestroyPipelineLayout(device, layouts[i], nullptr);
    }
}

bool GraphicsPipeline::recreateifShadersChanged()
{
    if (ShaderManager::recreateGraphicsPipeline) {
        GraphicsPipeline::create(device, renderPass, descriptorSetLayout,
            extent);
        ShaderManager::recreateGraphicsPipeline = false;
        return true;
    }

    return false;
}

VkPipelineLayout& GraphicsPipeline::getLastLayout()
{
    return layouts[layouts.size() - 1];
}

VkPipeline& GraphicsPipeline::getLast()
{
    return graphicsPipelines[layouts.size() - 1];
}

VkPipelineLayout& GraphicsPipeline::getLayout(const size_t index)
{
    return layouts[index];
}

VkPipeline& GraphicsPipeline::get(const size_t index)
{
    return graphicsPipelines[index];
}

size_t GraphicsPipeline::getPipelineHistorySize() { return graphicsPipelines.size(); }
