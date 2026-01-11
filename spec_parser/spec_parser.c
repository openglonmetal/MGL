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
 * parser.c
 * headerparser
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include "ezxml.h"

#define string(_STR_)   (const char *)_STR_

enum {
    kEnum,
    KCommand
};

enum {
    kGLHeaders,         // gl.h
    kGLFuncs,           // gl.c
    kMGLHeaders,        // mgl.h
    kMGLFuncs,          // mgl.c
    kMGLDispatch,       // glm_dispatch.h
    kMGLDispatchInit    // glm_dispatch.c
};

typedef struct {
    int major, minor;
} GLVersion;

GLVersion *get_gl_versions(unsigned es_only)
{
    if (es_only)
    {
        static GLVersion gl_es_version[] = {{3,0}, {3,1}, {3,2}, {0,0}};
        
        return gl_es_version;
    }

    static GLVersion gl_versions[] = {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {2,0}, {2,1}, {3,0}, {3,1}, {3,2}, {3,3}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {4,5}, {4,6}, {0,0}};
    
    return gl_versions;

}

char *insert_string(char *src, char *insert, size_t insert_at)
{
    char *ret, *ptr;
    size_t len_src, len_insert;

    assert(src);
    assert(insert);

    // if 0 this is a reverse strcat
    if (insert_at == 0)
    {
        size_t len;
        char *ptr;

        len = strlen(src) + strlen(insert) + 128;
        ptr = (char *)malloc(len);
        assert(ptr);

        strcpy(ptr, insert);
        strcat(ptr, src);

        return ptr;
    }

    len_src = strlen(src);
    assert(len_src);

    len_insert = strlen(insert);
    assert(len_insert);

    ret = (char *)malloc(len_src + len_insert + 128);
    assert(ret);

    ptr = ret;
    for(int i=0; i<insert_at; i++)
    {
        *ptr++ = *src++;
    }

    for(int i=0; i<len_insert; i++)
    {
        *ptr++ = *insert++;
    }

    while(*src)
    {
        *ptr++ = *src++;
    }

    *ptr = 0;

    return ret;
}

char *make_mgl_str(const char *str)
{
    char *new_str;
    size_t len;

    assert(str);

    len = strlen(str) + 128;
    new_str = (char *)malloc(len);
    assert(new_str);

    sprintf(new_str, "m%s", str);

    return new_str;
}

char *make_dispatch_str(const char *str)
{
    char *new_str, *ptr;
    size_t len;

    assert(str);

    len = strlen(str) + 128;
    new_str = (char *)malloc(len);
    assert(new_str);

    ptr = new_str;

    // jump over "gl"
    str = &str[2];

    *ptr++ = tolower(*str++);
    while(*str)
    {
        // fix odd case for 1D 2D 3D at end of string.. keep those uppercase
        if (isupper(*str) && (str[1] != 0))
        {
            *ptr++ = '_';
            *ptr++ = tolower(*str++);
        }
        else
        {
            *ptr++ = *str++;
        }
    }

    *ptr = 0;

    return new_str;
}

void print_type(ezxml_t type, FILE *fp_out)
{
    ezxml_t name;

    name = ezxml_child(type, "name");
    if (name)
    {
        char *new_str;

        new_str = insert_string(type->txt, name->txt, name->off);
        assert(new_str);

        fprintf(fp_out, "%s\n", new_str);

        free(new_str);
    }
    else
    {
        fprintf(fp_out, "%s\n", type->txt);
    }
}

void print_enum(ezxml_t enum_node, FILE *fp_out)
{
    const char *value, *name;

    value = ezxml_attr(enum_node, string("value"));
    name = ezxml_attr(enum_node, string("name"));

    assert(value);
    assert(name);

    fprintf(fp_out, "#define %-60s %s\n", name, value);
}

int proto_has_return_type(ezxml_t proto)
{
    ezxml_t ptype;

    assert(proto);
    assert(proto->txt);

    ptype = ezxml_child(proto, string("ptype"));

    if (ptype)
    {
        return 1;
    }

    if (strstr("void", proto->txt))
    {
        return 1;
    }

    return 0;
}

