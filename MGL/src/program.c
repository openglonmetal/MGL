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
 * program.c
 * MGL
 *
 */

#include <stdio.h>
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>
#include <glslang_c_interface.h>
#include <glslang_c_shader_types.h>
#include "spirv-tools/libspirv.h"
#include "spirv_cross_c.h"
#include "spirv.h"

#include "glm_context.h"
#include "shaders.h"
#include "buffers.h"

// Program Pipeline management
ProgramPipeline *newProgramPipeline(GLMContext ctx, GLuint pipeline)
{
    ProgramPipeline *ptr;

    ptr = (ProgramPipeline *)malloc(sizeof(ProgramPipeline));
    assert(ptr);

    bzero(ptr, sizeof(ProgramPipeline));
    ptr->name = pipeline;

    return ptr;
}

ProgramPipeline *findProgramPipeline(GLMContext ctx, GLuint pipeline)
{
    return (ProgramPipeline *)searchHashTable(&STATE(program_pipeline_table), pipeline);
}

ProgramPipeline *getProgramPipeline(GLMContext ctx, GLuint pipeline)
{
    ProgramPipeline *ptr = findProgramPipeline(ctx, pipeline);

    if (!ptr)
    {
        ptr = newProgramPipeline(ctx, pipeline);
        insertHashElement(&STATE(program_pipeline_table), pipeline, ptr);
    }

    return ptr;
}

// Transform Feedback management
TransformFeedback *newTransformFeedback(GLMContext ctx, GLuint name)
{
    TransformFeedback *ptr;

    ptr = (TransformFeedback *)malloc(sizeof(TransformFeedback));
    assert(ptr);

    bzero(ptr, sizeof(TransformFeedback));
    ptr->name = name;
    ptr->target = GL_TRANSFORM_FEEDBACK;
    ptr->active = GL_FALSE;
    ptr->paused = GL_FALSE;
    ptr->primitive_mode = GL_NONE;

    return ptr;
}

TransformFeedback *findTransformFeedback(GLMContext ctx, GLuint name)
{
    return (TransformFeedback *)searchHashTable(&STATE(transform_feedback_table), name);
}

TransformFeedback *getTransformFeedback(GLMContext ctx, GLuint name)
{
    TransformFeedback *ptr = findTransformFeedback(ctx, name);

    if (!ptr)
    {
        ptr = newTransformFeedback(ctx, name);
        insertHashElement(&STATE(transform_feedback_table), name, ptr);
    }

    return ptr;
}

Program *newProgram(GLMContext ctx, GLuint program)
{
    Program *ptr;

    ptr = (Program *)malloc(sizeof(Program));
    assert(ptr);

    bzero(ptr, sizeof(Program));

    ptr->name = program;

    return ptr;
}

Program *getProgram(GLMContext ctx, GLuint program)
{
    Program *ptr;

    ptr = (Program *)searchHashTable(&STATE(program_table), program);

    if (!ptr)
    {
        ptr = newProgram(ctx, program);

        insertHashElement(&STATE(program_table), program, ptr);
    }

    return ptr;
}

int isProgram(GLMContext ctx, GLuint program)
{
    Program *ptr;

    ptr = (Program *)searchHashTable(&STATE(program_table), program);

    if (ptr)
        return 1;

    return 0;
}

Program *findProgram(GLMContext ctx, GLuint program)
{
    Program *ptr;

    ptr = (Program *)searchHashTable(&STATE(program_table), program);

    return ptr;
}

GLuint mglCreateProgram(GLMContext ctx)
{
    GLuint program;

    program = getNewName(&STATE(program_table));

    getProgram(ctx, program);

    return program;
}

