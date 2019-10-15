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
  ActuatorPoint() {}
  ActuatorPoint(float x, float y) : std::pair<float, float>(x, y), timerMaxIntensity(0) {}
  inline float getDuration() const { return durations.first + durations.second; }
  inline float getStart() const { return start; }
  float timerMaxIntensity;
  std::pair<float, float> durations; ///< First member is duration before timerMaxIntensity, second is duration after timerMaxIntensity
  float start; ///< In msec **from the start of the stroke**
};

inline std::ostream& operator<<(std::ostream &os, const ActuatorPoint& m) {
    std::ostringstream oss;
    oss << "Virtual actuator at position (" << m.first << "cm," << m.second << "cm)";
    oss << " triggered at " << m.getStart() << " ms during " << m.getDuration() << "msec.";
    oss << " Max intensity reached at " << m.timerMaxIntensity << "msec.";
    return std::cout << oss.str() << std::endl;
}

/**
 * @class Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 * and has methods to compute virtual points and parameters of movement
 */
class Stroke {
public:
  Stroke(float startX, float startY, float endX, float endY, float duration, float intensity) :
    startX(startX), startY(startY), endX(endX), endY(endY),
    duration(duration), intensity(intensity) {}

  /**
   * Compute all virtual actuators' positions,
   * as well as their time and duration of activation.
   * @param lines     Number of lines in the grid
   * @param columns   Number of columns in the grid
   * @param interDist Distance (cm) between two points of the grid
   * @returns Vector of ActuatorPoints corresponding to virtual actuators and their parameters
   */
  const std::vector<ActuatorPoint>& computeParameters(float lines, float columns, float interDist);
  inline float getDuration() const { return duration; }
  inline float getIntensity() const { return intensity; }

  inline const ActuatorPoint& getStart() const { return start; }
  inline const ActuatorPoint& getEnd() const { return end; }

  void prettyPrint();
private:
  /**
   * @brief Computes control points for the stroke
   * The stroke line is defined by its start and ending points. We choose, as in the original paper,
   * to compute intersections between the stroke line and the grid lines and make them "virtual actuators",
   * so we can use phantom actuator illusion. Doing so allow to use all the actuators we have at their best.
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
  bool isPointOnStroke(const ActuatorPoint& point);

  std::vector<ActuatorPoint> virtualPoints;
  ActuatorPoint start, end;
  float startX, endX, startY, endY;
  float duration; ///< In msec
  float intensity; ///< Global intensity between 0 and 1
};
#endif
