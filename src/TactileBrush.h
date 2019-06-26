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
 * @brief A pair of float, representing 2D coordinates in the actuators grid (either physical or virtual actuators)
 * Coordinates are intented to be in centimeters (so they already take into account
 * distance between actuators)
 */
struct ActuatorPoint: public std::pair<float, float> {
  ActuatorPoint(float x, float y) : std::pair<float, float>(x, y), timerMaxIntensity(0) {}
  float timerMaxIntensity;
};

/**
 * @struct ActuatorStep
 * @brief Describes the activation of an **physical** actuator used to build the stroke
 */
struct ActuatorStep {
  unsigned int line;
  unsigned int column;
  float intensity;
  float soa;
  float duration;
};

/**
 * @struct Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 * Also, contains two fields describing control points of the line between start and end,
 * and a vector of physical actuators activation in order to create an haptic illusion of movement
 */
struct Stroke {
  ActuatorPoint start;
  ActuatorPoint end;
  float duration;
  std::vector<ActuatorPoint> virtualPoints;
  std::vector<ActuatorStep> actuatorTriggers;
};


class TactileBrush {
public:
  /**
   * @brief Initialize tactile brush grid
   * @param lines    Number of lines of the grid
   * @param columns  Number of columns of the grid
   * @param distance Distance between two actuators
   */
  TactileBrush(unsigned int lines, unsigned int columns, float distance) :
    lines(lines - 1),
    columns(columns - 1),
    interDist(distance),
    minCoord(ActuatorPoint(0, 0)),
    maxCoord(ActuatorPoint(columns * interDist, lines * interDist)) {}

  /**
   * @brief Compute the activation steps for a given stroke
   * Given a straight line and a duration, this function computes which
   * actuators must be enabled at which time (soa), for how long (duration),
   * and with which intensity (useful for phantome actuators). These steps
   * are computed with the Tactile Brush algorithm.
   * @param  s Desired stroke
   * @return   Vector of ActuatorStep to get the desired haptic stroke illustion
   */
  std::set<ActuatorStep> computeStroke(Stroke& s);
private:
  unsigned int lines;
  unsigned int columns;
  float interDist;

  ActuatorPoint minCoord;
  ActuatorPoint maxCoord;

  /**
   * @brief Computes control points for the stroke
   * The stroke line is defined by its start and ending points. We choose, as in the original paper,
   * to compute intersections between the stroke line and the grid lines and make them "virtual actuators",
   * so we can use phantom actuator illusion. Doing so allow to use all the actuators we have at their best.
   * @param  s Previsonial **valid** stroke
   */
  void computeVirtualPoints(Stroke& s);

  /**
   * @brief Computes the time, in sec, when the virtual actuator must reach its maximum intensity
   * @param  virtualPoints Vector of ordered virtual actuators describing the stroke path
   * @param  s             Desired stroke
   */
   void computeMaxIntensityTimers(Stroke& s);

  bool isPointOnSegment(const ActuatorPoint& point, const ActuatorPoint& start, const ActuatorPoint& end);
  bool isPointWithinGrid(const ActuatorPoint& point);
  inline void printCoord(const ActuatorPoint& c) {
    std::cout << "(" << c.first << "," << c.second << ") in " << c.timerMaxIntensity << "s" << std::endl;
  }
};

#endif