void mglFreeProgram(GLMContext ctx, Program *ptr)
{
    if (ptr->linked_glsl_program)
    {
        glslang_program_delete(ptr->linked_glsl_program);
    }

    if (ptr->mtl_data)
    {
        ctx->mtl_funcs.mtlDeleteMTLObj(ctx, ptr->mtl_data);
    }

    for(int i=0; i<_MAX_SHADER_TYPES; i++)
    {
        // CRITICAL FIX: Add NULL checks before all free/release operations to prevent double-frees
        if (ptr->spirv[i].ir) {
            free(ptr->spirv[i].ir);
            ptr->spirv[i].ir = NULL;
        }
        if (ptr->spirv[i].msl_str) {
            free(ptr->spirv[i].msl_str);
            ptr->spirv[i].msl_str = NULL;
        }
        if (ptr->spirv[i].entry_point) {
            free(ptr->spirv[i].entry_point);
            ptr->spirv[i].entry_point = NULL;
        }
        if (ptr->spirv[i].mtl_function) {
            CFRelease(ptr->spirv[i].mtl_function);
            ptr->spirv[i].mtl_function = NULL;
        }
        if (ptr->spirv[i].mtl_library) {
            CFRelease(ptr->spirv[i].mtl_library);
            ptr->spirv[i].mtl_library = NULL;
        }
        
        for(int j=0; j<_MAX_SPIRV_RES; j++)
        {
            // CRITICAL FIX: Add NULL checks and clear pointers to prevent double-frees
            if (ptr->spirv_resources_list[i][j].list) {
                free(ptr->spirv_resources_list[i][j].list);
                ptr->spirv_resources_list[i][j].list = NULL;
            }
        }
        
        if (ptr->shader_slots[i])
        {
            Shader *sptr = ptr->shader_slots[i];
            sptr->refcount--;
            if (sptr->refcount == 0 && sptr->delete_status)
            {
                mglFreeShader(ctx, sptr);
            }
        }
    }

    free(ptr);
}

void mglDeleteProgram(GLMContext ctx, GLuint program)
{
    Program *ptr;

    ptr = findProgram(ctx, program);

    if (!ptr)
    {
        // // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION; // Silent ignore if not found? OpenGL says GL_INVALID_VALUE usually, but delete is often silent for 0.
        // But if program != 0 and not found, it's GL_INVALID_VALUE.
        return;
    }

    deleteHashElement(&STATE(program_table), program);
    
    ptr->delete_status = GL_TRUE;
    
    if (ptr->refcount == 0)
    {
        mglFreeProgram(ctx, ptr);
    }
}

GLboolean mglIsProgram(GLMContext ctx, GLuint program)
{
    if (isProgram(ctx, program))
        return GL_TRUE;

    return GL_FALSE;
}

void mglAttachShader(GLMContext ctx, GLuint program, GLuint shader)
{
    Program *pptr;
    Shader *sptr;
    GLuint index;

    sptr = findShader(ctx, shader);

    if (!sptr)
    {
        // CRITICAL FIX: Handle missing shader gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Shader %u not found in attach shader\n", shader);
        STATE(error) = GL_INVALID_VALUE;
        return;
    }

    pptr = findProgram(ctx, program);

    if (!pptr)
    {
        // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;

        return;
    }

    index = sptr->glm_type;

    pptr->shader_slots[index] = sptr;
    sptr->refcount++;
    pptr->dirty_bits |= DIRTY_PROGRAM;
}

void mglDetachShader(GLMContext ctx, GLuint program, GLuint shader)
{
    Program *pptr;
    Shader *sptr;
    GLuint index;

    pptr = findProgram(ctx, program);
    if (!pptr)
    {
        // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
        return;
    }

    sptr = findShader(ctx, shader);

    if (!sptr)
    {
        // If not found in hash table, check if it is attached to the program
        for (int i=0; i<_MAX_SHADER_TYPES; i++) {
            if (pptr->shader_slots[i] && pptr->shader_slots[i]->name == shader) {
                sptr = pptr->shader_slots[i];
                break;
            }
        }
    }

    if (!sptr)
    {
        // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
        return;
    }

    index = sptr->glm_type;

    if (pptr->shader_slots[index] != sptr)
    {
        return;
    }

    pptr->shader_slots[index] = NULL;
    sptr->refcount--;
    
    if (sptr->refcount == 0 && sptr->delete_status)
    {
        mglFreeShader(ctx, sptr);
    }
    
    pptr->dirty_bits |= DIRTY_PROGRAM;
}

void error_callback(void *userdata, const char *error)
{
    assert(error);
    DEBUG_PRINT("parseSPIRVShader error:%s\n", error);
}


