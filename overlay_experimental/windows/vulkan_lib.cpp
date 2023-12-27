#include "Vulkan_Hook.h"
#include "vulkan/vulkan.h"
#define VULKAN_DLL (Vulkan_Hook::DLL_NAME)

void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetPhysicalDeviceMemoryProperties) *fun = (decltype(vkGetPhysicalDeviceMemoryProperties) *)GetProcAddress(vulkan_lib, "vkGetPhysicalDeviceMemoryProperties");
            if (fun) {
                return fun(physicalDevice, pMemoryProperties);
            }
        }
    }


VkResult VKAPI_CALL vkDeviceWaitIdle(
    VkDevice                                    device) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDeviceWaitIdle) *fun = (decltype(vkDeviceWaitIdle) *)GetProcAddress(vulkan_lib, "vkDeviceWaitIdle");
            if (fun) {
                return fun(device);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkAllocateMemory) *fun = (decltype(vkAllocateMemory) *)GetProcAddress(vulkan_lib, "vkAllocateMemory");
            if (fun) {
                return fun(device, pAllocateInfo, pAllocator, pMemory);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkFreeMemory) *fun = (decltype(vkFreeMemory) *)GetProcAddress(vulkan_lib, "vkFreeMemory");
            if (fun) {
                return fun(device, memory, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkMapMemory) *fun = (decltype(vkMapMemory) *)GetProcAddress(vulkan_lib, "vkMapMemory");
            if (fun) {
                return fun(device, memory, offset, size, flags, ppData);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkUnmapMemory) *fun = (decltype(vkUnmapMemory) *)GetProcAddress(vulkan_lib, "vkUnmapMemory");
            if (fun) {
                return fun(device, memory);
            }
        }
    }


VkResult VKAPI_CALL vkFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkFlushMappedMemoryRanges) *fun = (decltype(vkFlushMappedMemoryRanges) *)GetProcAddress(vulkan_lib, "vkFlushMappedMemoryRanges");
            if (fun) {
                return fun(device, memoryRangeCount, pMemoryRanges);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkBindBufferMemory) *fun = (decltype(vkBindBufferMemory) *)GetProcAddress(vulkan_lib, "vkBindBufferMemory");
            if (fun) {
                return fun(device, buffer, memory, memoryOffset);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkBindImageMemory) *fun = (decltype(vkBindImageMemory) *)GetProcAddress(vulkan_lib, "vkBindImageMemory");
            if (fun) {
                return fun(device, image, memory, memoryOffset);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetBufferMemoryRequirements) *fun = (decltype(vkGetBufferMemoryRequirements) *)GetProcAddress(vulkan_lib, "vkGetBufferMemoryRequirements");
            if (fun) {
                return fun(device, buffer, pMemoryRequirements);
            }
        }
    }


void VKAPI_CALL vkGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetImageMemoryRequirements) *fun = (decltype(vkGetImageMemoryRequirements) *)GetProcAddress(vulkan_lib, "vkGetImageMemoryRequirements");
            if (fun) {
                return fun(device, image, pMemoryRequirements);
            }
        }
    }


