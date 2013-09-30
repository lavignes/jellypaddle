/**
 * Methods for manipulating physics bodies: implementation
 * @author Scott LaVigne
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include "body.h"
#include "shader.h"
#include "list.h"

Pipeline* body_program;
unsigned body_vao;
List* bodies;

typedef struct CollisionCallback {
  Body* body;
  Body* other;
  void (*callback)(Body*,Body*,void*);
  void* data;
} CollisionCallback;

typedef struct CollisionHandler {
  float depth;
  vec2 normal;
  Edge* edge;
  vec2* vertex;
  struct Body* parent;
} CollisionHandler;

static CollisionHandler handler;

Body* body_new(
  vec2* points,
  vec3* colors,
  int num_points,
  vec2i* edges,
  int num_edges)
{
  int i;
  Body* body = malloc(sizeof(Body));
  body->colors = colors;
  body->points = malloc(sizeof(vec2) * num_points);
  body->last_points = malloc(sizeof(vec2) * num_points);
  body->num_points = num_points;
  body->edges = malloc(sizeof(Edge) * num_edges);
  body->num_edges = num_edges;
  memcpy(body->points, points, sizeof(vec2) * num_points);
  memcpy(body->last_points, points, sizeof(vec2) * num_points);
  
  for (i = 0; i < num_edges; i++) {
    Edge* edge = &body->edges[i];
    edge->parent = body;
    edge->point1 = &body->points[edges[i][0]];
    edge->point2 = &body->points[edges[i][1]];
    float dx = (*edge->point1)[0] - (*edge->point2)[0];
    float dy = (*edge->point1)[1] - (*edge->point2)[1];
    edge->length = sqrt(dx*dx + dy*dy);
  }

  body->step_callback = NULL;

  glGenBuffers(1, &body->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, body->vbo);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(vec2) + sizeof(vec3)) * num_points,
    NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * num_points, body->points);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec2) * num_points,
    sizeof(vec3) * num_points, body->colors);

  // bounding box calculates mass
  body_do_center(body);
  body->mass = fabs(body->bbox[0] - body->bbox[2]) * fabs(body->bbox[1] - body->bbox[3]);
  
  body->collision_callbacks = list_new();
  body->gravity = true;
  body->mask = 0x01;
  body->boxed = true;
  body->wire = false;
  return body;
}

void body_do_verlet(Body* body, double dt) {
  int i;
  for (i = 0; i < body->num_points; i++) {
    float nx = body->points[i][0]+body->points[i][0]-body->last_points[i][0];
    float ny;
    if (body->gravity == true)
      ny = body->points[i][1]+body->points[i][1]-body->last_points[i][1]-0.25;
    else
      ny = body->points[i][1]+body->points[i][1]-body->last_points[i][1];
    body->last_points[i][0] = body->points[i][0];
    body->last_points[i][1] = body->points[i][1];

    if (body->boxed) {
      body->points[i][0] = min(max(nx, 0.0), 800.0);
      body->points[i][1] = min(max(ny, 0.0), 600.0);
    } else {
      body->points[i][0] = nx;
      body->points[i][1] = ny;
    }
  }
}

void body_do_step(Body* body, double dt) {
  if (body->step_callback)
    body->step_callback(body, dt, body->step_data);
}

// pulls verts towards eachother to act as constraint
void body_do_edges(Body* body) {
  int i;

  for (i = 0; i < body->num_edges; i++) {
    Edge* edge = &body->edges[i];
    
    float dx = (*edge->point1)[0] - (*edge->point2)[0];
    float dy = (*edge->point1)[1] - (*edge->point2)[1];
    float d = sqrt(dx*dx + dy*dy);

    float diff = 0.0;
    if (d != 0.0) {
      diff = (edge->length - d) / d;
    }

    float tx = dx * 0.5 * diff;
    float ty = dy * 0.5 * diff;

    (*edge->point1)[0] += tx;
    (*edge->point1)[1] += ty;
    
    (*edge->point2)[0] -= tx;
    (*edge->point2)[1] -= ty;
  }
}

// collision response of the last detected collision
static void handle() {
  vec2* point1 = handler.edge->point1;
  vec2* point2 = handler.edge->point2;
  vec2 collision;
  float z, lambda, m, inv_m, r1, r2;

  collision[0] = handler.normal[0] * handler.depth;
  collision[1] = handler.normal[1] * handler.depth;

  z = (fabs((*point1)[0] - (*point2)[0]) > fabs((*point1)[1] - (*point2)[1]))?
    ((*handler.vertex)[0] - collision[0] - (*point1)[0]) / ((*point2)[0] - (*point1)[0])
    : ((*handler.vertex)[1] - collision[1] - (*point1)[1]) / ((*point2)[1] - (*point1)[1]);

  lambda = 1.0/(z*z + (1 - z)*(1 - z));
  m = z * handler.edge->parent->mass + (1.0 - z) * handler.edge->parent->mass;
  inv_m = 1.0/(m + handler.parent->mass);
  r1 = handler.parent->mass * inv_m;
  r2 = m * inv_m;

  (*point1)[0] -= collision[0] * ((1 - z) * r1 * lambda);
  (*point1)[1] -= collision[1] * ((1 - z) * r1 * lambda);
  (*point2)[0] -= collision[0] * (z * r1 * lambda);
  (*point2)[1] -= collision[1] * (z * r1 * lambda);
  (*handler.vertex)[0] += collision[0] * r2;
  (*handler.vertex)[1] += collision[1] * r2;
}

// return interval distance between 2 ranges
static float interv_dist(vec2* range1, vec2* range2) {
  return ((*range1)[0] < (*range2)[0])?
    (*range2)[0] - (*range1)[1] : (*range1)[0] - (*range2)[1];
}

// exactly what is sounds like. Used for AABB
static void project_to_axis(Body* body, vec2* axis, vec2* range) {
  float dot = (*axis)[0] * body->points[0][0] + (*axis)[1] * body->points[0][1];
  int i;
  (*range)[0] = dot;
  (*range)[1] = dot;

  for (i = 0; i < body->num_points; i++) {
    dot = (*axis)[0] * body->points[i][0] + (*axis)[1] * body->points[i][1];
    (*range)[0] = min((*range)[0], dot);
    (*range)[1] = max((*range)[1], dot);
  }
}

// AABB collision
static bool bodies_colliding(Body* body1, Body* body2) {
  float min_dist, small_dist;
  int i;
  float dist, len, xx, yy, dot;
  Body* temp;
  vec2 axis, range1, range2;
  // all edges in each body
  for (i = 0; i < (body1->num_edges + body2->num_edges); i++) {
    min_dist = 10000.0;
    Edge* edge = (i < body1->num_edges)?
      &body1->edges[i] : &body2->edges[i-body1->num_edges];

    // calc perpendicular axis
    axis[0] = (*edge->point1)[1] - (*edge->point2)[1];
    axis[1] = (*edge->point1)[0] - (*edge->point2)[0];

    // normalize
    len = 1.0/sqrt(axis[0]*axis[0] + axis[1]*axis[1]);
    axis[0] *= len;
    axis[1] *= len;

    project_to_axis(body1, &axis, &range1);
    project_to_axis(body2, &axis, &range2);

    dist = interv_dist(&range1, &range2);
    if (dist > 0.0)
      return false;
    else if (fabs(dist) < min_dist) {
      min_dist = fabs(dist);
      handler.normal[0] = axis[0];
      handler.normal[1] = axis[1];
      handler.edge = edge;
    }
  }

  handler.depth = min_dist;
  if (handler.edge->parent != body2) {
    // swap the bodies... its easier
    temp = body1;
    body1 = body2;
    body2 = temp;
  }

  xx = body1->center_of_mass[0] - body2->center_of_mass[0];
  yy = body1->center_of_mass[1] - body2->center_of_mass[1];
  dot = handler.normal[0] * xx + handler.normal[1] * yy;
  if (dot < 0.0) {
    handler.normal[0] = -handler.normal[0];
    handler.normal[1] = -handler.normal[1];
  }

  small_dist = 10000.0;
  for (i = 0; i < body1->num_points; i++) {
    xx = body1->points[i][0] - body2->center_of_mass[0];
    yy = body1->points[i][1] - body2->center_of_mass[1];
    dot = handler.normal[0] * xx + handler.normal[1] * yy;
    if (dot < small_dist) {
      small_dist = dot;
      handler.vertex = &body1->points[i];
      handler.parent = body1;
    }
  }

  return true;
}

// BB test
static bool bodies_overlapping(Body* body1, Body* body2) {
  // bbox = {minX minY maxX maxY}
  return (body1->bbox[0] <= body2->bbox[2])
      && (body1->bbox[1] <= body2->bbox[3])  
      && (body1->bbox[2] >= body2->bbox[0])  
      && (body1->bbox[3] >= body2->bbox[1]);
}

static bool test_callbacks1(void* vcb, void* vother) {
  CollisionCallback* cb = vcb;
  Body* other = vother;
  (void) other;
  if (cb->other == other) {
    cb->callback(cb->body, cb->other, cb->data);
    return true;
  }
  return false;
}

static bool do_bodies(void* vbody1, void* vbody2) {
  Body *body1 = vbody1, *body2 = vbody2;
  if (body1 != body2) {
    if (body1->mask & body2->mask) {
      if (bodies_overlapping(body1, body2)) {
        if (bodies_colliding(body1, body2)) {
          list_traverse(body1->collision_callbacks, test_callbacks1, body2);
          handle();
        }
      }
    }
  }
  return false;
}

static bool do_bodies_bodies(void* body, void* data) {
  list_traverse(bodies, do_bodies, body);
  return false;
}

void body_do_collisions(Body* body) {
  // Just do a pretty cheap n*n-body test
  list_traverse(bodies, do_bodies_bodies, NULL);
}

void body_do_center(Body* body) {
  int i;
  body->center_of_mass[0] = 0.0;
  body->center_of_mass[1] = 0.0;

  body->bbox[0] = 10000.0f;
  body->bbox[1] = 10000.0f;
  body->bbox[2] = -10000.0f;
  body->bbox[3] = -10000.0f;

  for (i = 0; i < body->num_points; i++) {
    body->center_of_mass[0] += body->points[i][0];
    body->center_of_mass[1] += body->points[i][1];

    body->bbox[0] = min(body->bbox[0], body->points[i][0]);
    body->bbox[1] = min(body->bbox[1], body->points[i][1]);
    body->bbox[2] = max(body->bbox[2], body->points[i][0]);
    body->bbox[3] = max(body->bbox[3], body->points[i][1]);
  }

  body->center_of_mass[0] /= body->num_points;
  body->center_of_mass[1] /= body->num_points;
}

void body_do_render(Body* body) {
  glBindBuffer(GL_ARRAY_BUFFER, body->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * body->num_points,
    body->points);
  glVertexAttribPointer(body_program->attribute[0], 2, GL_FLOAT, false,
    0, (void*)(0));
  glVertexAttribPointer(body_program->attribute[1], 3, GL_FLOAT, false,
    0, (void*)(sizeof(vec2) * body->num_points));
  if (body->wire == false)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, body->num_points);
  else
    glDrawArrays(GL_LINE_STRIP, 0, body->num_points);
}

void body_add_collision_callback(
  Body* body,
  Body* other,
  void (*callback)(Body*, Body*, void*),
  void* data)
{
  CollisionCallback* cb = malloc(sizeof(CollisionCallback));
  cb->body = body;
  cb->other = other;
  cb->callback = callback;
  cb->data = data;
  list_push_back(body->collision_callbacks, cb);
}

void body_set_logic(
  Body* body,
  void(*callback)(Body*, double, void*),
  void* data)
{
  body->step_callback = callback;
  body->step_data = data;
}