static_assert(_VERTEX_SHADER == GLSLANG_STAGE_VERTEX, "_VERTEX_SHADER == GLSLANG_STAGE_VERTEX failed");
static_assert(_TESS_CONTROL_SHADER == GLSLANG_STAGE_TESSCONTROL, "_TESS_CONTROL_SHADER == GLSLANG_STAGE_TESSCONTROL failed");
static_assert(_TESS_EVALUATION_SHADER == GLSLANG_STAGE_TESSEVALUATION, "_TESS_EVALUATION_SHADER == GLSLANG_STAGE_TESSEVALUATION failed");
static_assert(_GEOMETRY_SHADER == GLSLANG_STAGE_GEOMETRY, "_GEOMETRY_SHADER == GLSLANG_STAGE_GEOMETRY failed");
static_assert(_FRAGMENT_SHADER == GLSLANG_STAGE_FRAGMENT, "_FRAGMENT_SHADER == GLSLANG_STAGE_FRAGMENT failed");
static_assert(_COMPUTE_SHADER == GLSLANG_STAGE_COMPUTE, "_COMPUTE_SHADER == GLSLANG_STAGE_COMPUTE failed");

void addShadersToProgram(GLMContext ctx, Program *pptr, glslang_program_t *glsl_program)
{
    // add shaders
    for(int i=0;i<_MAX_SHADER_TYPES; i++)
    {
        Shader *ptr;

        ptr = pptr->shader_slots[i];

        if(ptr)
        {
            // should have glsl shader here
            assert(ptr->compiled_glsl_shader);

            glslang_program_add_shader(glsl_program, ptr->compiled_glsl_shader);
        }
    }
}