VkResult VKAPI_CALL vkCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateFence) *fun = (decltype(vkCreateFence) *)GetProcAddress(vulkan_lib, "vkCreateFence");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pFence);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyFence) *fun = (decltype(vkDestroyFence) *)GetProcAddress(vulkan_lib, "vkDestroyFence");
            if (fun) {
                return fun(device, fence, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateSemaphore) *fun = (decltype(vkCreateSemaphore) *)GetProcAddress(vulkan_lib, "vkCreateSemaphore");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pSemaphore);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroySemaphore) *fun = (decltype(vkDestroySemaphore) *)GetProcAddress(vulkan_lib, "vkDestroySemaphore");
            if (fun) {
                return fun(device, semaphore, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateBuffer) *fun = (decltype(vkCreateBuffer) *)GetProcAddress(vulkan_lib, "vkCreateBuffer");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pBuffer);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyBuffer) *fun = (decltype(vkDestroyBuffer) *)GetProcAddress(vulkan_lib, "vkDestroyBuffer");
            if (fun) {
                return fun(device, buffer, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateImage) *fun = (decltype(vkCreateImage) *)GetProcAddress(vulkan_lib, "vkCreateImage");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pImage);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyImage) *fun = (decltype(vkDestroyImage) *)GetProcAddress(vulkan_lib, "vkDestroyImage");
            if (fun) {
                return fun(device, image, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateImageView) *fun = (decltype(vkCreateImageView) *)GetProcAddress(vulkan_lib, "vkCreateImageView");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pView);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyImageView) *fun = (decltype(vkDestroyImageView) *)GetProcAddress(vulkan_lib, "vkDestroyImageView");
            if (fun) {
                return fun(device, imageView, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateShaderModule) *fun = (decltype(vkCreateShaderModule) *)GetProcAddress(vulkan_lib, "vkCreateShaderModule");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pShaderModule);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyShaderModule) *fun = (decltype(vkDestroyShaderModule) *)GetProcAddress(vulkan_lib, "vkDestroyShaderModule");
            if (fun) {
                return fun(device, shaderModule, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateGraphicsPipelines) *fun = (decltype(vkCreateGraphicsPipelines) *)GetProcAddress(vulkan_lib, "vkCreateGraphicsPipelines");
            if (fun) {
                return fun(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyPipeline) *fun = (decltype(vkDestroyPipeline) *)GetProcAddress(vulkan_lib, "vkDestroyPipeline");
            if (fun) {
                return fun(device, pipeline, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreatePipelineLayout) *fun = (decltype(vkCreatePipelineLayout) *)GetProcAddress(vulkan_lib, "vkCreatePipelineLayout");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pPipelineLayout);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyPipelineLayout) *fun = (decltype(vkDestroyPipelineLayout) *)GetProcAddress(vulkan_lib, "vkDestroyPipelineLayout");
            if (fun) {
                return fun(device, pipelineLayout, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateSampler) *fun = (decltype(vkCreateSampler) *)GetProcAddress(vulkan_lib, "vkCreateSampler");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pSampler);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroySampler) *fun = (decltype(vkDestroySampler) *)GetProcAddress(vulkan_lib, "vkDestroySampler");
            if (fun) {
                return fun(device, sampler, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateDescriptorSetLayout) *fun = (decltype(vkCreateDescriptorSetLayout) *)GetProcAddress(vulkan_lib, "vkCreateDescriptorSetLayout");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pSetLayout);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyDescriptorSetLayout) *fun = (decltype(vkDestroyDescriptorSetLayout) *)GetProcAddress(vulkan_lib, "vkDestroyDescriptorSetLayout");
            if (fun) {
                return fun(device, descriptorSetLayout, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkAllocateDescriptorSets) *fun = (decltype(vkAllocateDescriptorSets) *)GetProcAddress(vulkan_lib, "vkAllocateDescriptorSets");
            if (fun) {
                return fun(device, pAllocateInfo, pDescriptorSets);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkUpdateDescriptorSets) *fun = (decltype(vkUpdateDescriptorSets) *)GetProcAddress(vulkan_lib, "vkUpdateDescriptorSets");
            if (fun) {
                return fun(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
            }
        }
    }


VkResult VKAPI_CALL vkCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateFramebuffer) *fun = (decltype(vkCreateFramebuffer) *)GetProcAddress(vulkan_lib, "vkCreateFramebuffer");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pFramebuffer);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyFramebuffer) *fun = (decltype(vkDestroyFramebuffer) *)GetProcAddress(vulkan_lib, "vkDestroyFramebuffer");
            if (fun) {
                return fun(device, framebuffer, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateRenderPass) *fun = (decltype(vkCreateRenderPass) *)GetProcAddress(vulkan_lib, "vkCreateRenderPass");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pRenderPass);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyRenderPass) *fun = (decltype(vkDestroyRenderPass) *)GetProcAddress(vulkan_lib, "vkDestroyRenderPass");
            if (fun) {
                return fun(device, renderPass, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateCommandPool) *fun = (decltype(vkCreateCommandPool) *)GetProcAddress(vulkan_lib, "vkCreateCommandPool");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pCommandPool);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroyCommandPool) *fun = (decltype(vkDestroyCommandPool) *)GetProcAddress(vulkan_lib, "vkDestroyCommandPool");
            if (fun) {
                return fun(device, commandPool, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkAllocateCommandBuffers) *fun = (decltype(vkAllocateCommandBuffers) *)GetProcAddress(vulkan_lib, "vkAllocateCommandBuffers");
            if (fun) {
                return fun(device, pAllocateInfo, pCommandBuffers);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkFreeCommandBuffers) *fun = (decltype(vkFreeCommandBuffers) *)GetProcAddress(vulkan_lib, "vkFreeCommandBuffers");
            if (fun) {
                return fun(device, commandPool, commandBufferCount, pCommandBuffers);
            }
        }
    }


void VKAPI_CALL vkCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdBindPipeline) *fun = (decltype(vkCmdBindPipeline) *)GetProcAddress(vulkan_lib, "vkCmdBindPipeline");
            if (fun) {
                return fun(commandBuffer, pipelineBindPoint, pipeline);
            }
        }
    }


void VKAPI_CALL vkCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdSetViewport) *fun = (decltype(vkCmdSetViewport) *)GetProcAddress(vulkan_lib, "vkCmdSetViewport");
            if (fun) {
                return fun(commandBuffer, firstViewport, viewportCount, pViewports);
            }
        }
    }


void VKAPI_CALL vkCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdSetScissor) *fun = (decltype(vkCmdSetScissor) *)GetProcAddress(vulkan_lib, "vkCmdSetScissor");
            if (fun) {
                return fun(commandBuffer, firstScissor, scissorCount, pScissors);
            }
        }
    }


