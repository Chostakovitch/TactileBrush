#ifndef _STROKE_H
#define _STROKE_H

#include <utility>
#include <set>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <sstream>

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
  std::pair<float, float> durations; ///< First member is duration before timerMaxIntensity, second is duration after timerMaxIntensity
  float soa; ///< In msec **from the start of the stroke**
};

/**
 * @class Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 * Also, contains two fields describing control points of the line between start and end,
 * and a vector of physical actuators activation in order to create an haptic illusion of movement
 */
class Stroke {
public:
  ActuatorPoint start;
  ActuatorPoint end;
  float duration; ///< In msec
  float intensity; ///< Global intensity between 0 and 1

  Stroke(ActuatorPoint start, ActuatorPoint end, float duration, float intensity) :
    start(start), end(end), duration(duration), intensity(intensity) {}
  /**
   * @brief Computes control points for the stroke
   * The stroke line is defined by its start and ending points. We choose, as in the original paper,
   * to compute intersections between the stroke line and the grid lines and make them "virtual actuators",
   * so we can use phantom actuator illusion. Doing so allow to use all the actuators we have at their best.
   * @param lines     Number of lines in the grid
   * @param columns   Number of columns in the grid
   * @param interDist Distance (cm) between two points of the grid
   */
  void computeVirtualPoints(float lines, float columns, float interDist);

  /**
   * @brief Computes the time, in msec, when the virtual actuator must reach its maximum intensity
   */
  void computeMaxIntensityTimers();

  /**
  * @brief Computes the duration of each virtual actuator, and the SOA, i.e. interval between two actuator triggers
  */
  void computeDurationsAndSOAs();

  inline const std::vector<ActuatorPoint>& getVirtualPoints() {
    return virtualPoints;
  }
private:
  std::vector<ActuatorPoint> virtualPoints;
  bool isPointOnSegment(const ActuatorPoint& point, const ActuatorPoint& start, const ActuatorPoint& end);
};
#endif