char *parseSPIRVShaderToMetal(GLMContext ctx, Program *ptr, int stage)
{
    const SpvId *spirv;
    size_t word_count;
    char *str_ret;
    int parse_res;

    spvc_context context = NULL;
    spvc_parsed_ir ir = NULL;
    spvc_compiler compiler_msl = NULL;
    spvc_compiler_options options = NULL;
    spvc_resources resources = NULL;
    const spvc_reflected_resource *list = NULL;
    const char *result = NULL;
    size_t count;
    size_t i;

    spirv = ptr->spirv[stage].ir;
    assert(spirv);
    word_count = ptr->spirv[stage].size;
    assert(spirv);

    // Create context.
    spvc_context_create(&context);
    assert(context);

    // Set debug callback.
    spvc_context_set_error_callback(context, error_callback, ctx);

    // Parse the SPIR-V.
    parse_res = spvc_context_parse_spirv(context, spirv, word_count, &ir);
    assert(parse_res == SPVC_SUCCESS);

    // Hand it off to a compiler instance and give it ownership of the IR.
    spvc_context_create_compiler(context, SPVC_BACKEND_MSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_msl);
    assert(compiler_msl);
    // ERROR_CHECK_RETURN(spvc_compiler_msl_add_discrete_descriptor_set(compiler_msl, 3) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    if (spvc_compiler_msl_add_discrete_descriptor_set(compiler_msl, 3) != SPVC_SUCCESS) {
        fprintf(stderr, "MGL Error: spvc_compiler_msl_add_discrete_descriptor_set failed\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    // Modify options.
    // ERROR_CHECK_RETURN(spvc_compiler_create_compiler_options(compiler_msl, &options) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    if (spvc_compiler_create_compiler_options(compiler_msl, &options) != SPVC_SUCCESS) {
        fprintf(stderr, "MGL Error: spvc_compiler_create_compiler_options failed\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    // ERROR_CHECK_RETURN(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS, SPVC_FALSE) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    if (spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS, SPVC_FALSE) != SPVC_SUCCESS) {
        fprintf(stderr, "MGL Error: spvc_compiler_options_set_bool(SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS) failed\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    // ERROR_CHECK_RETURN(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_VERSION, SPVC_MAKE_MSL_VERSION(3,1,0)) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    if (spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_VERSION, SPVC_MAKE_MSL_VERSION(3,1,0)) != SPVC_SUCCESS) {
        fprintf(stderr, "MGL Error: spvc_compiler_options_set_uint(SPVC_COMPILER_OPTION_MSL_VERSION) failed\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    //ERROR_CHECK_RETURN(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 4.5) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    // ERROR_CHECK_RETURN(spvc_compiler_install_compiler_options(compiler_msl, options) == SPVC_SUCCESS, GL_INVALID_OPERATION);
    if (spvc_compiler_install_compiler_options(compiler_msl, options) != SPVC_SUCCESS) {
        fprintf(stderr, "MGL Error: spvc_compiler_install_compiler_options failed\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    
    // create an entry point for metal based on the shader type and name
    GLuint name;
    char entry_point[128];
    name = ptr->shader_slots[stage]->name;

    SpvExecutionModel model = SpvExecutionModelVertex; // CRITICAL FIX: Initialize with safe default
    switch(stage)
    {
        case _VERTEX_SHADER: model = SpvExecutionModelVertex; break;
        case _TESS_CONTROL_SHADER: model = SpvExecutionModelTessellationControl; break;
        case _TESS_EVALUATION_SHADER: model = SpvExecutionModelTessellationEvaluation; break;
        case _GEOMETRY_SHADER: model = SpvExecutionModelGeometry; break;
        case _FRAGMENT_SHADER: model = SpvExecutionModelFragment; break;
        case _COMPUTE_SHADER: model = SpvExecutionModelGLCompute; break;
        default: // CRITICAL FIX: Handle error gracefully instead of crashing
            fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
            STATE(error) = GL_INVALID_OPERATION;
            return NULL;
    }

    switch(stage)
    {
        case _VERTEX_SHADER: snprintf(entry_point, sizeof(entry_point), "vertex_%d_main",name); break;
        case _TESS_CONTROL_SHADER: snprintf(entry_point, sizeof(entry_point), "tess_control_%d_main",name); break;
        case _TESS_EVALUATION_SHADER: snprintf(entry_point, sizeof(entry_point), "tess_evaluation_%d_main",name); break;
        case _GEOMETRY_SHADER: snprintf(entry_point, sizeof(entry_point), "geometry_%d",name); break;
        case _FRAGMENT_SHADER: snprintf(entry_point, sizeof(entry_point), "fragment_%d",name); break;
        case _COMPUTE_SHADER: snprintf(entry_point, sizeof(entry_point), "compute_%d",name); break;
        default: // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
    }

    const char *cleansed_entry_point;
    cleansed_entry_point = spvc_compiler_get_cleansed_entry_point_name(compiler_msl, "main", model);

    spvc_result err;
    err = spvc_compiler_rename_entry_point(compiler_msl, cleansed_entry_point, entry_point, model);
    assert(err == SPVC_SUCCESS);

    // set the entry point for metal
    ptr->shader_slots[stage]->entry_point = strdup(entry_point);
    ptr->spirv[stage].entry_point = strdup(entry_point);

    // compute shader
    if (stage == _COMPUTE_SHADER)
    {
        spvc_result res;
        const spvc_entry_point *entry_points;
        size_t num_entry_points;

        res = spvc_compiler_get_entry_points(compiler_msl, &entry_points, &num_entry_points);
        assert(res);
        
        for(int i=0; i<num_entry_points; i++)
        {
            DEBUG_PRINT("Entry point: %s Execution Model: %d\n", entry_points[i].name, entry_points[i].execution_model);
        }

        ptr->local_workgroup_size.x = spvc_compiler_get_execution_mode_argument_by_index(compiler_msl, SpvExecutionModeLocalSize, 0);
        ptr->local_workgroup_size.y = spvc_compiler_get_execution_mode_argument_by_index(compiler_msl, SpvExecutionModeLocalSize, 1);
        ptr->local_workgroup_size.z = spvc_compiler_get_execution_mode_argument_by_index(compiler_msl, SpvExecutionModeLocalSize, 2);
    }
    
    // Do some basic reflection.
    spvc_compiler_create_shader_resources(compiler_msl, &resources);
    for (int res_type=SPVC_RESOURCE_TYPE_UNIFORM_BUFFER; res_type < SPVC_RESOURCE_TYPE_ACCELERATION_STRUCTURE; res_type++)
    {
#if DEBUG
        const char *res_name[] = {"NONE", "UNIFORM_BUFFER", "UNIFORM_CONSTANT", "STORAGE_BUFFER", "STAGE_INPUT", "STAGE_OUTPUT",
            "SUBPASS_INPUT", "STORAGE_INPUT", "SAMPLED_IMAGE", "ATOMIC_COUNTER", "PUSH_CONSTANT", "SEPARATE_IMAGE",
            "SEPARATE_SAMPLERS", "ACCELERATION_STRUCTURE", "RAY_QUERY"};
#endif
        
        spvc_resources_get_resource_list_for_type(resources, res_type, &list, &count);

        ptr->spirv_resources_list[stage][res_type].count = (GLuint)count;

        // CRITICAL SECURITY FIX: Prevent integer overflow in resource allocation
        // Check if count * sizeof(SpirvResource) would overflow size_t
        if (count > SIZE_MAX / sizeof(SpirvResource)) {
            fprintf(stderr, "MGL SECURITY ERROR: Resource count %zu would cause allocation overflow\n", count);
            ERROR_RETURN(GL_OUT_OF_MEMORY);
        }

        size_t alloc_size = count * sizeof(SpirvResource);
        ptr->spirv_resources_list[stage][res_type].list = (SpirvResource *)malloc(alloc_size);
        if (!ptr->spirv_resources_list[stage][res_type].list) {
            fprintf(stderr, "MGL SECURITY ERROR: Failed to allocate %zu bytes for resource list\n", alloc_size);
            ERROR_RETURN(GL_OUT_OF_MEMORY);
        }

        for (i = 0; i < count; i++)
        {
            DEBUG_PRINT("res_type: %s ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s ", res_name[res_type], list[i].id, list[i].base_type_id, list[i].type_id,
                   list[i].name);
            
            switch(res_type)
            {
                case SPVC_RESOURCE_TYPE_UNIFORM_BUFFER:
                case SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT:
                case SPVC_RESOURCE_TYPE_STORAGE_BUFFER:
                case SPVC_RESOURCE_TYPE_ATOMIC_COUNTER:
                    DEBUG_PRINT("Set: %u, Binding: %u Uniform: %d offset: %d\n",
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationDescriptorSet),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationBinding),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationUniform),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationOffset));
                    break;

                case SPVC_RESOURCE_TYPE_STAGE_INPUT:
                case SPVC_RESOURCE_TYPE_STAGE_OUTPUT:
                case SPVC_RESOURCE_TYPE_SUBPASS_INPUT:
                    DEBUG_PRINT("Set: %u, Location: %d Index: %d, offset: %d\n",
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationDescriptorSet),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationLocation),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationIndex),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationOffset));
                    break;
                    
                case SPVC_RESOURCE_TYPE_SAMPLED_IMAGE:
                case SPVC_RESOURCE_TYPE_SEPARATE_IMAGE:
                    DEBUG_PRINT("Set: %u, Location: %d\n",
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationDescriptorSet),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationLocation));
                    break;

                default:
                    DEBUG_PRINT("Set: %u, Binding: %u Location: %d Index: %d, Uniform: %d offset: %d\n",
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationDescriptorSet),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationBinding),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationLocation),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationIndex),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationUniform),
                           spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationOffset));
                    break;
            }
            
            ptr->spirv_resources_list[stage][res_type].list[i]._id = list[i].id;
            ptr->spirv_resources_list[stage][res_type].list[i].base_type_id = list[i].base_type_id;
            ptr->spirv_resources_list[stage][res_type].list[i].type_id = list[i].type_id;
            ptr->spirv_resources_list[stage][res_type].list[i].name = strdup(list[i].name);
            ptr->spirv_resources_list[stage][res_type].list[i].set = spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationDescriptorSet);
            ptr->spirv_resources_list[stage][res_type].list[i].binding = spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationBinding);
            ptr->spirv_resources_list[stage][res_type].list[i].location = spvc_compiler_get_decoration(compiler_msl, list[i].id, SpvDecorationLocation);
        }
    }

    spvc_compiler_compile(compiler_msl, &result);
    DEBUG_PRINT("\n%s\n", result);

    str_ret = strdup(result);

    // Frees all memory we allocated so far.
    spvc_context_destroy(context);

    return str_ret;
}

