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
 * enum_parser.c
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

GLVersion *get_gl_versions()
{
    static GLVersion gl_versions[] = {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {2,0}, {2,1}, {3,0}, {3,1}, {3,2}, {3,3}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {4,5}, {4,6}, {0,0}};

    return gl_versions;
}

int hex_to_int(const char *str)
{
    int ret = 0;

    while(tolower(*str) != 'x') {
        str++;
    }

    assert(tolower(*str) == 'x');
    str++;

    while(*str) {
        char c = *str;

        ret *= 16;

        if (isnumber(c)) {
            ret += c - '0';
        } else {
            ret += (tolower(c) - 'a');
        }

        str++;
    }

    return ret;
}

/*
void print_enum(ezxml_t enum_node, FILE *fp_out)
{
    const char *value, *name, *group;

    value = ezxml_attr(enum_node, string("value"));
    name = ezxml_attr(enum_node, string("name"));
    group = ezxml_attr(enum_node, string("group"));

    assert(value);
    assert(name);

    if (group) {
        fprintf(fp_out, "\tif ((target == %s) && !strstr(group, \"%s\")) return \"%s\";\n", value, group, name);
    } else {
        fprintf(fp_out, "\tif (target == %s) return \"%s\";\n", value, name);
    }
}
*/

char *make_struct_field_from_enum(const char *name)
{
    char *str, *ptr;

    str = strdup(&name[3]);

    ptr = str;
    while(*ptr)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }

    return str;
}

void print_enum(ezxml_t enum_node, FILE *fp_out)
{
    const char *value, *name, *group;

    value = ezxml_attr(enum_node, string("value"));
    name = ezxml_attr(enum_node, string("name"));
    group = ezxml_attr(enum_node, string("group"));

    assert(value);
    assert(name);

    if (!group)
        return;

    //if (strcmp("InternalFormat", group) &&
    //    strcmp("InternalFormat,SizedInternalFormat", group))
    //    return;

    if (strcmp("TextureEnvMode,EnableCap,GetPName", group))
        return;

   // char *str_name = make_struct_field_from_enum(name);

    fprintf(fp_out, "\t\tGLboolean %s;\n", group);
//    fprintf(fp_out, "\t\t GLuint %s;\n", str_name);

//    fprintf(fp_out, "\tcase %s: RET_TYPE(type, ctx->state.vars.%s); break; // %s\n", value, str_name, name);
// fprintf(fp_out, "\tGLuint\t\t%-s; // %s\n", str_name, name);

    //fprintf(fp_out, "\t\tcase %s:\n", name);

    //free(str_name);
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

ezxml_t find_feature(ezxml_t registry, const char *feature)
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
        if (!strcmp(api, "gl") && !strcmp(name, feature))
        {
            return node;
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
        else
        {
            assert(0);
        }
    }
}

void print_required_features(ezxml_t registry, const char *version, int mode, FILE *fout)
{
    ezxml_t feature;

    feature = find_feature(registry, version);
    assert(feature);

    print_features(registry, feature, mode, fout);
}

void print_required(ezxml_t registry, FILE *fp_out)
{
    GLVersion *gl_versions = get_gl_versions();
    char version_str[64];

    for (int i=0; gl_versions[i].major !=0; i++)
    {
        snprintf(version_str, sizeof(version_str), "GL_VERSION_%d_%d", gl_versions[i].major, gl_versions[i].minor);
        print_required_features(registry, string(version_str), kEnum, fp_out);
    }
}

void print_about(FILE *fp_out, const char *filename)
{
    fprintf(fp_out, "//\n// %s\n", filename);
    fprintf(fp_out, "//\n// Autogenerated from gl.xml\n");
    fprintf(fp_out, "//\n// Mike Larson\n");
    fprintf(fp_out, "//\n// October 2021\n");
    fprintf(fp_out, "//\n\n");
}

void print_gl_enums(ezxml_t registry)
{
    FILE *fp_out;
    const char *filename = "/tmp/enums.c";

    fp_out = fopen(filename, "w");
    assert(fp_out);

    // print header
    print_about(fp_out, filename);

    fprintf(fp_out, "char *get_enum_str(GLMContext ctx, GLenum target, const char *group) {\n");

    // print the required commands
    print_required(registry, fp_out);

    fprintf(fp_out, "}\n");

    fclose(fp_out);
}

void print_gl_get(ezxml_t registry)
{

}

int main(int argc, char **argv)
{
    ezxml_t registry;

    registry = ezxml_parse_file("gl.xml");
    assert(registry);

    // mgl gl exported enums and prototypes
    print_gl_enums(registry);
}
