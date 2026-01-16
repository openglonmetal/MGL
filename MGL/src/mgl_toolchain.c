#include "mgl_toolchain.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__has_include)
#if __has_include(<glslang_c_interface.h>) && __has_include(<glslang_c_shader_types.h>)
#define MGL_TOOLCHAIN_HAS_GLSLANG 1
#include <glslang_c_interface.h>
#include <glslang_c_shader_types.h>
#else
#define MGL_TOOLCHAIN_HAS_GLSLANG 0
#endif

#if __has_include("spirv_cross_c.h")
#define MGL_TOOLCHAIN_HAS_SPVC 1
#include "spirv_cross_c.h"
#elif __has_include(<spirv_cross_c.h>)
#define MGL_TOOLCHAIN_HAS_SPVC 1
#include <spirv_cross_c.h>
#else
#define MGL_TOOLCHAIN_HAS_SPVC 0
#endif
#else
#define MGL_TOOLCHAIN_HAS_GLSLANG 0
#define MGL_TOOLCHAIN_HAS_SPVC 0
#endif

#if MGL_TOOLCHAIN_HAS_GLSLANG
const glslang_resource_t* glslang_default_resource(void);
#endif

static int mgl_set_error(char **out_error, const char *msg) {
    if (!out_error) {
        return 1;
    }
    if (!msg) {
        *out_error = NULL;
        return 1;
    }
    *out_error = strdup(msg);
    return 1;
}

void mgl_toolchain_free(void *ptr) {
    free(ptr);
}

#if MGL_TOOLCHAIN_HAS_GLSLANG && MGL_TOOLCHAIN_HAS_SPVC

static glslang_stage_t mgl_stage_to_glslang(mgl_toolchain_stage stage) {
    switch (stage) {
        case MGL_TOOLCHAIN_STAGE_VERTEX:
            return GLSLANG_STAGE_VERTEX;
        case MGL_TOOLCHAIN_STAGE_FRAGMENT:
            return GLSLANG_STAGE_FRAGMENT;
        case MGL_TOOLCHAIN_STAGE_COMPUTE:
            return GLSLANG_STAGE_COMPUTE;
        default:
            return GLSLANG_STAGE_VERTEX;
    }
}

static void mgl_init_input(glslang_input_t *input,
                           mgl_toolchain_stage stage,
                           const char *src) {
    fprintf(stderr, "[MGL DEBUG] mgl_init_input called, src first 50 chars: %.50s\n", src);
    memset(input, 0, sizeof(*input));
    input->language = GLSLANG_SOURCE_GLSL;
    input->stage = mgl_stage_to_glslang(stage);
    input->client = GLSLANG_CLIENT_OPENGL;
    input->client_version = GLSLANG_TARGET_OPENGL_450;
    input->target_language = GLSLANG_TARGET_SPV;
    // according to khronos GLSLANG_TARGET_OPENGL_450 client maps to spirv 1_6
    input->target_language_version = GLSLANG_TARGET_SPV_1_6;

    /* Detect and upgrade GLSL version in source code
     * GLSL 1.40 (OpenGL 3.1) shaders from virglrenderer need upgrading to 3.30
     * for glslang's SPIR-V compatibility with desktop OpenGL
     */
    int glsl_version = 330; /* Default to GLSL 3.30 */
    int original_version = 330;
    const char *version_str = strstr(src, "#version");
    if (version_str) {
        int scanned_version;
        if (sscanf(version_str, "#version %d", &scanned_version) == 1) {
            original_version = scanned_version;
            glsl_version = scanned_version;
            if (glsl_version < 330) {
                glsl_version = 330;
            }
        }
    }

    /* Log version upgrade for debugging */
    if (original_version < 330) {
        fprintf(stderr, "[MGL] Upgrading GLSL shader from version %d to %d\n",
                original_version, glsl_version);
    }

    /* For GLSL < 330, we need to replace the #version directive in source
     * Allocate a mutable copy to modify
     */
    static char *modified_src = NULL;
    static size_t modified_src_size = 0;

    size_t src_len = strlen(src);
    if (src_len + 100 > modified_src_size) {
        modified_src_size = src_len + 100;
        free(modified_src);
        modified_src = (char *)malloc(modified_src_size);
    }

    if (modified_src) {
        strcpy(modified_src, src);

        /* Find and replace #version line */
        char *version_line = strstr(modified_src, "#version");
        if (version_line) {
            char *newline = strchr(version_line, '\n');
            if (newline) {
                /* Replace with new version, keeping the rest of the line intact
                 * This preserves any profile specification or comments
                 */
                char version_buf[64];
                snprintf(version_buf, sizeof(version_buf), "#version %d core", glsl_version);
                ptrdiff_t old_len = newline - version_line;
                ptrdiff_t new_len = (ptrdiff_t)strlen(version_buf);

                if (new_len <= old_len) {
                    /* Simple replacement - pad with spaces */
                    memset(version_line, ' ', old_len);
                    memcpy(version_line, version_buf, new_len);
                    fprintf(stderr, "[MGL] Replaced version line, first 100 chars: %.100s\n", modified_src);
                } else {
                    fprintf(stderr, "[MGL] WARNING: New version line too long, skipping replacement\n");
                }
            }
        }
        input->code = modified_src;
    } else {
        /* Fallback: use original source */
        fprintf(stderr, "[MGL] WARNING: Failed to allocate modified_src, using original\n");
        input->code = src;
    }

    input->default_version = glsl_version;
    input->default_profile = GLSLANG_CORE_PROFILE;
    input->force_default_version_and_profile = 1;
    input->messages = GLSLANG_MSG_DEFAULT_BIT | GLSLANG_MSG_RELAXED_ERRORS_BIT;
    input->resource = glslang_default_resource();
}