bool linkAndCompileProgramToMetal(GLMContext ctx, Program *pptr, int stage)
{
    glslang_program_t *glsl_program;
    int err;

    /* Safety check: ensure we have a shader for this stage */
    if (!pptr->shader_slots[stage]) {
        return false;
    }

    // Clean up old resources
    if (pptr->spirv[stage].ir) {
        free(pptr->spirv[stage].ir);
        pptr->spirv[stage].ir = NULL;
    }
    if (pptr->spirv[stage].msl_str) {
        free(pptr->spirv[stage].msl_str);
        pptr->spirv[stage].msl_str = NULL;
    }
    if (pptr->spirv[stage].entry_point) {
        free(pptr->spirv[stage].entry_point);
        pptr->spirv[stage].entry_point = NULL;
    }
    if (pptr->spirv[stage].mtl_function) {
        CFRelease(pptr->spirv[stage].mtl_function);
        pptr->spirv[stage].mtl_function = NULL;
    }
    if (pptr->spirv[stage].mtl_library) {
        CFRelease(pptr->spirv[stage].mtl_library);
        pptr->spirv[stage].mtl_library = NULL;
    }

    fprintf(stderr, "MGL DEBUG: Creating glslang program for stage %d\n", stage);
    glsl_program = glslang_program_create();
    assert(glsl_program);
    fprintf(stderr, "MGL DEBUG: Created glslang program %p\n", (void*)glsl_program);

    // shaders to glsl program
    fprintf(stderr, "MGL DEBUG: Adding shaders to program\n");
    addShadersToProgram(ctx, pptr, glsl_program);
    fprintf(stderr, "MGL DEBUG: Shaders added\n");

    // link
    fprintf(stderr, "MGL DEBUG: About to link program\n");
    err = glslang_program_link(glsl_program, GLSLANG_MSG_DEFAULT_BIT);
    fprintf(stderr, "MGL DEBUG: Program link returned %d\n", err);
    if (!err)
    {
        // this is useful.. but information after this failure isn't that interesting
        fprintf(stderr, "MGL Error: glslang_program_link failed err: %d\n", err);
        fprintf(stderr, "MGL Error: glslang_program_SPIRV_get_messages:\n%s\n", glslang_program_SPIRV_get_messages(glsl_program));
        fprintf(stderr, "MGL Error: glslang_program_get_info_log:\n%s\n", glslang_program_get_info_log(glsl_program));
        fprintf(stderr, "MGL Error: glslang_program_get_info_debug_log:\n%s\n", glslang_program_get_info_debug_log(glsl_program));

        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    // generate SPIVR
    fprintf(stderr, "MGL DEBUG: Generating SPIRV for stage %d\n", stage);
    glslang_program_SPIRV_generate(glsl_program, stage);
    fprintf(stderr, "MGL DEBUG: SPIRV generated\n");

    if (glslang_program_SPIRV_get_messages(glsl_program))
    {
        DEBUG_PRINT("%s\n", glslang_program_SPIRV_get_messages(glsl_program));

        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    // save SPIRV code
    fprintf(stderr, "MGL DEBUG: Getting SPIRV size\n");
    pptr->spirv[stage].size = glslang_program_SPIRV_get_size(glsl_program);
    fprintf(stderr, "MGL DEBUG: SPIRV size: %zu\n", pptr->spirv[stage].size);

    // CRITICAL SECURITY FIX: Prevent integer overflow in SPIRV allocation
    // Check if size * sizeof(unsigned) would overflow size_t
    if (pptr->spirv[stage].size > SIZE_MAX / sizeof(unsigned)) {
        fprintf(stderr, "MGL SECURITY ERROR: SPIRV size %zu would cause allocation overflow\n", pptr->spirv[stage].size);
        ERROR_RETURN(GL_OUT_OF_MEMORY);
    }

    size_t alloc_size = pptr->spirv[stage].size * sizeof(unsigned);
    pptr->spirv[stage].ir = (unsigned int *)malloc(alloc_size);
    if (!pptr->spirv[stage].ir) {
        fprintf(stderr, "MGL SECURITY ERROR: Failed to allocate %zu bytes for SPIRV\n", alloc_size);
        ERROR_RETURN(GL_OUT_OF_MEMORY);
    }
    fprintf(stderr, "MGL DEBUG: Getting SPIRV IR\n");
    glslang_program_SPIRV_get(glsl_program, pptr->spirv[stage].ir);
    fprintf(stderr, "MGL DEBUG: SPIRV IR obtained\n");

    // compile SPIRV to Metal
    fprintf(stderr, "MGL DEBUG: About to parse SPIRV to Metal\n");
    pptr->spirv[stage].msl_str = parseSPIRVShaderToMetal(ctx, pptr, stage);
    fprintf(stderr, "MGL DEBUG: SPIRV parsed to Metal\n");
    // ERROR_CHECK_RETURN(pptr->spirv[stage].msl_str, GL_INVALID_OPERATION);
    if (pptr->spirv[stage].msl_str == NULL) {
        fprintf(stderr, "MGL Error: parseSPIRVShaderToMetal failed for stage %d\n", stage);
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    pptr->linked_glsl_program = glsl_program;
    pptr->dirty_bits |= DIRTY_PROGRAM;

    glslang_program_delete(glsl_program);

    return true;
}

void mglLinkProgram(GLMContext ctx, GLuint program)
{
    Program *pptr;

    pptr = findProgram(ctx, program);

    if (!pptr)
    {
        // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;

        return;
    }

    for (int stage=0; stage<_MAX_SHADER_TYPES; stage++)
    {
        pptr->spirv[stage].msl_str = 0;
        
        if (pptr->shader_slots[stage])
        {
            linkAndCompileProgramToMetal(ctx, pptr, stage);
        }
    }

    /* Only call mtlBindProgram if Metal functions are initialized */
    if (ctx->mtl_funcs.mtlBindProgram) {
        ctx->mtl_funcs.mtlBindProgram(ctx, pptr);
    } else {
        fprintf(stderr, "WARNING: Metal functions not initialized, skipping mtlBindProgram\n");
    }

    //ERROR_CHECK_RETURN(pptr->mtl_data, GL_INVALID_OPERATION);
}

void mglUseProgram(GLMContext ctx, GLuint program)
{
    Program *pptr;

    if (program)
    {
        pptr = findProgram(ctx, program);

        if (!pptr)
        {
            fprintf(stderr, "MGL Error: mglUseProgram program %u not found\n", program);
            // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;

            return;
        }

        ERROR_CHECK_RETURN(pptr->linked_glsl_program, GL_INVALID_OPERATION);
    }
    else
    {
        pptr = NULL;
    }

    if (ctx->state.program != pptr)
    {
        if (ctx->state.program)
        {
            ctx->state.program->refcount--;
            if (ctx->state.program->refcount == 0 && ctx->state.program->delete_status)
            {
                mglFreeProgram(ctx, ctx->state.program);
            }
        }

        ctx->state.program = pptr;

        if (ctx->state.program)
        {
            ctx->state.program->refcount++;
            // Only mark dirty when binding a valid program
            // Don't mark dirty when unbinding (pptr=NULL) to preserve existing pipeline
            ctx->state.dirty_bits |= DIRTY_PROGRAM;
        }
        // When unbinding (pptr=NULL), don't mark dirty - keep existing pipeline state
    }
}

void mglBindAttribLocation(GLMContext ctx, GLuint program, GLuint index, const GLchar *name)
{
    // Unimplemented function
    // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
}

void mglGetActiveAttrib(GLMContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    // Unimplemented function
    // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
}

void mglGetActiveUniform(GLMContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    // Unimplemented function
    // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
}

void mglGetAttachedShaders(GLMContext ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
    // Unimplemented function
    // CRITICAL FIX: Handle error gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Critical error in program.c at line %d\n", __LINE__);
        STATE(error) = GL_INVALID_OPERATION;
}

GLint  mglGetAttribLocation(GLMContext ctx, GLuint program, const GLchar *name)
{
	if (isProgram(ctx, program) == GL_FALSE)
	{
		ERROR_RETURN(GL_INVALID_OPERATION); // also may be GL_INVALID_VALUE ????

		return -1;
	}

	Program *ptr;

	ptr = getProgram(ctx, program);
	assert(program);

	if (ptr->linked_glsl_program == NULL)
	{
		ERROR_RETURN(GL_INVALID_OPERATION);

		return -1;
	}

	for (int stage=_VERTEX_SHADER; stage<_MAX_SHADER_TYPES; stage++)
	{
		int count;

		count = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_STAGE_INPUT].count;

		for (int i=0; i<count; i++)
		{
			const char *str = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_STAGE_INPUT].list[i].name;

			if (!strcmp(str, name))
			{
				GLuint location;

				location = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_STAGE_INPUT].list[i].location;

				return location;
			}
		}
	}
	
	return -1;
}

