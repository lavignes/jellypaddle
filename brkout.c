#include <stdio.h>
#include <time.h>
#include <string.h>
#include "game.h"

static int broken = 0;       // how many broken bricks
static int score = 10000;    // score, it goes down as time passes
static float response = 30.0;// amount of force the paddle applies
static Body* bricks[8];      // bricks

/**
 * prototype for paddle
 */
static vec2 paddle_points[8] = {
 {0.0, 16+0.0},
 {0.0, 16+32.0},
 {32.0, 16+0.0},
 {32.0, 16+32.0},
 {64.0, 16+0.0},
 {64.0, 16+32.0},
 {96.0, 16+0.0},
 {96.0, 16+32.0},
};

static vec2i paddle_edges[10] = {
  {0, 1},
  {1, 3},
  {3, 5},
  {5, 7},
  {7, 6},
  {6, 4},
  {4, 2},
  {2, 0},
  {0, 7},
  {1, 6},
};

static vec3 paddle_colors[8] = {
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
 {1.0, 0.0, 0.0},
};

static void paddle_logic(Body* paddle, double dt, void* data) {

  if (GAME_KEY_HELD[GLUT_KEY_UP]) {
    paddle->points[0][1] += 1.5;
    paddle->points[1][1] += 1.5;
    paddle->points[7][1] += 1.5;
    paddle->points[6][1] += 1.5;
  } else if (GAME_KEY_HELD[GLUT_KEY_DOWN]) {
    paddle->points[0][1] -= 0.5;
    paddle->points[1][1] -= 0.5;
    paddle->points[7][1] -= 0.5;
    paddle->points[6][1] -= 0.5;
  }

  if (GAME_KEY_HELD[GLUT_KEY_RIGHT]) {
    paddle->points[0][0] += 0.5;
    paddle->points[1][0] += 0.5;
    paddle->points[7][0] += 0.5;
    paddle->points[6][0] += 0.5;
  } else if (GAME_KEY_HELD[GLUT_KEY_LEFT]) {
    paddle->points[0][0] -= 0.5;
    paddle->points[1][0] -= 0.5;
    paddle->points[7][0] -= 0.5;
    paddle->points[6][0] -= 0.5;
  }

  //This makes the paddle hover 16 pixels above the bottom
  int i;
  for (i = 0; i < paddle->num_points; i++) {
    paddle->points[i][1] = max(paddle->points[i][1], 16.0);
  }
}

/**
 * Prototype for ball
 */
static vec2 ball_points[5] = {
 {400+1.0, 400+13.0},
 {400+7.0, 400+29.0},
 {400+15.0, 400+1.0},
 {400+24.0, 400+29.0},
 {400+30.0, 400+13.0},
};

static vec2i ball_edges[10] = {
 {0, 1},
 {1, 3},
 {3, 4},
 {4, 2},
 {2, 0},
 {1, 2},
 {2, 3},
 {0, 4},
 {4, 1},
 {3, 0},
};

static vec3 ball_colors[5] = {
 {0.0, 0.0, 1.0},
 {0.0, 0.0, 1.0},
 {0.0, 0.0, 1.0},
 {0.0, 0.0, 1.0},
 {0.0, 0.0, 1.0},
};

static void ball_logic(Body* ball, double dt, void* data) {
  int i, j;
  for (i = 0; i < ball->num_points; i++) {
    if (ball->points[i][1] < 2) {
      memcpy(ball->points, ball_points, sizeof(vec2) * ball->num_points);
      memcpy(ball->last_points, ball_points, sizeof(vec2) * ball->num_points);
      for (j = 0; j < ball->num_points; j++)
        ball->points[j][0] += (rand() % 10+1)-5;
      break;
    }
  }
  if (broken < 7) {
    score -= 1;
  }
}

/**
 * Collision callback for ball
 */
static void ball_extra_bounce(Body* paddle, Body* ball, void* data) {
  int i;
  for (i = 0; i < ball->num_points; i++) {
    ball->points[i][1] += response;
  }
}

/**
 * Prototype for brick
 */
static vec3 brick_colors[8] = {
 {1.0, 1.0, 1.0},
 {0.0, 0.0, 0.0},
 {1.0, 1.0, 1.0},
 {0.0, 0.0, 0.0},
 {1.0, 1.0, 1.0},
 {0.0, 0.0, 0.0},
 {1.0, 1.0, 1.0},
 {0.0, 0.0, 0.0},
};

/**
 * Collision callback for brick
 */
static void brick_hit(Body* brick, Body* ball, void* data) {
  int i, j;
  if (brick->gravity == false) {
    broken++;
    brick->gravity = true;
    brick->wire = true;
    // if all bricks broken
    if (broken == 7) {
      broken = 0;
      // make game harder
      response /= 2;
      char buffer[256];
      sprintf(buffer, "SCORE: %d", score);
      score = 10000;
      game_set_title((const char*) buffer);
      // Reset bricks to initial position
      for (i = 0; i < 7; i++) {
        bricks[i]->gravity = false;
        bricks[i]->wire = false;
        bricks[i]->boxed = false;
        memcpy(bricks[i]->points, paddle_points, sizeof(vec2) * bricks[i]->num_points);
        memcpy(bricks[i]->last_points, paddle_points, sizeof(vec2) * bricks[i]->num_points);
        for (j = 0; j < bricks[i]->num_points; j++) {
          bricks[i]->last_points[j][0] += 48+100*i;
          bricks[i]->points[j][0] += 48+100*i;
          bricks[i]->last_points[j][1] += 500;
          bricks[i]->points[j][1] += 500;
        }
      }
    }
  }
}

int main(int argc, char** argv) {
  int i, j;
  srand(time(NULL));
  game_init(&argc, argv, "jelly paddle");

  Body* paddle = body_new(paddle_points, paddle_colors, 8, paddle_edges, 10);
  Body* ball = body_new(ball_points, ball_colors, 5, ball_edges, 10);
  body_add_collision_callback(paddle, ball, ball_extra_bounce, NULL);

  body_set_logic(paddle, paddle_logic, NULL);
  game_add_body(paddle);

  ball->mask = 0xFFF;
  body_set_logic(ball, ball_logic, NULL);
  game_add_body(ball);

  // Place bricks
  for (i = 0; i < 7; i++) {
    bricks[i] = body_new(paddle_points, brick_colors, 8, paddle_edges, 10);
    body_add_collision_callback(bricks[i], ball, brick_hit, NULL);
    bricks[i]->gravity = false;
    bricks[i]->boxed = false;
    bricks[i]->mass *= 2;
    bricks[i]->mask = 0x02 << i;
    for (j = 0; j < bricks[i]->num_points; j++) {
      bricks[i]->last_points[j][0] += 48+100*i;
      bricks[i]->points[j][0] += 48+100*i;
      bricks[i]->last_points[j][1] += 500;
      bricks[i]->points[j][1] += 500;
    }
    game_add_body(bricks[i]);
  }

  game_run();

  return 0;
}
