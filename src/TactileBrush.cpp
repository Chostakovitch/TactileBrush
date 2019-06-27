#include "TactileBrush.h"

void TactileBrush::computeStroke(Stroke& s) {
  if(!(isPointWithinGrid(s.start) && isPointWithinGrid(s.end))) {
    throw std::out_of_range("Stroke start or end point out of the grid range");
  }
  computeVirtualPoints(s);
  computeMaxIntensityTimers(s);
  computeDurationsAndSOAs(s);
  computePhysicalMapping(s);
  for(const auto& p : s.virtualPoints) printCoord(p);
}

void TactileBrush::computeVirtualPoints(Stroke& s) {
  // Ensure the comparison will ignore floating point errors
  auto cmp = [](ActuatorPoint a, ActuatorPoint b) {
    float diffX = b.first - a.first;
    if(diffX > EPSILON) return true;
    return std::abs(diffX) < EPSILON && b.second - a.second > EPSILON;
  };
  std::set<ActuatorPoint, decltype(cmp)> v(cmp);

  // Add first point
  v.insert(s.start);

  // Vertical segment : no slope
  if(s.start.first == s.end.first) {
    for(int l = 0; l < this->lines; ++l) {
      ActuatorPoint c(s.start.first, l * this->interDist);
      if(isPointOnSegment(c, s.start, s.end)) v.insert(c);
    }
  }
  else {
    // Stroke parameters
    float coef = (s.end.second - s.start.second) / (s.end.first - s.start.first);
    float orig = s.start.second - coef * s.start.first;

    // Find intersections of the slope with the grid (virtual points)
    for(int l = 0; l < this->lines; ++l) {
      float y = l * this->interDist;
      // If the fiber of the "line" is within the grid, add a virtual point
      ActuatorPoint ant((y - orig) / coef, y);
      if(isPointOnSegment(ant, s.start, s.end)) v.insert(ant);
    }
    for(int c = 0; c < this->columns; ++c) {
      float x = c * this->interDist;
      // If the image of the "column" is within the grid, add a virtual point
      ActuatorPoint res(x, coef * x + orig);
      if(isPointOnSegment(res, s.start, s.end)) v.insert(res);
    }
  }

  // Add last point
  v.insert(s.end);

  auto res = std::vector<ActuatorPoint>(v.begin(), v.end());
  // The direction of the movement matters a lot
  if(s.start > s.end) {
    std::reverse(res.begin(), res.end());
  }

  s.virtualPoints = res;
}

void TactileBrush::computeMaxIntensityTimers(Stroke& s) {
  float speed = std::hypot(s.start.first - s.end.first, s.start.second - s.end.second) / s.duration;
  ActuatorPoint begin = s.virtualPoints[0];

  // Divide distance from origin by speed, and get the **minimum** time when the actuator must reach its maximum intensity
  for(int i = 1; i < s.virtualPoints.size(); ++i) {
    ActuatorPoint& e = s.virtualPoints[i];
    e.timerMaxIntensity = std::hypot(e.first - begin.first, e.second - begin.second) / speed;
  }
}

// Computations are extrapolated from the paper, more details to come in README probably
void TactileBrush::computeDurationsAndSOAs(Stroke& s) {
  float sumSOA = 0;
  // First actuator is not triggered before first timerMaxIntensity, and last not after last timerMaxIntensity
  s.virtualPoints[0].durations.first = 0;
  s.virtualPoints[s.virtualPoints.size() - 1].durations.second = 0;
  s.virtualPoints[s.virtualPoints.size() - 1].soa = 0;

  for(auto it = s.virtualPoints.begin(); it != std::prev(s.virtualPoints.end()); ++it) {
    ActuatorPoint& e = *(it);
    sumSOA += (0.32f * (e.durations.first - sumSOA + (*(it + 1)).timerMaxIntensity) + 47.3f) / 1.32f;
    e.soa = sumSOA;
    e.durations.second = (*(it + 1)).timerMaxIntensity - sumSOA;
    (*(it + 1)).durations.first = e.durations.second;
  }

  // Duration of the last actuator is based on total previsional time minus all SOAs
  s.virtualPoints[s.virtualPoints.size() - 1].durations.first = s.duration - sumSOA;
}

void TactileBrush::computePhysicalMapping(Stroke& s) {
  for(const auto& e : s.virtualPoints) {
    // If virtual actuator is in fact physical, let full intensity
    if(std::fmod(e.first, interDist) < EPSILON && std::fmod(e.second, interDist) < EPSILON) {
      ActuatorStep step(std::round(e.first / interDist), std::round(e.second / interDist), 1, e.soa, e.durations.first + e.durations.second);
      s.actuatorTriggers.push_back(step);
    }
    // Otherwise, use phantom actuator energy model and compute intensities for
    // the two closest actuators (easy because virtual actuators are on the grid!)
    else {
      unsigned int l1, c1, l2, c2;
      // Virtual actuator is on a column
      if(std::fmod(e.first, interDist)) {
        c1 = c2 = std::round(e.first / interDist) * interDist;
        l1 = std::floor(e.second);
        l2 = std::ceil(e.second);
      }
      // Virtual actuator is on a line
      else if(std::fmod(e.second, interDist)) {
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
    }
  }
}

bool TactileBrush::isPointOnSegment(const ActuatorPoint& point, const ActuatorPoint& start, const ActuatorPoint& end) {
  float segDist = std::hypot(start.first - end.first, start.second - end.second);
  float startToPointDist = std::hypot(start.first - point.first, start.second - point.second);
  float pointToEndDist = std::hypot(end.first - point.first, end.second - point.second);
  // Avoid rounding errors ; if the point lies on the segment, the equality will be true, otherwise it's a triangle
  return startToPointDist + pointToEndDist - segDist < EPSILON;
}

bool TactileBrush::isPointWithinGrid(const ActuatorPoint& point) {
  // Assume the grid is aligned with the base
  if(point.first < minCoord.first || point.first > maxCoord.first) return false;
  if(point.second < minCoord.second || point.second > maxCoord.second) return false;
  return true;
}
