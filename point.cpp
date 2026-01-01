#include "renderer.hpp"

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

std::tuple<std::vector<std::array<int,2>>, std::vector<float>> getPoints(std::array<int, 2> startPos, float startDepth, std::array<int, 2> endPos, float endDepth) {
  std::vector<std::array<int,2>> points;
  std::vector<float> depths;
  float closerDepth = std::min(startDepth, endDepth);

  int axis;
  float gradient = (float)(startPos[1] - endPos[1]) / (float)(startPos[0] - endPos[0]);

  if (gradient < -1 || gradient > 1) {
    axis = 1;
  } else {
    axis = 0;
  }

  std::array<int, 2> initialPos;
  std::array<int, 2> finishPos;

  if (startPos[axis] <= endPos[axis]) {
    for (int i = 0; i < 2; ++i) {
      initialPos[i] = startPos[i];
      finishPos[i] = endPos[i];
    }
  } else {
    for (int i = 0; i < 2; ++i) {
      initialPos[i] = endPos[i];
      finishPos[i] = startPos[i];
    }
  }

  if (axis == 0) {
    for (int x = initialPos[0]; x <= finishPos[0]; ++x) {
      int y = (int)(gradient * (x - initialPos[0]) + initialPos[1]);
      std::array<int, 2> point = {x, y};
      points.push_back(point);

      float depth;
      if (finishPos[0] == initialPos[0]) {
        depth = closerDepth;
      } else {
        depth = ((x - initialPos[0]) / (finishPos[0] - initialPos[0]) * std::abs(startDepth - endDepth) + closerDepth); // linear interpolation
      }
      depths.push_back(depth);
    }
  } else {
    for (int y = initialPos[1]; y <= finishPos[1]; ++y) {
      int x = (int)((y - initialPos[1]) / gradient + initialPos[0]);
      std::array<int, 2> point = {x, y};
      points.push_back(point);

      float depth;
      if (finishPos[0] == initialPos[0]) {
        depth = closerDepth;
      } else {
        depth = ((y - initialPos[1]) / (finishPos[1] - initialPos[1]) * std::abs(startDepth - endDepth) + closerDepth); // linear interpolation
      }
      depths.push_back(depth);
    }
  }

  return  { points, depths };
}