void VKAPI_CALL vkCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdBindDescriptorSets) *fun = (decltype(vkCmdBindDescriptorSets) *)GetProcAddress(vulkan_lib, "vkCmdBindDescriptorSets");
            if (fun) {
                return fun(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
            }
        }
    }


void VKAPI_CALL vkCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdBindIndexBuffer) *fun = (decltype(vkCmdBindIndexBuffer) *)GetProcAddress(vulkan_lib, "vkCmdBindIndexBuffer");
            if (fun) {
                return fun(commandBuffer, buffer, offset, indexType);
            }
        }
    }


void VKAPI_CALL vkCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdBindVertexBuffers) *fun = (decltype(vkCmdBindVertexBuffers) *)GetProcAddress(vulkan_lib, "vkCmdBindVertexBuffers");
            if (fun) {
                return fun(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
            }
        }
    }


void VKAPI_CALL vkCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdDraw) *fun = (decltype(vkCmdDraw) *)GetProcAddress(vulkan_lib, "vkCmdDraw");
            if (fun) {
                return fun(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
            }
        }
    }


void VKAPI_CALL vkCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdDrawIndexed) *fun = (decltype(vkCmdDrawIndexed) *)GetProcAddress(vulkan_lib, "vkCmdDrawIndexed");
            if (fun) {
                return fun(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            }
        }
    }


void VKAPI_CALL vkCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdCopyBuffer) *fun = (decltype(vkCmdCopyBuffer) *)GetProcAddress(vulkan_lib, "vkCmdCopyBuffer");
            if (fun) {
                return fun(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
            }
        }
    }


void VKAPI_CALL vkCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdCopyBufferToImage) *fun = (decltype(vkCmdCopyBufferToImage) *)GetProcAddress(vulkan_lib, "vkCmdCopyBufferToImage");
            if (fun) {
                return fun(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
            }
        }
    }


void VKAPI_CALL vkCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdPipelineBarrier) *fun = (decltype(vkCmdPipelineBarrier) *)GetProcAddress(vulkan_lib, "vkCmdPipelineBarrier");
            if (fun) {
                return fun(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
            }
        }
    }


void VKAPI_CALL vkCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCmdPushConstants) *fun = (decltype(vkCmdPushConstants) *)GetProcAddress(vulkan_lib, "vkCmdPushConstants");
            if (fun) {
                return fun(commandBuffer, layout, stageFlags, offset, size, pValues);
            }
        }
    }


void VKAPI_CALL vkDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroySurfaceKHR) *fun = (decltype(vkDestroySurfaceKHR) *)GetProcAddress(vulkan_lib, "vkDestroySurfaceKHR");
            if (fun) {
                return fun(instance, surface, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) *fun = (decltype(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) *)GetProcAddress(vulkan_lib, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
            if (fun) {
                return fun(physicalDevice, surface, pSurfaceCapabilities);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetPhysicalDeviceSurfaceFormatsKHR) *fun = (decltype(vkGetPhysicalDeviceSurfaceFormatsKHR) *)GetProcAddress(vulkan_lib, "vkGetPhysicalDeviceSurfaceFormatsKHR");
            if (fun) {
                return fun(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetPhysicalDeviceSurfacePresentModesKHR) *fun = (decltype(vkGetPhysicalDeviceSurfacePresentModesKHR) *)GetProcAddress(vulkan_lib, "vkGetPhysicalDeviceSurfacePresentModesKHR");
            if (fun) {
                return fun(physicalDevice, surface, pPresentModeCount, pPresentModes);
            }
        }
    
    return VK_NOT_READY;
}


VkResult VKAPI_CALL vkCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkCreateSwapchainKHR) *fun = (decltype(vkCreateSwapchainKHR) *)GetProcAddress(vulkan_lib, "vkCreateSwapchainKHR");
            if (fun) {
                return fun(device, pCreateInfo, pAllocator, pSwapchain);
            }
        }
    
    return VK_NOT_READY;
}


void VKAPI_CALL vkDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkDestroySwapchainKHR) *fun = (decltype(vkDestroySwapchainKHR) *)GetProcAddress(vulkan_lib, "vkDestroySwapchainKHR");
            if (fun) {
                return fun(device, swapchain, pAllocator);
            }
        }
    }


VkResult VKAPI_CALL vkGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) {

        HMODULE vulkan_lib = GetModuleHandleA(VULKAN_DLL);
        if (vulkan_lib) {
            decltype(vkGetSwapchainImagesKHR) *fun = (decltype(vkGetSwapchainImagesKHR) *)GetProcAddress(vulkan_lib, "vkGetSwapchainImagesKHR");
            if (fun) {
                return fun(device, swapchain, pSwapchainImageCount, pSwapchainImages);
            }
        }
    
    return VK_NOT_READY;
}

