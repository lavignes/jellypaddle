/**
 * Functions pretaining to game and physics state: implementation
 * @author Scott LaVigne
 */
#include <stdint.h>
#include <time.h>

#include "game.h"
#include "list.h"
#include "shader.h"

bool GAME_KEY_PRESSED[256];
bool GAME_KEY_HELD[256];
bool GAME_KEY_RELEASED[256];

static bool window_open = true;
static clock_t start_time;
static double t0, t1;

extern Pipeline* body_program;
extern unsigned body_vao;
extern List* bodies;

static uint64_t raw_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
}

static double get_time() {
  return (double) (raw_time() - start_time) * 1e-9;
}

static void keyboard_down(unsigned char key, int x, int y) {
  GAME_KEY_PRESSED[key] = true;
  GAME_KEY_HELD[key] = true;
  GAME_KEY_RELEASED[key] = false;
}

static void keyboard_up(unsigned char key, int x, int y) {
  GAME_KEY_PRESSED[key] = false;
  GAME_KEY_HELD[key] = false;
  GAME_KEY_RELEASED[key] = true;
}

static void keyboard_special_down(int key, int x, int y) {
  GAME_KEY_PRESSED[key] = true;
  GAME_KEY_HELD[key] = true;
  GAME_KEY_RELEASED[key] = false;
}

static void keyboard_special_up(int key, int x, int y) {
  GAME_KEY_PRESSED[key] = false;
  GAME_KEY_HELD[key] = false;
  GAME_KEY_RELEASED[key] = true;
}

static void window_close() {
  window_open = false;
}

static void reprocess_keys() {
  int i;
  for (i = 0; i < 256; i++) {
    GAME_KEY_PRESSED[i] &= false;
    GAME_KEY_RELEASED[i] &= false;
  }
}

void game_init(int* argc, char** argv, const char* title) {
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(800, 600);
  glutInitContextVersion(3, 1);
  glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);
  glutCreateWindow(title);
  glewExperimental = true;
  glewInit();

  glutKeyboardFunc(keyboard_down);
  glutKeyboardUpFunc(keyboard_up);
  glutSpecialFunc(keyboard_special_down);
  glutSpecialUpFunc(keyboard_special_up);
  glutCloseFunc(window_close);

  glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  bodies = list_new();
  body_program = pipeline_new(
    shader_new(SHADER_VERTEX, "body.vert"),
    shader_new(SHADER_FRAGMENT, "body.frag"));

  glGenVertexArrays(1, &body_vao);
  glBindVertexArray(body_vao);

  pipeline_attribute(body_program, "coord", 0);
  pipeline_attribute(body_program, "color", 1);
}

static bool do_verlet(void* body, void* dt) {
  body_do_verlet(body, *(double*) dt);
  return false;
}

static bool do_step(void* body, void* dt) {
  body_do_step(body, *(double*) dt);
  return false;
}

static bool do_edges(void* body, void* data) {
  body_do_edges(body);
  return false;
}

static bool do_center(void* body, void* data) {
  body_do_center(body);
  return false;
}

static bool do_collisions(void* body, void* data) {
  body_do_collisions(body);
  return false;
}

static bool do_render(void* body, void* data) {
  body_do_render(body);
  return false;
}

static void step(int value) {
  static double dt;
  int i;
  t1 = get_time();
  dt = t1 - t0;
  t0 = t1;

  reprocess_keys();
  list_traverse(bodies, do_step, &dt);
  list_traverse(bodies, do_verlet, &dt);

  for (i = 0; i < 5; i++) {
    list_traverse(bodies, do_edges, NULL);
    list_traverse(bodies, do_center, NULL);
    list_traverse(bodies, do_collisions, NULL);
  }

  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(body_program->id);
  glEnableVertexAttribArray(body_program->attribute[0]);
  glEnableVertexAttribArray(body_program->attribute[1]);
  list_traverse(bodies, do_render, &dt);

  glutSwapBuffers();
  glutTimerFunc(16.6667, step, 0);
}

void game_run() {
  glutTimerFunc(16.6667, step, 0);
  start_time = raw_time();
  t0 = get_time();
  glutMainLoop();
}

void game_set_title(const char* title) {
  glutSetWindowTitle(title);
}

void game_add_body(Body* body) {
  list_push_back(bodies, body);
}
