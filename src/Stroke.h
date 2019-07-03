#ifndef _STROKE_H
#define _STROKE_H

#include <utility>
#include <set>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>

// Floating point rounding limit
const float EPSILON = 0.001f;

/**
 * @struct ActuatorPoint
 * @brief A pair of float, representing 2D coordinates in the actuators grid (either physical or virtual actuators)
 * Coordinates are intented to be in centimeters (so they already take into account
 * distance between actuators)
 */
struct ActuatorPoint: public std::pair<float, float> {
  ActuatorPoint(float x, float y) : std::pair<float, float>(x, y), timerMaxIntensity(0) {}
  inline float getDuration() const { return durations.first + durations.second; }
  inline float getStart() const { return start; }
  float timerMaxIntensity;
  std::pair<float, float> durations; ///< First member is duration before timerMaxIntensity, second is duration after timerMaxIntensity
  float start; ///< In msec **from the start of the stroke**
};

inline std::ostream& operator<<(std::ostream &os, const ActuatorPoint& m) {
    std::ostringstream oss;
    oss << "Virtual actuator at position (" << m.first << "," << m.second << ")";
    oss << " triggered at " << m.getStart() << " ms during " << m.getDuration() << "msec";
    return std::cout << oss.str() << std::endl;
}

/**
 * @class Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 * Also, contains a field describing the "control points" of the stroke, i.e.
 * points belonging to the line
 */
class Stroke {
public:
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

  inline float getDuration() const { return duration; }
  inline float getIntensity() const { return intensity; }

  inline const std::vector<ActuatorPoint>& getVirtualPoints() const { return virtualPoints; }
  inline const ActuatorPoint& getStart() const { return start; }
  inline const ActuatorPoint& getEnd() const { return end; }

  void prettyPrint();
private:
  std::vector<ActuatorPoint> virtualPoints;
  bool isPointOnSegment(const ActuatorPoint& point, const ActuatorPoint& start, const ActuatorPoint& end);

  ActuatorPoint start;
  ActuatorPoint end;
  float duration; ///< In msec
  float intensity; ///< Global intensity between 0 and 1
};
#endif
