/**
 * Methods for manipulating shader objects
 * @author Scott LaVigne
 */
#ifndef SHADER_H
#define SHADER_H

#include <stddef.h>

#include <GL/gl.h>

typedef enum ShaderType {

  SHADER_VERTEX = GL_VERTEX_SHADER,
  SHADER_FRAGMENT = GL_FRAGMENT_SHADER

} ShaderType;

typedef struct Shader {

  unsigned id;
  ShaderType type;

} Shader;

typedef struct Pipeline {

  unsigned id;
  Shader* vert_shader;
  Shader* frag_shader;
  unsigned attribute[8];
  unsigned uniform[8];

} Pipeline;

/**
 * Allocates and compiles a shader
 * @param  type shader type. Either SHADER_VERTEX or SHADER_FRAGMENT
 * @param  path file path to shader file
 * @return      a new shader object
 */
Shader* shader_new(ShaderType type, const char* path);

/**
 * Free a shader
 * @param shader a shader
 */
void shader_free(Shader* shader);

/**
 * Links shaders into a shader program and returns it
 * @param  vert_shader a vertex shader
 * @param  frag_shader a fragment shader
 * @return             a new shader program
 */
Pipeline* pipeline_new(Shader* vert_shader, Shader* frag_shader);

// locate an attribute and add it to an attribute table with id
/**
 * Locate an attribute and add it to the attribute table under an id
 * @param pipeline a shader program
 * @param attr     the attribute name to search for
 * @param id       an index in the shader to use
 */
void pipeline_attribute(Pipeline* pipeline, const char* attr, unsigned id);

/**
 * Locate a uniform and add it the uniform table under and id
 * @param pipeline a shader program
 * @param unif     the uniform name to search for
 * @param id       an index in the shader to use
 */
void pipeline_uniform(Pipeline* pipeline, const char* unif, unsigned id);

/**
 * Free a shader program
 * @param pipeline a shader program
 */
void pipeline_free(Pipeline* pipeline);

#endif /* SHADER_H */