void mglGetProgramiv(GLMContext ctx, GLuint program, GLenum pname, GLint *params)
{
    Program *pptr = findProgram(ctx, program);
    ERROR_CHECK_RETURN(pptr, GL_INVALID_VALUE);
    
    switch (pname) {
        case GL_LINK_STATUS:
            /* If we got here after mglLinkProgram, linking succeeded */
            *params = GL_TRUE;
            break;
        case GL_DELETE_STATUS:
            *params = GL_FALSE;  /* Programs are not deleted by default */
            break;
        case GL_VALIDATE_STATUS:
            *params = GL_TRUE;  /* Assume valid */
            break;
        case GL_INFO_LOG_LENGTH:
            *params = 0;  /* No info log for now */
            break;
        case GL_ATTACHED_SHADERS:
            {
                int count = 0;
                for (int i = 0; i < _MAX_SHADER_TYPES; i++) {
                    if (pptr->shader_slots[i]) count++;
                }
                *params = count;
            }
            break;
        case GL_ACTIVE_ATTRIBUTES:
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        case GL_ACTIVE_UNIFORMS:
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            /* These require SPIRV resource reflection - return 0 for now */
            *params = 0;
            break;
        case GL_COMPUTE_WORK_GROUP_SIZE:
            if (pptr->shader_slots[_COMPUTE_SHADER]) {
                /* Return local workgroup size for compute shaders */
                params[0] = pptr->local_workgroup_size.x;
                params[1] = pptr->local_workgroup_size.y;
                params[2] = pptr->local_workgroup_size.z;
            } else {
                params[0] = params[1] = params[2] = 0;
            }
            break;
        default:
            fprintf(stderr, "mglGetProgramiv: unhandled pname 0x%x\n", pname);
            *params = 0;
            break;
    }
}

