#include "TactileBrush.h"
#include "Stroke.h"

void TactileBrush::computeStroke(Stroke& s) {
  if(!(isPointWithinGrid(s.getStart()) && isPointWithinGrid(s.getEnd()))) {
    throw std::out_of_range("Stroke start or end point out of the grid range");
  }
  s.computeVirtualPoints(lines, columns, interDist);
  s.computeMaxIntensityTimers();
  s.computeDurationsAndSOAs();
  computePhysicalMapping(s.getVirtualPoints(), s.getIntensity());
}

void TactileBrush::computePhysicalMapping(const std::vector<ActuatorPoint>& virtualPoints, float globalIntensity) {
  for(const auto& e : virtualPoints) {
    // If virtual actuator is in fact physical, let full intensity
    if(std::fmod(e.first, interDist) < EPSILON && std::fmod(e.second, interDist) < EPSILON) {
      ActuatorStep step(std::round(e.first / interDist), std::round(e.second / interDist), globalIntensity, e.getDuration());
      insertActuatorStep(e.getSOA(), step);
    }

    // Otherwise, use phantom actuator energy model and compute intensities for
    // the two closest actuators (easy because virtual actuators are on the grid!)
    else {
      unsigned int l1, c1, l2, c2;
      // Virtual actuator is on a column
      if(std::fmod(e.first, interDist) < EPSILON) {
        c1 = c2 = std::round(e.first / interDist) * interDist;
        l1 = std::floor(e.second);
        l2 = std::ceil(e.second);
      }
      // Virtual actuator is on a line
      else if(std::fmod(e.second, interDist) < EPSILON) {
        l1 = l2 = std::round(e.second / interDist) * interDist;
        c1 = std::floor(e.first);
        c2 = std::ceil(e.first);
      }
      // Anormal case : virtual actuator is not on the grid
      else {
        std::ostringstream ss;
        ss << "Virtual actuator at position (" << e.first << "," << e.second << ") is not on the physical actuators' grid";
        throw std::logic_error(ss.str());
      }
      // Ratio of the distance (physical - virtual) over (physical - physical) :
      // tells us from which physical actuator the virtual actuator is closer
      float ratio = std::hypot(c1 - e.first, l1 - e.second) / std::hypot(c1 - c2, l1 - l2);

      // Adjust physical actuators' intensity according to the ratio
      ActuatorStep phy1(l1, c1, std::sqrt(1 - ratio) * globalIntensity, e.getDuration());
      ActuatorStep phy2(l2, c2, std::sqrt(ratio) * globalIntensity, e.getDuration());
      insertActuatorStep(e.getSOA(), phy1);
      insertActuatorStep(e.getSOA(), phy2);
    }
  }
}

bool TactileBrush::isPointWithinGrid(const ActuatorPoint& point) {
  // Assume the grid is aligned with the base vectors
  if(point.first < minCoord.first || point.first > maxCoord.first) return false;
  if(point.second < minCoord.second || point.second > maxCoord.second) return false;
  return true;
}

void TactileBrush::prettyPrint() {
  for(const auto& p : actuatorTriggers) {
    std::cout << "Time " << p.first << "ms :" << std::endl;
    for(const auto& s : p.second) {
      std::cout << "\t" << s;
    }
  }
}

void TactileBrush::insertActuatorStep(float time, ActuatorStep step) {
  auto const result = actuatorTriggers.insert(std::make_pair(time, std::vector<ActuatorStep>({ step })));
  if (!result.second) { result.first->second.push_back(step); }
}
