#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <libplacebo/context.h>
#include <libplacebo/vulkan.h>
#include <vulkan/vulkan.h>

int main()
{
    VkResult res;
    VkSurfaceKHR surf = NULL;
    const struct pl_vulkan *vk = NULL;
    const struct pl_vk_inst *vkinst = NULL;
    const struct pl_swapchain *swch = NULL;

    struct pl_context *ctx;
    ctx = pl_context_create(PL_API_VER, &(struct pl_context_params) {
        .log_level  = PL_LOG_DEBUG,
        .log_cb     = pl_log_color,
        .log_priv   = stdout,
    });

    vkinst = pl_vk_inst_create(ctx, &(struct pl_vk_inst_params) {
        .debug = false,
        .extensions = (const char *[]) {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_DISPLAY_EXTENSION_NAME
        },
        .num_extensions = 2,
    });

    if (!vkinst) {
        fprintf(stderr, "Failed initializing vulkan instance\n");
        return 1;
    }

    VkPhysicalDevice physd = NULL;
    vkEnumeratePhysicalDevices(vkinst->instance, &(uint32_t){1}, &physd);
    if (!physd) {
        fprintf(stderr, "No VkPhysicalDevice?\n");
        goto cleanup;
    }

    uint32_t num;
    vkGetPhysicalDeviceDisplayPropertiesKHR(physd, &num, NULL);
    VkDisplayPropertiesKHR *dprops = calloc(num, sizeof(*dprops));
    vkGetPhysicalDeviceDisplayPropertiesKHR(physd, &num, dprops);

    printf("Number of displays: %d\n", num);
    for (int i = 0; i < num; i++) {
        VkDisplayPropertiesKHR *props = &dprops[i];
        printf("Display %d:\n", i);
        printf("    Name: %s\n", props->displayName);
        printf("    Resolution: %d x %d\n", props->physicalResolution.width,
                                            props->physicalResolution.height);
        printf("    Reorderable: %s\n", props->planeReorderPossible ? "true" : "false");
        printf("    Persistent: %s\n", props->persistentContent ? "true" : "false");
    }

    if (num == 0) {
        free(dprops);
        goto cleanup;
    }

    VkDisplayKHR disp = dprops[0].display;
    free(dprops);

    vkGetDisplayModePropertiesKHR(physd, disp, &num, NULL);
    VkDisplayModePropertiesKHR *mprops = calloc(num, sizeof(*mprops));
    vkGetDisplayModePropertiesKHR(physd, disp, &num, mprops);

    printf("Number of modes: %d\n", num);
    for (int i = 0; i < num; i++) {
        VkDisplayModeParametersKHR *params = &mprops[i].parameters;
        printf("Mode %d: %d x %d (%.2f fps)\n", i,
               params->visibleRegion.width, params->visibleRegion.height,
               (float) params->refreshRate / 1000.0f);

    }

    if (num == 0) {
        free(mprops);
        goto cleanup;
    }

    VkDisplayModeKHR mode = mprops[0].displayMode;
    free(mprops);

    VkDisplaySurfaceCreateInfoKHR sinfo = {
        .sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
        .displayMode = mode,
        .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
    };

    res = vkCreateDisplayPlaneSurfaceKHR(vkinst->instance, &sinfo, NULL, &surf);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "Failed initializing surface!\n");
        goto cleanup;
    }

    struct pl_vulkan_params vpars = pl_vulkan_default_params;
    vpars.instance = vkinst->instance;
    vpars.surface = surf;
    vpars.device = physd;
    vk = pl_vulkan_create(ctx, &vpars);
    if (!vk) {
        fprintf(stderr, "Failed creating vulkan context!\n");
        goto cleanup;
    }

    swch = pl_vulkan_create_swapchain(vk, &(struct pl_vulkan_swapchain_params) {
        .surface = surf,
        .present_mode = VK_PRESENT_MODE_FIFO_KHR,
    });

    if (!swch) {
        fprintf(stderr, "Failed creating vulkan context!\n");
        goto cleanup;
    }

    const struct pl_gpu *gpu = vk->gpu;
    const int frames = 120;
    printf("Rendering %d frames\n", frames);
    for (int i = 0; i < frames; i++) {
        struct pl_swapchain_frame frame;
        if (!pl_swapchain_start_frame(swch, &frame))
            continue;

        pl_tex_clear(gpu, frame.fbo, (float[4]){
            (float) i / frames,
            1.0 - (float) i / frames,
            0.1,
            1.0
        });

        if (!pl_swapchain_submit_frame(swch)) {
            fprintf(stderr, "Failed rendering frame!\n");
            break;
        }

        pl_swapchain_swap_buffers(swch);
    }

    printf("Done rendering.. cleanup\n");
    pl_gpu_finish(gpu);

cleanup:
    pl_swapchain_destroy(&swch);
    pl_vulkan_destroy(&vk);
    if (vkinst)
        vkDestroySurfaceKHR(vkinst->instance, surf, NULL);
    pl_vk_inst_destroy(&vkinst);
}
