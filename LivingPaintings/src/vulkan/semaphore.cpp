#include "semaphore.h"

void Semaphore::create(VkDevice& device)
{
    this->device = device;

    semaphores.resize(Constants::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semafore.");
        }
    }
}

void Semaphore::destroy()
{
    for (VkSemaphore& semaphore : semaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
}

std::vector<VkSemaphore>& Semaphore::get()
{
    return semaphores;
}

VkSemaphore& Semaphore::get(const size_t frame)
{
    return semaphores[frame];
}
