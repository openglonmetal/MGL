/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * mgl_safety.h
 * MGL - Safety and validation utilities
 *
 */

#ifndef MGL_SAFETY_H
#define MGL_SAFETY_H

#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

// CRITICAL SAFETY MACROS - Replace unsafe assert() calls
// These provide production-safe validation instead of crashes

// For pointer validation - returns error instead of crashing
#define MGL_VALIDATE_POINTER(ptr, error_msg, return_value) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "MGL SECURITY ERROR: %s at %s:%d\n", (error_msg), __FILE__, __LINE__); \
            return (return_value); \
        } \
    } while(0)

// For condition validation - returns error instead of crashing
#define MGL_VALIDATE_CONDITION(condition, error_msg, return_value) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "MGL SECURITY ERROR: %s at %s:%d\n", (error_msg), __FILE__, __LINE__); \
            return (return_value); \
        } \
    } while(0)

// For array bounds checking
#define MGL_VALIDATE_ARRAY_INDEX(index, max_size, error_msg, return_value) \
    do { \
        if ((index) >= (max_size)) { \
            fprintf(stderr, "MGL SECURITY ERROR: %s (index=%d, max=%d) at %s:%d\n", \
                    (error_msg), (int)(index), (int)(max_size), __FILE__, __LINE__); \
            return (return_value); \
        } \
    } while(0)

// For Metal object validation
#define MGL_VALIDATE_METAL_OBJECT(obj, obj_name, return_value) \
    do { \
        if (!(obj)) { \
            fprintf(stderr, "MGL SECURITY ERROR: Metal object %s is NULL at %s:%d\n", \
                    (obj_name), __FILE__, __LINE__); \
            return (return_value); \
        } \
    } while(0)

// For malloc/realloc validation with cleanup
#define MGL_VALIDATE_ALLOC(ptr, size, cleanup_action, return_value) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "MGL SECURITY ERROR: Failed to allocate %zu bytes at %s:%d\n", \
                    (size_t)(size), __FILE__, __LINE__); \
            { cleanup_action } \
            return (return_value); \
        } \
    } while(0)

// Comprehensive buffer pointer validation to prevent memory corruption crashes (for functions that return values)
#define MGL_VALIDATE_BUFFER_POINTER(ptr, function_name) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - NULL buffer pointer at %s:%d\n", \
                    (function_name), __FILE__, __LINE__); \
            return GL_INVALID_OPERATION; \
        } \
        \
        uintptr_t ptr_val = (uintptr_t)(ptr); \
        \
        /* Check for known corrupted pointer patterns */ \
        if (ptr_val == 0x10 || ptr_val == 0x30 || ptr_val == 0x1000 || ptr_val == 0x10000) { \
            fprintf(stderr, "MGL BUFFER CRITICAL ERROR: %s - Known corrupted buffer pointer 0x%lx at %s:%d\n", \
                    (function_name), ptr_val, __FILE__, __LINE__); \
            return GL_INVALID_OPERATION; \
        } \
        \
        /* Check for obviously invalid pointer values */ \
        if (ptr_val < 0x1000 || (ptr_val & 0xF) != 0) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Invalid buffer pointer %p at %s:%d\n", \
                    (function_name), (ptr), __FILE__, __LINE__); \
            return GL_INVALID_OPERATION; \
        } \
        \
        /* Check for suspiciously high pointer values */ \
        if (ptr_val > 0x100000000ULL) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Suspicious buffer pointer %p at %s:%d\n", \
                    (function_name), (ptr), __FILE__, __LINE__); \
            return GL_INVALID_OPERATION; \
        } \
    } while(0)

// Buffer pointer validation for void functions (returns early instead of returning a value)
#define MGL_VALIDATE_BUFFER_POINTER_VOID(ptr, function_name) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - NULL buffer pointer at %s:%d\n", \
                    (function_name), __FILE__, __LINE__); \
            return; \
        } \
        \
        uintptr_t ptr_val = (uintptr_t)(ptr); \
        \
        /* Check for known corrupted pointer patterns */ \
        if (ptr_val == 0x10 || ptr_val == 0x30 || ptr_val == 0x1000 || ptr_val == 0x10000) { \
            fprintf(stderr, "MGL BUFFER CRITICAL ERROR: %s - Known corrupted buffer pointer 0x%lx at %s:%d\n", \
                    (function_name), ptr_val, __FILE__, __LINE__); \
            return; \
        } \
        \
        /* Check for obviously invalid pointer values */ \
        if (ptr_val < 0x1000 || (ptr_val & 0xF) != 0) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Invalid buffer pointer %p at %s:%d\n", \
                    (function_name), (ptr), __FILE__, __LINE__); \
            return; \
        } \
        \
        /* Check for suspiciously high pointer values */ \
        if (ptr_val > 0x100000000ULL) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Suspicious buffer pointer %p at %s:%d\n", \
                    (function_name), (ptr), __FILE__, __LINE__); \
            return; \
        } \
    } while(0)

// Safe buffer size access with validation (for functions that return values)
#define MGL_GET_BUFFER_SIZE_SAFE(ptr, size_var, function_name) \
    do { \
        MGL_VALIDATE_BUFFER_POINTER((ptr), (function_name)); \
        \
        /* Validate that the buffer structure hasn't been corrupted */ \
        if ((ptr)->size < 0) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Corrupted buffer size %ld at %s:%d\n", \
                    (function_name), (long)((ptr)->size), __FILE__, __LINE__); \
            return GL_INVALID_OPERATION; \
        } \
        \
        /* Check for suspiciously large buffer sizes */ \
        if ((ptr)->size > 0x10000000) { \
            fprintf(stderr, "MGL BUFFER WARNING: %s - Suspiciously large buffer size %ld at %s:%d\n", \
                    (function_name), (long)((ptr)->size), __FILE__, __LINE__); \
        } \
        \
        (size_var) = (ptr)->size; \
    } while(0)

// Safe buffer size access with validation (for void functions)
#define MGL_GET_BUFFER_SIZE_SAFE_VOID(ptr, size_var, function_name) \
    do { \
        MGL_VALIDATE_BUFFER_POINTER_VOID((ptr), (function_name)); \
        \
        /* Validate that the buffer structure hasn't been corrupted */ \
        if ((ptr)->size < 0) { \
            fprintf(stderr, "MGL BUFFER ERROR: %s - Corrupted buffer size %ld at %s:%d\n", \
                    (function_name), (long)((ptr)->size), __FILE__, __LINE__); \
            return; \
        } \
        \
        /* Check for suspiciously large buffer sizes */ \
        if ((ptr)->size > 0x10000000) { \
            fprintf(stderr, "MGL BUFFER WARNING: %s - Suspiciously large buffer size %ld at %s:%d\n", \
                    (function_name), (long)((ptr)->size), __FILE__, __LINE__); \
        } \
        \
        (size_var) = (ptr)->size; \
    } while(0)

// Debug-only assertions that remain in debug builds
#ifdef DEBUG
    #define MGL_DEBUG_ASSERT(condition) assert(condition)
    #define MGL_DEBUG_ASSERT_PTR(ptr) assert(ptr)
#else
    #define MGL_DEBUG_ASSERT(condition) do {} while(0)
    #define MGL_DEBUG_ASSERT_PTR(ptr) do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif // MGL_SAFETY_H