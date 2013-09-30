/**
 * Useful maths: implementation
 * @author Scott LaVigne
 */
#include "maths.h"

float max(float a, float b) {
  return (a > b)? a : b;
}

float min(float a, float b) {
  return (a < b)? a : b;
}

float clamp(float a, float min, float max) {
  return (a < min)? min : (a > max)? max : a;
}