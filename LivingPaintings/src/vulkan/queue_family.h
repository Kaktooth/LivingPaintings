#pragma once
#include "surface.h"
#include <optional>
#include <stdexcept>
#include <vector>

struct QueueFamily {
    struct Indices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> computeFamily;

        bool isAvailable();
    } indicies;

    QueueFamily::Indices findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& surface);
};
