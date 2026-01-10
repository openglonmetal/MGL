#ifndef MGL_TOOLCHAIN_H
#define MGL_TOOLCHAIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mgl_toolchain_stage {
    MGL_TOOLCHAIN_STAGE_VERTEX = 0,
    MGL_TOOLCHAIN_STAGE_FRAGMENT = 1,
    MGL_TOOLCHAIN_STAGE_COMPUTE = 2,
} mgl_toolchain_stage;

/*
 * Compile GLSL to Metal Shading Language.
 *
 * On success returns 0 and sets *out_msl to a newly allocated NUL-terminated
 * string (free with mgl_toolchain_free).
 *
 * On failure returns non-zero and sets *out_error (if provided) to a newly
 * allocated message string (free with mgl_toolchain_free).
 */
int mgl_toolchain_glsl_to_msl(
    mgl_toolchain_stage stage,
    const char *glsl_source,
    size_t glsl_len,
    char **out_msl,
    size_t *out_msl_len,
    char **out_error);

void mgl_toolchain_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* MGL_TOOLCHAIN_H */
