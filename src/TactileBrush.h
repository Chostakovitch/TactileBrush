#ifndef _TACTILE_BRUSH_H
#define _TACTILE_BRUSH_H

#include "Stroke.h"
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>

/**
 * @struct ActuatorStep
 * @brief Describes the activation of an **physical** actuator used to build the stroke
 */
struct ActuatorStep {
  ActuatorStep(unsigned int column, unsigned int line, float intensity, float duration) :
    line(line), column(column), intensity(intensity), duration(duration) {}
  unsigned int line; ///< Line number of the physical actuator, NOT CENTIMERS
  unsigned int column; ///< Column number of the physical actuator, NOT CENTIMERS
  float intensity; ///< On the scale [0, 1]
  float duration; ///< In miliseconds
};

inline std::ostream& operator<<(std::ostream &os, const ActuatorStep& m) {
    std::ostringstream oss;
    oss << "Actuator at position (" << m.column << "," << m.line << ")";
    oss << " triggered during " << m.duration << "msec";
    oss << " with intensity " << m.intensity;
    return std::cout << oss.str() << std::endl;
}

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
   */
  void computeStroke(Stroke& s);

  void prettyPrint();

  const std::map<float, std::vector<ActuatorStep>>& getMotion() const { return actuatorTriggers; }
private:
  unsigned int lines;
  unsigned int columns;
  float interDist;

  ActuatorPoint minCoord;
  ActuatorPoint maxCoord;

  std::map<float, std::vector<ActuatorStep>> actuatorTriggers;

  /**
   * @brief From SOAs and duration for virtual actuators, compute SOAs and duration for physical actuators
   * @param virtualPoints   Virtual points (with SOA and duration) to map on physical actuators
   * @param globalIntensity Desired intensity for phantom actuators
   */
  void computePhysicalMapping(const std::vector<ActuatorPoint>& virtualPoints, float globalIntensity);
  bool isPointWithinGrid(const ActuatorPoint& point);


  /**
   * Small helper to update the <time, actuator> trigger map
   * @param time Time to trigger actuator, according to the beginning of the stroke
   * @param step Parameters of the physical actuator activation
   */
  void insertActuatorStep(float time, ActuatorStep step);
};

#endif