int mgl_toolchain_glsl_to_msl(
    mgl_toolchain_stage stage,
    const char *glsl_source,
    size_t glsl_len,
    char **out_msl,
    size_t *out_msl_len,
    char **out_error) {

    if (out_msl) {
        *out_msl = NULL;
    }
    if (out_msl_len) {
        *out_msl_len = 0;
    }
    if (out_error) {
        *out_error = NULL;
    }

    if (!glsl_source || !out_msl) {
        return mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: invalid arguments");
    }

    /* Ensure NUL-terminated source for glslang. */
    char *src = NULL;
    if (glsl_len == 0) {
        glsl_len = strlen(glsl_source);
    }
    src = (char *)malloc(glsl_len + 1);
    if (!src) {
        return mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: OOM copying source");
    }
    memcpy(src, glsl_source, glsl_len);
    src[glsl_len] = '\0';

    static int glslang_ready = 0;
    if (!glslang_ready) {
        glslang_initialize_process();
        glslang_ready = 1;
    }

    glslang_input_t input;
    mgl_init_input(&input, stage, src);

    glslang_shader_t *shader = glslang_shader_create(&input);
    if (!shader) {
        free(src);
        return mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: glslang_shader_create failed");
    }

    if (!glslang_shader_preprocess(shader, &input)) {
        const char *info_log = glslang_shader_get_info_log(shader);
        const char *debug_log = glslang_shader_get_info_debug_log(shader);
        size_t info_len = info_log ? strlen(info_log) : 0;
        size_t dbg_len = debug_log ? strlen(debug_log) : 0;
        // CRITICAL SECURITY FIX: Prevent integer overflow in error message allocation
        if (info_len > SIZE_MAX - dbg_len - 64) {
            mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: Error message allocation would overflow");
            return 1;
        }

        size_t msg_size = info_len + dbg_len + 64;
        char *msg = (char *)malloc(msg_size);
        if (msg) {
            msg[0] = 0;
            strcat(msg, "glslang preprocess failed: ");
            if (info_log) strcat(msg, info_log);
            if (debug_log && dbg_len) {
                strcat(msg, "\n");
                strcat(msg, debug_log);
            }
            mgl_set_error(out_error, msg);
            free(msg);
        } else {
            mgl_set_error(out_error, "glslang preprocess failed");
        }
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    if (!glslang_shader_parse(shader, &input)) {
        const char *info_log = glslang_shader_get_info_log(shader);
        const char *debug_log = glslang_shader_get_info_debug_log(shader);
        size_t info_len = info_log ? strlen(info_log) : 0;
        size_t dbg_len = debug_log ? strlen(debug_log) : 0;
        // CRITICAL SECURITY FIX: Prevent integer overflow in error message allocation
        if (info_len > SIZE_MAX - dbg_len - 64) {
            mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: Error message allocation would overflow");
            return 1;
        }

        size_t msg_size = info_len + dbg_len + 64;
        char *msg = (char *)malloc(msg_size);
        if (msg) {
            msg[0] = 0;
            strcat(msg, "glslang parse failed: ");
            if (info_log) strcat(msg, info_log);
            if (debug_log && dbg_len) {
                strcat(msg, "\n");
                strcat(msg, debug_log);
            }
            mgl_set_error(out_error, msg);
            free(msg);
        } else {
            mgl_set_error(out_error, "glslang parse failed");
        }
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    glslang_program_t *program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_DEFAULT_BIT)) {
        const char *info_log = glslang_program_get_info_log(program);
        const char *debug_log = glslang_program_get_info_debug_log(program);
        size_t info_len = info_log ? strlen(info_log) : 0;
        size_t dbg_len = debug_log ? strlen(debug_log) : 0;
        // CRITICAL SECURITY FIX: Prevent integer overflow in error message allocation
        if (info_len > SIZE_MAX - dbg_len - 64) {
            mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: Error message allocation would overflow");
            return 1;
        }

        size_t msg_size = info_len + dbg_len + 64;
        char *msg = (char *)malloc(msg_size);
        if (msg) {
            msg[0] = 0;
            strcat(msg, "glslang link failed: ");
            if (info_log) strcat(msg, info_log);
            if (debug_log && dbg_len) {
                strcat(msg, "\n");
                strcat(msg, debug_log);
            }
            mgl_set_error(out_error, msg);
            free(msg);
        } else {
            mgl_set_error(out_error, "glslang link failed");
        }
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    glslang_program_SPIRV_generate(program, input.stage);
    size_t word_count = glslang_program_SPIRV_get_size(program);
    if (word_count == 0) {
        mgl_set_error(out_error, "glslang SPIR-V generation produced empty module");
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    // CRITICAL SECURITY FIX: Prevent integer overflow in SPIR-V buffer allocation
    if (word_count > SIZE_MAX / sizeof(unsigned int)) {
        mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: SPIR-V word count would cause allocation overflow");
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    size_t alloc_size = word_count * sizeof(unsigned int);
    unsigned int *words = (unsigned int *)malloc(alloc_size);
    if (!words) {
        mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: OOM allocating SPIR-V buffer");
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    glslang_program_SPIRV_get(program, words);

    spvc_context context = NULL;
    spvc_parsed_ir ir = NULL;
    spvc_compiler compiler = NULL;
    spvc_compiler_options options = NULL;
    const char *msl = NULL;

    if (spvc_context_create(&context) != SPVC_SUCCESS) {
        mgl_set_error(out_error, "SPIRV-Cross: spvc_context_create failed");
        free(words);
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    if (spvc_context_parse_spirv(context, words, word_count, &ir) != SPVC_SUCCESS) {
        mgl_set_error(out_error, "SPIRV-Cross: parse SPIR-V failed");
        spvc_context_destroy(context);
        free(words);
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    if (spvc_context_create_compiler(context, SPVC_BACKEND_MSL, ir, SPVC_CAPTURE_MODE_COPY, &compiler) != SPVC_SUCCESS) {
        mgl_set_error(out_error, "SPIRV-Cross: create MSL compiler failed");
        spvc_context_destroy(context);
        free(words);
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    if (spvc_compiler_create_compiler_options(compiler, &options) == SPVC_SUCCESS) {
        /* Try to target Metal 2.1 by default (macOS 10.14+). */
        spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_VERSION, 20100);
        spvc_compiler_install_compiler_options(compiler, options);
    }

    if (spvc_compiler_compile(compiler, &msl) != SPVC_SUCCESS || !msl) {
        const char *err = spvc_context_get_last_error_string(context);
        if (err && *err) {
            mgl_set_error(out_error, err);
        } else {
            mgl_set_error(out_error, "SPIRV-Cross: compile to MSL failed");
        }
        spvc_context_destroy(context);
        free(words);
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    *out_msl = strdup(msl);
    if (!*out_msl) {
        mgl_set_error(out_error, "mgl_toolchain_glsl_to_msl: OOM copying MSL output");
        spvc_context_destroy(context);
        free(words);
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        free(src);
        return 1;
    }

    if (out_msl_len) {
        *out_msl_len = strlen(*out_msl);
    }

    spvc_context_destroy(context);
    free(words);
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    free(src);
    return 0;
}

#else

int mgl_toolchain_glsl_to_msl(
    mgl_toolchain_stage stage,
    const char *glsl_source,
    size_t glsl_len,
    char **out_msl,
    size_t *out_msl_len,
    char **out_error) {

    (void)stage;
    (void)glsl_source;
    (void)glsl_len;
    if (out_msl) {
        *out_msl = NULL;
    }
    if (out_msl_len) {
        *out_msl_len = 0;
    }
    return mgl_set_error(out_error,
                         "MGL toolchain unavailable (missing glslang and/or spirv-cross headers). "
                         "Build deps under MGL/external or install via Homebrew.");
}

#endif
