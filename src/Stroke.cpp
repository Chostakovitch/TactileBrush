#include "Stroke.h"

void Stroke::computeVirtualPoints(float lines, float columns, float interDist) {
  // Ensure the comparison will ignore floating point errors
  auto cmp = [](ActuatorPoint a, ActuatorPoint b) {
    float diffX = b.first - a.first;
    if(diffX > EPSILON) return true;
    return std::abs(diffX) < EPSILON && b.second - a.second > EPSILON;
  };
  std::set<ActuatorPoint, decltype(cmp)> v(cmp);

  // Add first point
  v.insert(start);

  // Vertical segment : no slope
  if(start.first == end.first) {
    for(int l = 0; l < lines; ++l) {
      ActuatorPoint c(start.first, l * interDist);
      if(isPointOnSegment(c, start, end)) v.insert(c);
    }
  }
  else {
    // Stroke parameters
    float coef = (end.second - start.second) / (end.first - start.first);
    float orig = start.second - coef * start.first;

    // Find intersections of the slope with the grid (virtual points)
    for(int l = 0; l < lines; ++l) {
      float y = l * interDist;
      // If the fiber of the "line" is within the grid, add a virtual point
      ActuatorPoint ant((y - orig) / coef, y);
      if(isPointOnSegment(ant, start, end)) v.insert(ant);
    }
    for(int c = 0; c < columns; ++c) {
      float x = c * interDist;
      // If the image of the "column" is within the grid, add a virtual point
      ActuatorPoint res(x, coef * x + orig);
      if(isPointOnSegment(res, start, end)) v.insert(res);
    }
  }

  // Add last point
  v.insert(end);

  auto res = std::vector<ActuatorPoint>(v.begin(), v.end());
  // The direction of the movement matters a lot
  if(start > end) {
    std::reverse(res.begin(), res.end());
  }

  virtualPoints = res;
}

void Stroke::computeMaxIntensityTimers() {
  float speed = std::hypot(start.first - end.first, start.second - end.second) / duration;
  ActuatorPoint begin = virtualPoints[0];

  // Divide distance from origin by speed, and get the **minimum** time when the actuator must reach its maximum intensity
  for(int i = 1; i < virtualPoints.size(); ++i) {
    ActuatorPoint& e = virtualPoints[i];
    e.timerMaxIntensity = std::hypot(e.first - begin.first, e.second - begin.second) / speed;
  }
}

// Computations are extrapolated from the paper, more details to come in README probably
void Stroke::computeDurationsAndSOAs() {
  float sumSOA = 0;
  // First actuator is not triggered before first timerMaxIntensity, and last not after last timerMaxIntensity
  virtualPoints[0].durations.first = 0;
  virtualPoints[0].start = 0;
  virtualPoints[virtualPoints.size() - 1].durations.second = 0;

  for(auto it = virtualPoints.begin(); it != std::prev(virtualPoints.end()); ++it) {
    ActuatorPoint& e = *(it);
    sumSOA += (0.32f * (e.durations.first - sumSOA + (*(it + 1)).timerMaxIntensity) + 47.3f) / 1.32f;
    (*(it + 1)).start = sumSOA;
    e.durations.second = (*(it + 1)).timerMaxIntensity - sumSOA;
    (*(it + 1)).durations.first = e.durations.second;
  }

  // Duration of the last actuator is based on total previsional time minus all SOAs
  virtualPoints[virtualPoints.size() - 1].durations.first = duration - sumSOA;

}

bool Stroke::isPointOnSegment(const ActuatorPoint& point, const ActuatorPoint& start, const ActuatorPoint& end) {
  float segDist = std::hypot(start.first - end.first, start.second - end.second);
  float startToPointDist = std::hypot(start.first - point.first, start.second - point.second);
  float pointToEndDist = std::hypot(end.first - point.first, end.second - point.second);
  // Avoid rounding errors ; if the point lies on the segment, the equality will be true, otherwise it's a triangle
  return startToPointDist + pointToEndDist - segDist < EPSILON;
}
