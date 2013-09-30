/**
 * Functions pretaining to game and physics state
 * @author Scott LaVigne
 */
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include "body.h"

/**
 * Array of keystates for testing key being pressed in a frame
 */
extern bool GAME_KEY_PRESSED[];

/**
 * Array of keystates for testing key being held in a frame.
 * Essentially, this tells you if you are still pressing a key you
 * pressed last frame.
 */
extern bool GAME_KEY_HELD[];

/**
 * Array of keystates for testing key being released in a frame.
 */
extern bool GAME_KEY_RELEASED[];

/**
 * Initialize a physics world.
 * @param argc  passed from main
 * @param argv  passed from main
 * @param title a window title to use initially
 */
void game_init(int* argc, char** argv, const char* title);

/**
 * Start the physics step.
 */
void game_run();

/**
 * Change the window title.
 * @param title a new title to use
 */
void game_set_title(const char* title);

/**
 * Add a body to the physics world.
 * @param body a body to add
 */
void game_add_body(Body* body);

#endif /* GAME_H */
