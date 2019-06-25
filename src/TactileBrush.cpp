#include "TactileBrush.h"

std::set<ActuatorStep> TactileBrush::computeStroke(const Stroke& s) {
  if(!(isPointWithinGrid(s.start) && isPointWithinGrid(s.end))) {
    throw std::out_of_range("Stroke start or end point out of the grid range");
  }
  auto virtual_points = computeVirtualPoints(s);
  for(const auto& p : virtual_points) printCoord(p);
  return std::set<ActuatorStep>();
}

std::vector<Coord> TactileBrush::computeVirtualPoints(const Stroke& s) {
  // Ensure the comparison will ignore floating point errors
  auto cmp = [](Coord a, Coord b) {
    if(b.first - a.first > EPSILON) return true;
    return b.second - a.second > EPSILON;
  };
  std::set<Coord, decltype(cmp)> v(cmp);

  // Add first point
  v.insert(s.start);

  // Vertical segment : no slope
  if(s.start.first == s.end.first) {
    for(int l = 0; l < this->lines; ++l) {
      Coord c(s.start.first, l * this->interDist);
      if(isPointOnSegment(c, s.start, s.end)) {
        v.insert(c);
      }
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
      Coord ant((y - orig) / coef, y);
      if(isPointOnSegment(ant, s.start, s.end)) {
        v.insert(ant);
      }
    }
    for(int c = 0; c < this->columns; ++c) {
      float x = c * this->interDist;
      // If the image of the "column" is within the grid, add a virtual point
      Coord res(x, coef * x + orig);
      if(isPointOnSegment(res, s.start, s.end)) {
        v.insert(res);
      }
    }
  }

  // Add last point
  v.insert(s.end);

  auto res = std::vector<Coord>(v.begin(), v.end());
  // The direction of the movement matters a lot
  if(s.start > s.end) {
    std::reverse(res.begin(), res.end());
  }

  return res;
}

bool TactileBrush::isPointOnSegment(const Coord& point, const Coord& start, const Coord& end) {
  float segDist = std::hypot(start.first - end.first, start.second - end.second);
  float startToPointDist = std::hypot(start.first - point.first, start.second - point.second);
  float pointToEndDist = std::hypot(end.first - point.first, end.second - point.second);
  // Avoid rounding errors ; if the point lies on the segment, the equality will be true, otherwise it's a triangle
  return startToPointDist + pointToEndDist - segDist < EPSILON;
}

bool TactileBrush::isPointWithinGrid(const Coord& point) {
  // Assume the grid is aligned with the base
  if(point.first < minCoord.first || point.first > maxCoord.first) return false;
  if(point.second < minCoord.second || point.second > maxCoord.second) return false;
  return true;
}
