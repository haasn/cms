#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include <config.h>
#include <libplacebo/context.h>
#include <libplacebo/vulkan.h>
#include <libplacebo/dispatch.h>
#include <libplacebo/shaders/av1.h>
#include <libplacebo/shaders/sampling.h>
#include <libplacebo/shaders/colorspace.h>
#include <libplacebo/gpu.h>

int main()
{
    int width, height;
    char type;
    if (scanf("P%c\n%d %d\n-1.0\n", &type, &width, &height) != 3 ||
        toupper(type) != 'F')
    {
        fprintf(stderr, "Got unexpected file header? Input must be PF format\n");
        return 1;
    }

    // basic sanitization
    if (width <= 0 || height <= 0 || width > 1<<20 || height > 1<<20) {
        fprintf(stderr, "Got nonsensical dimensions %dx%d!\n", width, height);
        return 1;
    }

    struct pl_context *ctx;
    ctx = pl_context_create(PL_API_VER, &(struct pl_context_params) {
        .log_cb     = pl_log_color,
        .log_level  = PL_LOG_DEBUG,
        .log_priv   = stderr,
    });

    struct pl_vulkan_params vk_params = pl_vulkan_default_params;
    const struct pl_vulkan *vk = pl_vulkan_create(ctx, &vk_params);

    if (!vk) {
        fprintf(stderr, "Failed initializing vulkan context\n");
        return 3;
    }

    const struct pl_gpu *gpu = vk->gpu;

    const struct pl_fmt *fmt;
    fmt = pl_find_fmt(gpu, PL_FMT_FLOAT, 4, 16, 32, PL_FMT_CAP_SAMPLEABLE |
                      PL_FMT_CAP_RENDERABLE | PL_FMT_CAP_STORABLE);
    if (!fmt) {
        fprintf(stderr, "GPU does not support 32-bit RGBA textures!\n");
        return 3;
    }

    // Compute the optimal stride, simply because we can
    int stride_w = pl_optimal_transfer_stride(gpu, width);
    char *buf = malloc(stride_w * height * fmt->texel_size);
    if (!buf) {
        fprintf(stderr, "Failed allocating memory for image buffer!\n");
        return 2;
    }

    // Fill the buffer with the input data
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * stride_w + x;
            float *texel = (float *) &buf[idx * fmt->texel_size];

            int num = 0;
            switch (type) {
            case 'F': num = 3; break; // RGB
            case 'f': num = 1; break; // grayscale
            }

            if (fread(texel, sizeof(float), num, stdin) != num) {
                fprintf(stderr, "Expected %d floats, got fewer!\n", num);
                return 1;
            }

            if (type == 'f') // grayscale
                texel[2] = texel[1] = texel[0];

            // Explicitly clear the alpha channel
            texel[3] = 1.0;
        }
    }

    // Create the source and destination textures
    const struct pl_tex *src, *dst, *inter;
    src = pl_tex_create(gpu, &(struct pl_tex_params) {
        .format         = fmt,
        .w              = width,
        .h              = height,
        .sampleable     = true,
        .host_writable  = true,
        .sample_mode    = PL_TEX_SAMPLE_LINEAR,
    });

    dst = pl_tex_create(gpu, &(struct pl_tex_params) {
        .format         = fmt,
        .w              = width,
        .h              = height,
        .renderable     = true,
        .storable       = true,
        .host_readable  = true,
    });

    /*
    inter = pl_tex_create(gpu, &(struct pl_tex_params) {
        .format         = fmt,
        .w              = src->params.w,
        .h              = dst->params.h,
        .renderable     = true,
        .sampleable     = true,
        .storable       = true,
        .sample_mode    = PL_TEX_SAMPLE_LINEAR,
    });
    */

    if (!src || !dst) {
        fprintf(stderr, "Failed creating render textures!\n");
        return 3;
    }

    // Upload the source data
    struct pl_tex_transfer_params transfer = {
        .tex            = src,
        .stride_w       = stride_w,
        .ptr            = buf,
    };

    if (!pl_tex_upload(gpu, &transfer)) {
        fprintf(stderr, "Failed uploading source data!\n");
        return 4;
    }

    struct pl_dispatch *dp = pl_dispatch_create(ctx, gpu);
    if (!dp)
        return 3;

    struct pl_shader_obj *state = NULL, *dither_state = NULL;

    struct pl_shader *sh = pl_dispatch_begin(dp);
    if (!sh)
        return 3;

    pl_shader_sample_direct(sh, &(struct pl_sample_src) {
        .tex = src,
        .components = 3,
        .rect = { 0, 0, src->params.w, src->params.h },
        .new_w = dst->params.w,
        .new_h = dst->params.h,
    });

    /*
    struct pl_sample_filter_params filter = {
        .filter = pl_filter_lanczos,
        .lut = &state,
    };

    pl_shader_sample_ortho(sh, PL_SEP_VERT, &(struct pl_sample_src) {
        .tex = src,
        .components = 3,
        .rect = { 0, 0, src->params.w, src->params.h },
        .new_w = dst->params.w,
        .new_h = dst->params.h,
    }, &filter);

    pl_shader_dither(sh, 8, &dither_state, NULL);
    if (!pl_dispatch_finish(dp, &sh, inter, NULL, NULL)) {
        fprintf(stderr, "Failed dispatching shader!\n");
        return 4;
    }

    sh = pl_dispatch_begin(dp);
    if (!sh)
        return 3;

    pl_shader_sample_ortho(sh, PL_SEP_HORIZ, &(struct pl_sample_src) {
        .tex = inter,
        .components = 3,
        .rect = { 0, 0, src->params.w, src->params.h },
        .new_w = dst->params.w,
        .new_h = dst->params.h,
    }, &filter);

    pl_shader_dither(sh, 8, &dither_state, NULL);
    if (!pl_dispatch_finish(dp, &sh, dst, NULL, NULL)) {
        fprintf(stderr, "Failed dispatching shader!\n");
        return 4;
    }
    */

    /*
    pl_shader_sample_direct(sh, &(struct pl_sample_src) {
        .tex            = src,
        .components     = 3,
    });

    pl_shader_decode_color(sh, &(struct pl_color_repr){
        .sys = PL_COLOR_SYSTEM_RGB,
        .levels = PL_COLOR_LEVELS_PC,
    }, &(struct pl_color_adjustment) {
        .brightness = 0.0,
        .contrast = 2.0,
        .saturation = 1.0,
        .hue = 0.0,
        .gamma = 1.0,
    });
    */

    struct pl_color_map_params params = pl_color_map_default_params;
    //params.gamut_warning = true;
    params.gamut_clipping = true;
    params.desaturation_strength = 1;
    params.desaturation_exponent = 0;

    struct pl_color_space src_space = {
        .primaries = PL_COLOR_PRIM_BT_2020,
        .transfer = PL_COLOR_TRC_GAMMA22,
        .light = PL_COLOR_LIGHT_DISPLAY,
    };

    struct pl_color_space dst_space = {
        .primaries = PL_COLOR_PRIM_BT_709,
        .transfer = PL_COLOR_TRC_SRGB,
        .light = PL_COLOR_LIGHT_DISPLAY,
    };

    //struct pl_color_space dst_space = pl_color_space_srgb;
    //dst_space.sig_scale = 3.0;

    pl_shader_color_map(sh, &params,
                        src_space, dst_space,
                        NULL, false);

    /*
    pl_shader_cone_distort(sh, pl_color_space_srgb, &(struct pl_cone_params) {
        .cones = PL_CONE_L,
        .strength = 0.0,
    });
    */

    /*
    static const enum pl_channel channels[3] = {0, 0, 0};
    pl_shader_av1_grain(sh, &state, channels, NULL, &(struct pl_grain_params) {
        .width = width,
        .height = height,
        .repr = {
            .levels = PL_COLOR_LEVELS_TV,
            .sys = PL_COLOR_SYSTEM_BT_709,
            .bits = { .color_depth = 10, .sample_depth = 10 },
        },
        .grain_seed = 4316,
        .overlap = false,

        .num_points_y = 6,
        .points_y = {{0, 4}, {27, 33}, {54, 55}, {67, 61}, {108, 71}, {-1, 72}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        .chroma_scaling_from_luma = false,
        .num_points_uv = {2, 2},
        .points_uv = {{{0, 0}, {255, 0}}, {{0, 0}, {255, 0}}},
        .scaling_shift = 11,
        .ar_coeff_lag = 3,
        .ar_coeffs_y = {0, 0, -1, 0, 0, 1, 1, 2, 1, -3, -8, -1, 2, 1, 4, 0, -5, -10, -4, 0, 2, 9, 1, 1},
        .ar_coeffs_uv = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128},
        },
        .ar_coeff_shift = 6,
        .grain_scale_shift = 0,
        .uv_mult = {0, 0},
        .uv_mult_luma = {64, 64},
        .uv_offset = {0, 0},
    });

    pl_shader_cone_distort(sh, pl_color_space_srgb, &(struct pl_cone_params) {
        .cones = PL_CONE_M,
        .strength = 0.5,
    });
    */

    /*
    pl_shader_deband(sh, &(struct pl_sample_src) {
        .tex            = src,
        .components     = 4,
    }, NULL);
    */

    pl_shader_dither(sh, 8, &dither_state, NULL);

    struct pl_dispatch_params dparams = {
        .shader = &sh,
        .target = dst,
    };

    if (!pl_dispatch_finish(dp, &dparams)) {
        fprintf(stderr, "Failed dispatching shader!\n");
        return 4;
    }

    // Download the result data
    transfer.tex = dst;
    if (!pl_tex_download(gpu, &transfer)) {
        fprintf(stderr, "Failed downloading result data!\n");
        return 4;
    }

    // Write the result to stdout
    printf("PF\n%d %d\n-1.0\n", dst->params.w, dst->params.h);
    for (int y = 0; y < dst->params.h; y++) {
        for (int x = 0; x < dst->params.w; x++) {
            int idx = y * stride_w + x;
            float *texel = (float *) &buf[idx * fmt->texel_size];
            //fprintf(stderr, "%f %f %f\n", texel[0], texel[1], texel[2]);
            fwrite(texel, sizeof(float), 3, stdout);
        }
    }

    // Clean up state and exit
    pl_shader_obj_destroy(&state);
    pl_shader_obj_destroy(&dither_state);
    pl_dispatch_destroy(&dp);
    pl_tex_destroy(gpu, &inter);
    pl_tex_destroy(gpu, &src);
    pl_tex_destroy(gpu, &dst);
    pl_vulkan_destroy(&vk);
    pl_context_destroy(&ctx);
}
