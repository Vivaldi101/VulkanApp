#include <stdlib.h>

#include "platform.h"

// TODO: Use the push buffer style macros from a single vulkan object pool
void* allocate(size_t size)
{
	return malloc(size);
}

// TODO: Use this
static void VulkanRecordCommandBuffer(VulkanContext* context, u32 imageIndex)
{
    Pre(context);
}

void VulkanRender(VulkanContext* context)
{
    Pre(context);
    if (!VK_VALID(vkWaitForFences(context->logicalDevice, 1, &context->fenceFrame, VK_TRUE, UINT64_MAX)))
		Post(0);

	if (!VK_VALID(vkResetFences(context->logicalDevice, 1, &context->fenceFrame)))
        Post(0);

    u32 nextImageIndex = 0;
    if (!VK_VALID(vkAcquireNextImageKHR(context->logicalDevice, context->swapChain, UINT64_MAX, context->semaphoreImageAvailable, VK_NULL_HANDLE, &nextImageIndex)))
        ;

	if (!VK_VALID(vkResetCommandBuffer(context->drawCmdBuffer, 0)))
		Post(0);

    {
        VkCommandBufferBeginInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            //.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .flags = 0, .pInheritanceInfo = 0, .pNext = 0,
        };
        // start this cmd buffer
        if (!VK_VALID(vkBeginCommandBuffer(context->drawCmdBuffer, &info)))
            Post(0);
    }
    {
        // temporary float that oscilates between 0 and 1
        // to gradually change the color on the screen
        static float aa = 0.0f;
        // slowly increment
        aa += 0.011f;
        // when value reaches 1.0 reset to 0
        if (aa >= 1.0) aa = 0;
        // activate render pass:
        // clear color (r,g,b,a)
        VkClearValue clearValue[] = 
        { 
            { 0.0f, aa, aa, 1.0f },	// color
			{ 1.0, 0.0 }				// depth stencil
        }; 

        // define render pass structure
        VkRenderPassBeginInfo info = 
        {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = context->renderPass,
            .framebuffer = context->frameBuffers[nextImageIndex],
            .renderArea.offset = {0,0},
            .renderArea.extent = {context->surfaceWidth, context->surfaceHeight},
        };
        VkOffset2D a = { 0, 0 };
        VkExtent2D b = { context->surfaceWidth, context->surfaceHeight };
        VkRect2D c = { a,b };
        info.renderArea = c;
        info.clearValueCount = 2;
        info.pClearValues = clearValue;

		vkCmdBeginRenderPass(context->drawCmdBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        {
			// draw cmds
        }
        vkCmdEndRenderPass(context->drawCmdBuffer);
    }

	// end this cmd buffer
    if (!VK_VALID(vkEndCommandBuffer(context->drawCmdBuffer)))
        Post(0);

    {
        const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo info =
        {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &context->semaphoreImageAvailable,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &context->drawCmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &context->semaphoreRenderFinished,
        };

        if (!VK_VALID(vkQueueSubmit(context->queue, 1, &info, context->fenceFrame)))
			Post(0);
    }
    // present the image on the screen (flip the swap-chain image)
    {
        VkPresentInfoKHR info =
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &context->semaphoreRenderFinished,
            .swapchainCount = 1,
            .pSwapchains = &context->swapChain,
            .pImageIndices = &nextImageIndex,
            .pResults = NULL,
        };
        if (!VK_VALID(vkQueuePresentKHR(context->queue, &info)))
			;
    }
}

static void VulkanReset(VulkanContext* context)
{
    Pre(context);
    Pre(context->instance);
    // TODO: Clear all teh resources like vulkan image views framebuffers, fences and semaphores etc.
    vkDestroyInstance(context->instance, 0);
}

VulkanContext vulkanInitialize(OSXPlatformWindow* platformWindow)
{
    return OSXVulkanInitialize(platformWindow);
}

void vulkanDeinitialize(VulkanContext* context)
{
    VulkanReset(context);
}