char *proto_return_type(ezxml_t proto)
{
    ezxml_t ptype;

    assert(proto);
    assert(proto->txt);

    ptype = ezxml_child(proto, string("ptype"));

    if (ptype)
    {
        return ptype->txt;
    }

    if (strstr("void", proto->txt))
    {
        return proto->txt;
    }

    return 0;
}

int get_param_count(ezxml_t command)
{
    ezxml_t param;
    int param_count = 0;

    for (param = ezxml_child(command, "param"); param; param = param->next)
    {
        param_count++;
    }

    return param_count;
}

void print_command(ezxml_t command, int mode, FILE *fp_out)
{
    ezxml_t proto, name;
    ezxml_t param, ptype;

    proto = ezxml_child(command, string("proto"));
    assert(proto);

    name = ezxml_child(proto, string("name"));

    ptype = ezxml_child(proto, string("ptype"));

    int param_count = get_param_count(command);

    if (mode == kMGLDispatch)
    {
        char *dispatch_str;

        dispatch_str = make_dispatch_str(name->txt);

        if (ptype)
        {
            if (param_count)
            {
                fprintf(fp_out, "\t\t%s (*%s)(GLMContext ctx, ", ptype->txt, dispatch_str);
            }
            else
            {
                fprintf(fp_out, "\t\t%s (*%s)(GLMContext ctx", ptype->txt, dispatch_str);
            }
        }
        else
        {
            if (param_count)
            {
                fprintf(fp_out, "\t\t%s (*%s)(GLMContext ctx, ", proto->txt, dispatch_str);
            }
            else
            {
                fprintf(fp_out, "\t\t%s (*%s)(GLMContext ctx", proto->txt, dispatch_str);
            }
        }

        free(dispatch_str);
    }
    else if (mode == kGLHeaders || mode == kGLFuncs)
    {
        char *str;

        str = insert_string(proto->txt, name->txt, name->off);
        assert(str);

        if (ptype)
        {
            fprintf(fp_out, "%s %s(", ptype->txt, str);
        }
        else
        {
            fprintf(fp_out, "%s(", str);
        }

        free(str);
    }
    else if (mode == kMGLHeaders || mode == kMGLFuncs)
    {
        char *str, *mgl_func;

        mgl_func = make_mgl_str(name->txt);
        assert(mgl_func);

        str = insert_string(proto->txt, mgl_func, name->off);
        assert(str);

        if (param_count)
        {
            if (ptype)
            {
                fprintf(fp_out, "%s %s(GLMContext ctx, ", ptype->txt, str);
            }
            else
            {
                fprintf(fp_out, "%s(GLMContext ctx, ", str);
            }
        }
        else
        {
            if (ptype)
            {
                fprintf(fp_out, "%s %s(GLMContext ctx", ptype->txt, str);
            }
            else
            {
                fprintf(fp_out, "%s(GLMContext ctx", str);
            }
        }

        free(str);

        free(mgl_func);
    }
    else
    {
        assert(0);
    }

    if (param_count)
    {
        if (!strcmp("glGenBuffers", name->txt))
        {
            printf("glGenBuffers\n");
        }

        for (param = ezxml_child(command, "param"); param; param = param->next)
        {
            assert(param->off);

            ptype = ezxml_child(param, "ptype");
            name = ezxml_child(param, "name");

            assert(name);
            assert(name->txt);

            param_count--;

            if (ptype)
            {
                char *str = insert_string(param->txt, ptype->txt, ptype->off);

                fprintf(fp_out, "%s%s", str, name->txt);

                if (param_count)
                {
                    fprintf(fp_out, ", ");
                }

                free(str);
            }
            else
            {
                fprintf(fp_out, "%s%s", param->txt, name->txt);

                if (param_count)
                {
                    fprintf(fp_out, ", ");
                }
            }
        }
    }

    if (mode == kGLHeaders || mode == kMGLHeaders || mode == kMGLDispatch)
    {
        fprintf(fp_out, ");\n");
    }
    else if (mode == kGLFuncs)
    {
        char *dispatch_name;

        name = ezxml_child(proto, string("name"));

        fprintf(fp_out, ")\n{\n");
        fprintf(fp_out, "\tGLMContext ctx = GET_CONTEXT();\n\n");

        dispatch_name = make_dispatch_str(name->txt);

        if (proto_has_return_type(proto))
        {
            fprintf(fp_out, "\treturn ctx->dispatch.%s(ctx, ", dispatch_name);
        }
        else
        {
            fprintf(fp_out, "\tctx->dispatch.%s(ctx, ", dispatch_name);
        }

        free(dispatch_name);

        param_count = get_param_count(command);

        if (param_count)
        {
            for (param = ezxml_child(command, "param"); param; param = param->next)
            {
                ptype = ezxml_child(param, "ptype");
                name = ezxml_child(param, "name");

                assert(name);
                assert(name->txt);

                param_count--;

                if (ptype)
                {
                    fprintf(fp_out, "%s", name->txt);

                    if (param_count)
                    {
                        fprintf(fp_out, ", ");
                    }
                }
                else
                {
                    fprintf(fp_out, "%s", name->txt);
                }
            }
        }
        // end of function call
        fprintf(fp_out, ");\n");

        fprintf(fp_out, "}\n\n");
    }
    else if (mode == kMGLFuncs)
    {
        char *mgl_name;

        name = ezxml_child(proto, string("name"));

        fprintf(fp_out, ")\n{\n");

        mgl_name = make_mgl_str(name->txt);

        if (proto_has_return_type(proto))
        {
            char *ret_type;

            ret_type = proto_return_type(proto);

            if (!strcmp(ret_type, "GLuint"))
            {
                fprintf(fp_out, "\t%s ret = 0;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLubyte"))
            {
                fprintf(fp_out, "\t%s ret = 0;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLboolean"))
            {
                fprintf(fp_out, "\t%s ret = 0;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLenum"))
            {
                fprintf(fp_out, "\t%s ret = (GLenum)0;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLint"))
            {
                fprintf(fp_out, "\t%s ret = -1;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLhandleARB"))
            {
                fprintf(fp_out, "\t%s ret = NULL;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLintptr"))
            {
                fprintf(fp_out, "\t%s ret = 0;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLsync"))
            {
                fprintf(fp_out, "\t%s ret = NULL;\n\n", ret_type);
            }
            else if (!strcmp(ret_type, "GLvoid"))
            {
                fprintf(fp_out, "\t%s* ret = NULL;\n\n", ret_type);
            }
            else
            {
                assert(0);
            }

            fprintf(fp_out, "\t// Unimplemented function\n");
            fprintf(fp_out, "\tassert(0);\n");

            fprintf(fp_out, "\treturn ret;\n");
        }
        else
        {
            fprintf(fp_out, "\t// Unimplemented function\n");
            fprintf(fp_out, "\tassert(0);\n");
        }

        free(mgl_name);

        // end of function call
        fprintf(fp_out, "}\n\n");
    }
   else
    {
        assert(0);
    }
}

