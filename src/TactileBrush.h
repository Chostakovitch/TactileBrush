#ifndef _TACTILE_BRUSH_H
#define _TACTILE_BRUSH_H

#include <utility>
#include <set>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>

const float EPSILON = 0.001f;

/**
 * @struct Coord
 * @brief A pair of float, representing 2D coordinates in the actuators grid
 * Coordinates are intented to be in centimeters (so they already take into account
 * distance between actuators)
 */
typedef std::pair<float, float> Coord;

/**
 * @struct Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 */
struct Stroke {
  Coord start;
  Coord end;
  float duration;
};

/**
 * @struct ActuatorStep
 * @brief Describes the activation of an actuator used to build the stroke
 */
struct ActuatorStep {
  unsigned int line;
  unsigned int column;
  float intensity;
  float soa;
  float duration;
};

class TactileBrush {
public:
  /**
   * @brief Initialize tactile brush grid
   * @param lines    Number of lines of the grid
   * @param columns  Number of columns of the grid
   * @param distance Distance between two actuators
   */
  TactileBrush(unsigned int lines, unsigned int columns, float distance) : lines(lines - 1), columns(columns - 1), interDist(distance) {
    minCoord = Coord(0, 0);
    maxCoord = Coord(columns * interDist, lines * interDist);
  }

  /**
   * @brief Compute the activation steps for a given stroke
   * Given a straight line and a duration, this function computes which
   * actuators must be enabled at which time (soa), for how long (duration),
   * and with which intensity (useful for phantome actuators). These steps
   * are computed with the Tactile Brush algorithm.
   * @param  s Desired stroke
   * @return   Vector of ActuatorStep to get the desired haptic stroke illustion
   */
  std::set<ActuatorStep> computeStroke(const Stroke& s);
private:
  unsigned int lines;
  unsigned int columns;
  float interDist;

  Coord minCoord;
  Coord maxCoord;

  std::vector<Coord> computeVirtualPoints(const Stroke& s);

  bool isPointOnSegment(const Coord& point, const Coord& start, const Coord& end);
  bool isPointWithinGrid(const Coord& point);
  inline void printCoord(const Coord& c) {
    std::cout << "(" << c.first << "," << c.second << ")" << std::endl;
  }
};

#endif
