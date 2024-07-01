// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "framebuffer.h"
#include <stdexcept>

using namespace std;

std::vector<VkFramebuffer> framebuffers;

void Framebuffer::create(VkDevice device, vector<VkImageView> imageViews, VkExtent2D extent, VkRenderPass renderPass)
{
    framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = {
            imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create framebuffer.");
        }
    }
}

void Framebuffer::destroy(VkDevice device)
{
    for (const auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

std::vector<VkFramebuffer> Framebuffer::get()
{
    return framebuffers;
}