void mglGetProgramInfoLog(GLMContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    Program *pptr = findProgram(ctx, program);
    ERROR_CHECK_RETURN(pptr, GL_INVALID_VALUE);
    
    /* For now, always return an empty info log */
    if (bufSize > 0 && infoLog) {
        infoLog[0] = '\0';
        if (length) {
            *length = 0;
        }
    }
}



#pragma mark program pipelines
void mglGenProgramPipelines(GLMContext ctx, GLsizei n, GLuint *pipelines)
{
    for (GLsizei i = 0; i < n; i++)
    {
        pipelines[i] = getNewName(&STATE(program_pipeline_table));
        getProgramPipeline(ctx, pipelines[i]);
    }
}

GLboolean mglIsProgramPipeline(GLMContext ctx, GLuint pipeline)
{
    ProgramPipeline *ptr = findProgramPipeline(ctx, pipeline);
    return ptr ? GL_TRUE : GL_FALSE;
}

void mglDeleteProgramPipelines(GLMContext ctx, GLsizei n, const GLuint *pipelines)
{
    for (GLsizei i = 0; i < n; i++)
    {
        if (pipelines[i] == 0)
            continue;
            
        ProgramPipeline *ptr = findProgramPipeline(ctx, pipelines[i]);
        if (!ptr)
            continue;
            
        // If deleting currently bound pipeline, unbind it
        if (STATE(program_pipeline) && STATE(program_pipeline)->name == pipelines[i])
        {
            STATE(program_pipeline) = NULL;
        }
        
        // Remove from hash table and free
        deleteHashElement(&STATE(program_pipeline_table), pipelines[i]);
        free(ptr);
    }
}

