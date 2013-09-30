/**
 * Methods for manipulating physics bodies
 * @author Scott LaVigne
 */
#ifndef BODY_H
#define BODY_H

#include "maths.h"
#include "list.h"

typedef struct Edge {

  vec2* point1;
  vec2* point2;
  struct Body* parent;
  float length;

} Edge;

typedef struct Body {

  // Collision infos
  vec2 center_of_mass;
  float mass;
  vec4 bbox; // = {minX minY maxX maxY}

  vec2* points;
  vec2* last_points;
  vec3* colors;
  int num_points;

  Edge* edges;
  int num_edges;

  unsigned vbo;

  void (*step_callback)(struct Body*, double, void*);
  void* step_data;

  List* collision_callbacks;
  bool gravity; // Whether gravity is being applied
  int mask;     // collision group mask
  bool boxed;   // whether to constrain to the world
  bool wire;    // display as wireframe

} Body;

/**
 * Create a new physics body
 * @param  points     array of points to prototype the body
 * @param  colors     array of colors to prototype the body
 * @param  num_points number of verticies
 * @param  edges      array of edge descriptions to prototype the body
 * @param  num_edges  number of edges
 * @return            a new body
 */
Body* body_new(
  vec2* points,
  vec3* colors,
  int num_points,
  vec2i* edges,
  int num_edges);

/**
 * Do a single timestep of verlet integration on a body
 * @param body a body
 * @param dt   time since last frame in seconds
 */
void body_do_verlet(Body* body, double dt);

/**
 * Execute step callback on a body
 * @param body a body
 * @param dt   time since last frame in seconds
 */
void body_do_step(Body* body, double dt);

/**
 * Calculate edge constraints on a body
 * @param body a body
 */
void body_do_edges(Body* body);

/**
 * Calculate collision constraints on a body
 * @param body a body
 */
void body_do_collisions(Body* body);

/**
 * Calculate center of mass on a body
 * @param body a body
 */
void body_do_center(Body* body);

/**
 * Render a body
 * @param body a body
 */
void body_do_render(Body* body);

/**
 * Add a callback if a body touches another
 * @param body     a body
 * @param other    the body to test against
 * @param callback a method to execute on the body. The first
 *                 argument is the body. The second argument
 *                 is the opposing body. The last argument is
 *                 the extra data being passed.
 * @param data     extra data to pass to the method
 */
void body_add_collision_callback(
  Body* body,
  Body* other,
  void (*callback)(Body*, Body*, void*),
  void* data);

/**
 * Set the step callback for a body
 * @param body     a body
 * @param callback a method to execute on the body.
 *                 The first argument is the body. The
 *                 second argument is the time in seconds
 *                 since the last frame. The last argument is
 *                 the extra data to pass to the method.
 * @param data     extra data to pass to the method
 */
void body_set_logic(
  Body* body,
  void(*callback)(Body*, double, void*),
  void* data);

#endif /* BODY_H */