void print_init_dispatch_command(ezxml_t command, FILE *fp_out)
{
    ezxml_t proto, name;
    char *dispatch_name, *mgl_name;

    proto = ezxml_child(command, string("proto"));
    assert(proto);

    name = ezxml_child(proto, string("name"));

    dispatch_name = make_dispatch_str(name->txt);
    mgl_name = make_mgl_str(name->txt);

    fprintf(fp_out, "\tctx->dispatch.%s = %s;\n", dispatch_name, mgl_name);

    free(dispatch_name);
    free(mgl_name);
}

ezxml_t find_enum(ezxml_t registry, const char *str)
{
    ezxml_t nodes, node;

    for (nodes = ezxml_child(registry, "enums"); nodes; nodes = nodes->next)
    {
        for (node = ezxml_child(nodes, "enum"); node; node = node->next)
        {
            const char *name;

            name = ezxml_attr(node, "name");

            if (!strcmp(name, str))
            {
                return node;
            }
        }
    }

    return NULL;
}

ezxml_t find_command(ezxml_t registry, const char *str)
{
    ezxml_t nodes, node;

    for (nodes = ezxml_child(registry, "commands"); nodes; nodes = nodes->next)
    {
        for (node = ezxml_child(nodes, "command"); node; node = node->next)
        {
            ezxml_t proto, name;

            proto = ezxml_child(node, "proto");
            assert(proto);

            name = ezxml_child(proto, string("name"));
            assert(name);

            if (!strcmp(name->txt, str))
            {
                return node;
            }
        }
    }

    return NULL;
}

