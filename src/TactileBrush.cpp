#include "TactileBrush.h"

std::set<ActuatorStep> TactileBrush::computeStroke(Stroke& s) {
  if(!(isPointWithinGrid(s.start) && isPointWithinGrid(s.end))) {
    throw std::out_of_range("Stroke start or end point out of the grid range");
  }
  computeVirtualPoints(s);
  computeMaxIntensityTimers(s);
  for(const auto& p : s.virtualPoints) printCoord(p);
  return std::set<ActuatorStep>();
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
    auto& e = s.virtualPoints[i];
    e.timerMaxIntensity = std::hypot(e.first - begin.first, e.second - begin.second) / speed;
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
