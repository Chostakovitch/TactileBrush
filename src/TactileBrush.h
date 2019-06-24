#ifndef _TACTILE_BRUSH_H
#define _TACTILE_BRUSH_H

#include <utility>
#include <vector>

/**
 * @struct Stroke
 * @brief Describes a straight-line stroke (start, end, duration)
 */
struct Stroke {
  std::pair<float, float> start;
  std::pair<float, float> end;
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
  TactileBrush(unsigned int lines, unsigned int columns, float distance);

  /**
   * @brief Compute the activation steps for a given stroke
   * Given a straight line and a duration, this function computes which
   * actuators must be enabled at which time (soa), for how long (duration),
   * and with which intensity (useful for phantome actuators). These steps
   * are computed with the Tactile Brush algorithm.
   * @param  s Desired stroke
   * @return   Vector of ActuatorStep to get the desired haptic stroke illustion
   */
  std::vector<ActuatorStep> computeStroke(Stroke s);
private:
  // Lines
  unsigned int n;
  // Columns
  unsigned int m;
  // Distance between actuators
  float l;
};

#endif