ezxml_t find_feature(ezxml_t registry, const char *feature, unsigned es_only)
{
    ezxml_t node;

    for (node = ezxml_child(registry, "feature"); node; node = node->next)
    {
        const char *api, *name;

        api = ezxml_attr(node, "api");
        name = ezxml_attr(node, "name");

        assert(api);
        assert(name);

        // search by api and version
        if (es_only == 0)
        {
            if (!strcmp(api, "gl") && !strcmp(name, feature))
            {
                return node;
            }
        }
        else
        {
            if (!strcmp(api, "gles2") && !strcmp(name, feature))
            {
                return node;
            }
        }
    }

    return NULL;
}

void print_features(ezxml_t registry, ezxml_t feature, int type, FILE *fout)
{
    ezxml_t require, node;

    for (require = ezxml_child(feature, "require"); require; require = require->next)
    {
        if (type == kEnum)
        {
            ezxml_t enum_node;

            for (node = ezxml_child(require, "enum"); node; node = node->next)
            {
                const char *name;

                name = ezxml_attr(node, "name");
                assert(name);

                enum_node = find_enum(registry, name);
                assert(enum_node);

                print_enum(enum_node, fout);
            }
        }
        else if (type == KCommand)
        {
            ezxml_t command_node;

            for (node = ezxml_child(require, "command"); node; node = node->next)
            {
                const char *name;

                name = ezxml_attr(node, "name");
                assert(name);

                command_node = find_command(registry, name);
                assert(command_node);

                print_command(command_node, kGLHeaders, fout);
            }
        }
        else
        {
            assert(0);
        }
    }
}

void print_required_features(ezxml_t registry, const char *version, int mode, FILE *fout, unsigned es_only)
{
    ezxml_t feature;

    feature = find_feature(registry, version, es_only);
    assert(feature);

    print_features(registry, feature, mode, fout);
}

