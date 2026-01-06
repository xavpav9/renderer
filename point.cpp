#include "renderer.hpp"

// uses similar triangles to get the projection of a 2d position on a screen
std::array<int, 2> get2dPos(std::array<float, 3> pos, int focalLength) {
  if (pos[1] == 0) {
    pos[1] -= 1;
  }

  // camera pos is at (0,0,0) in rendered world.
  int newX = ((0 - pos[0]) * (focalLength) / (0 - pos[1]));
  int newY = ((0 - pos[2]) * (focalLength) / (0 - pos[1]));
  std::array<int, 2> newPos = {newX, newY};
  return newPos;
}

// uses the Bresenham line algorithm to find the points on a 2d screen which a 2d line goes through.
// it also records the depth of each point for the zbuffer
std::tuple<std::vector<std::array<int,2>>, std::vector<float>> getPoints(std::array<int, 2> startPos, float startDepth, std::array<int, 2> endPos, float endDepth) {
  std::vector<std::array<int,2>> points;
  std::vector<float> depths;

  int axis;
  float gradient = (float)(startPos[1] - endPos[1]) / (float)(startPos[0] - endPos[0]);

  if (gradient < -1 || gradient > 1) {
    axis = 1;
  } else {
    axis = 0;
  }

  std::array<int, 2> initialPos;
  std::array<int, 2> finishPos;
  float initialDepth;
  float finishDepth;

  if (startPos[axis] <= endPos[axis]) {
    for (int i = 0; i < 2; ++i) {
      initialPos[i] = startPos[i];
      finishPos[i] = endPos[i];
      initialDepth = startDepth;
      finishDepth = endDepth;
    }
  } else {
    for (int i = 0; i < 2; ++i) {
      initialPos[i] = endPos[i];
      finishPos[i] = startPos[i];
      initialDepth = endDepth;
      finishDepth = startDepth;
    }
  }

  if (axis == 0) {
    for (int x = initialPos[0]; x <= finishPos[0]; ++x) {
      int y = std::round(gradient * (x - initialPos[0]) + initialPos[1]);
      std::array<int, 2> point = {x, y};
      points.push_back(point);

      float depth;
      if (finishPos[0] == initialPos[0]) {
        depth = initialDepth;
      } else {
        depth = (float)(x - initialPos[0]) / (float)(finishPos[0] - initialPos[0]) * (finishDepth - initialDepth) + initialDepth; // linear interpolation
      }
      depths.push_back(depth);
    }
  } else {
    for (int y = initialPos[1]; y <= finishPos[1]; ++y) {
      int x = std::round((y - initialPos[1]) / gradient + initialPos[0]);
      std::array<int, 2> point = {x, y};
      points.push_back(point);

      float depth;
      if (finishPos[1] == initialPos[1]) {
        depth = initialDepth;
      } else {
        depth = (float)(y - initialPos[1]) / (float)(finishPos[1] - initialPos[1]) * (finishDepth - initialDepth) + initialDepth; // linear interpolation
      }
      depths.push_back(depth);
    }
  }

  return  { points, depths };
}
