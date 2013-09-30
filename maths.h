/**
 * Useful maths
 * @author Scott LaVigne
 */
#ifndef MATHS_H
#define MATHS_H

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];

typedef int vec2i[2];
typedef int vec3i[3];
typedef int vec4i[4];

float max(float a, float b);
float min(float a, float b);
float clamp(float a, float min, float max);

#endif /* MATHS_H */