void print_required(ezxml_t registry, FILE *fp_out, unsigned es_only)
{
    GLVersion *gl_versions = get_gl_versions(es_only);
    char version_str[64];

    for (int i=0; gl_versions[i].major !=0; i++)
    {
        if (es_only == 0)
        {
            sprintf(version_str, "GL_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }
        else
        {
            sprintf(version_str, "GL_ES_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }

        fprintf(fp_out, "\n\n#ifndef %s\n", version_str);
        fprintf(fp_out, "#define %s 1\n", version_str);
        fprintf(fp_out, "\n// %s enums\n\n", version_str);
        print_required_features(registry, string(version_str), kEnum, fp_out, es_only);
        fprintf(fp_out, "#endif // %s\n\n", version_str);
    }

    for (int i=0; gl_versions[i].major !=0; i++)
    {
        if (es_only == 0)
        {
            sprintf(version_str, "GL_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }
        else
        {
            sprintf(version_str, "GL_ES_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }

        fprintf(fp_out, "\n\n#ifndef %s\n", version_str);
        fprintf(fp_out, "#define %s 1\n", version_str);
        fprintf(fp_out, "\n// %s commands\n\n", version_str);
        print_required_features(registry, string(version_str), KCommand, fp_out, es_only);
        fprintf(fp_out, "#endif // %s\n\n", version_str);
    }
}

void print_required_feature_commands(ezxml_t registry, ezxml_t feature, int mode, FILE *fout)
{
    ezxml_t require, node;

    for (require = ezxml_child(feature, "require"); require; require = require->next)
    {
        ezxml_t command_node;

        for (node = ezxml_child(require, "command"); node; node = node->next)
        {
            const char *name;

            name = ezxml_attr(node, "name");
            assert(name);

            command_node = find_command(registry, name);
            assert(command_node);

            if (mode == kMGLDispatchInit)
            {
                print_init_dispatch_command(command_node, fout);
            }
            else
            {
                print_command(command_node, mode, fout);
            }
        }
    }
}

void print_required_commands(ezxml_t registry, int mode, FILE *fout, unsigned es_only)
{
    GLVersion *gl_versions = get_gl_versions(es_only);
    char version_str[64];

    for (int i=0; gl_versions[i].major !=0; i++)
    {
        ezxml_t feature;

        if (es_only == 0)
        {
            sprintf(version_str, "GL_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }
        else
        {
            sprintf(version_str, "GL_ES_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        }

        feature = find_feature(registry, version_str, es_only);
        assert(feature);

        print_required_feature_commands(registry, feature, mode, fout);
    }
}

void print_about(FILE *fp_out, const char *filename)
{
    fprintf(fp_out, "//\n// %s\n", filename);
    fprintf(fp_out, "//\n// Autogenerated from gl.xml\n");
    fprintf(fp_out, "//\n// Mike Larson\n");
    fprintf(fp_out, "//\n// January 2026\n");
    fprintf(fp_out, "//\n\n");
}

void print_mgl_core_header(ezxml_t registry)
{
    FILE *fp_out;
    ezxml_t nodes, node;
    const char *filename = "/tmp/gl_core.h";

    //fp_out = fopen(filename, "w");
    fp_out = stdout;
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    // print GL types
    for (nodes = ezxml_child(registry, "types"); nodes; nodes = nodes->next)
    {
        for (node = ezxml_child(nodes, "type"); node; node = node->next)
        {
            print_type(node, fp_out);
        }
    }

    // print all the required enums and prototypes
    print_required(registry, fp_out, 0);

    //fclose(fp_out);
}


void print_mgl_core_source(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/gl_core.c";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    // print the required commands
    print_required_commands(registry, kGLFuncs, fp_out, 0);

    fclose(fp_out);
}

void print_mgl_es_source(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/gl_es.c";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    // print the required commands
    print_required_commands(registry, kGLFuncs, fp_out, 1);

    fclose(fp_out);
}

void print_mgl_dispatch(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/glm_dispatch.h";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    for (int version=0; version<2; version++)
    {
        if (version == 0)
        {
            fprintf(fp_out, "struct GLMDispatchTable {\n");
        }
        else
        {
            fprintf(fp_out, "\nstruct GLM_ES_DispatchTable {\n");
        }
        
        // print the required commands
        print_required_commands(registry, kMGLDispatch, fp_out, version);
        
        fprintf(fp_out, "};\n");
    }
    
    fclose(fp_out);
}

void print_mgl_init_dispatch(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/glm_dispatch.c";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    fprintf(fp_out, "#include \"mgl.h\"\n\n");

    for (int version=0; version<2; version++)
    {
        
        if (version == 0)
        {
            fprintf(fp_out, "void init_dispatch(GLMContext ctx)\n");
        }
        else
        {
            fprintf(fp_out, "void init_es_dispatch(GLMContext ctx)\n");
        }
        
        fprintf(fp_out, "{\n");
        
        // print the required commands
        print_required_commands(registry, kMGLDispatchInit, fp_out, version);
        
        fprintf(fp_out, "};\n");
    }
    
    fclose(fp_out);
}

void print_mgl_header(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/mgl.h";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    // print the required commands
    print_required_commands(registry, kMGLHeaders, fp_out, 0);

    fclose(fp_out);
}

void print_mgl_functions(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/mgl.c";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print mgl_core.h header
    print_about(fp_out, filename);

    // print the required commands
    print_required_commands(registry, kMGLFuncs, fp_out, 0);

    fclose(fp_out);
}


int main(int argc, char **argv)
{
    ezxml_t registry;

    registry = ezxml_parse_file("gl.xml");
    assert(registry);

    // mgl gl exported enums and prototypes
    print_mgl_core_header(registry);

    // mgl gl exported functions
    print_mgl_core_source(registry);

    // mgl es exported functions
    print_mgl_es_source(registry);

    // mgl dispatch table
    print_mgl_dispatch(registry);

    // mgl init dispatch table
    print_mgl_init_dispatch(registry);

    // mgl prototypes
    print_mgl_header(registry);

    // mgl functions
    print_mgl_functions(registry);

    ezxml_free(registry);
}
