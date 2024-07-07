// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "fence.h"
#include <stdexcept>

using namespace std;

void Fence::create(VkDevice& device, const bool signaled)
{
    this->device = device;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    fences.resize(Constants::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(device, &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create fence.");
        }
    }
}

void Fence::wait(const size_t frame)
{
    vkWaitForFences(device, 1, &fences[frame], VK_TRUE, UINT64_MAX);
}

void Fence::reset(const size_t frame)
{
    vkResetFences(device, 1, &fences[frame]);
}

void Fence::destroy()
{
    for (const VkFence& fence : fences) {
        vkDestroyFence(device, fence, nullptr);
    }
}

VkFence& Fence::get(const size_t frame)
{
    return fences[frame];
}