void mglBindProgramPipeline(GLMContext ctx, GLuint pipeline)
{
    if (pipeline == 0)
    {
        STATE(program_pipeline) = NULL;
        STATE(dirty_bits) |= DIRTY_PROGRAM;
        return;
    }
    
    ProgramPipeline *ptr = getProgramPipeline(ctx, pipeline);
    STATE(program_pipeline) = ptr;
    STATE(dirty_bits) |= DIRTY_PROGRAM;
}

void mglUseProgramStages(GLMContext ctx, GLuint pipeline, GLbitfield stages, GLuint program)
{
    ProgramPipeline *pipe_ptr = findProgramPipeline(ctx, pipeline);
    if (!pipe_ptr)
    {
        STATE(error) = GL_INVALID_OPERATION;
        return;
    }
    
    Program *prog_ptr = NULL;
    if (program != 0)
    {
        prog_ptr = findProgram(ctx, program);
        if (!prog_ptr)
        {
            STATE(error) = GL_INVALID_VALUE;
            return;
        }
    }
    
    // Attach program to specified stages
    if (stages & GL_VERTEX_SHADER_BIT)
        pipe_ptr->stage_programs[_VERTEX_SHADER] = prog_ptr;
    if (stages & GL_FRAGMENT_SHADER_BIT)
        pipe_ptr->stage_programs[_FRAGMENT_SHADER] = prog_ptr;
    if (stages & GL_GEOMETRY_SHADER_BIT)
        pipe_ptr->stage_programs[_GEOMETRY_SHADER] = prog_ptr;
    if (stages & GL_TESS_CONTROL_SHADER_BIT)
        pipe_ptr->stage_programs[_TESS_CONTROL_SHADER] = prog_ptr;
    if (stages & GL_TESS_EVALUATION_SHADER_BIT)
        pipe_ptr->stage_programs[_TESS_EVALUATION_SHADER] = prog_ptr;
    if (stages & GL_COMPUTE_SHADER_BIT)
        pipe_ptr->stage_programs[_COMPUTE_SHADER] = prog_ptr;
        
    pipe_ptr->validated = GL_FALSE;
    STATE(dirty_bits) |= DIRTY_PROGRAM;
}
