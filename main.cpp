#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <cmath>
#include <array>
#include <tuple>

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

void emptyBuffer(char buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = ' ';
  }
}

void emptyBuffer(float buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = 0;
  }
}

std::array<int, 2> get2dPos(std::array<float, 3> pos, std::array<float, 3> cameraPos, int focalLength) {
  if (cameraPos[1] == pos[1]) {
    cameraPos[1] += 1;
  }
  int newX = ((cameraPos[0] - pos[0]) * (focalLength) / (cameraPos[1] - pos[1]));
  int newY = ((cameraPos[2] - pos[2]) * (focalLength) / (cameraPos[1] - pos[1]));
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

void drawBuffer(char buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    std::cout << buffer[i];
    if ((i + 1) % width == 0) {
      std::cout << std::endl;
    }
  }
}

void addPoint(char buffer[], float zBuffer[], int width, int height, std::array<int, 2> point, float ooz /* one over z - for z-buffer */, char letter) {
  point[1] = -point[1];
  point[0] += (int)(width / 2);
  point[1] += (int)(height / 2);
  if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
    if (ooz > zBuffer[width * point[1] + point[0]]) {
      buffer[width * point[1] + point[0]] = letter;
      zBuffer[width * point[1] + point[0]] = ooz;
    }
  }
}

void drawLine(char buffer[], float zBuffer[], int width, int height, std::array<float, 3> start, std::array<float, 3> end, std::array<float, 3> cameraPos, int focalLength) {
  std::array<int, 2> start2d = get2dPos(start, cameraPos, focalLength);
  std::array<int, 2> end2d = get2dPos(end, cameraPos, focalLength);
  auto values = getPoints(start2d, start[1], end2d, end[1]);
  std::vector<std::array<int,2>> points = std::get<0>(values);

  for (int i = 0; i < points.size(); ++i) {
    addPoint(buffer, zBuffer, width, height, points[i], 100, '$');
  }
}

class Triangle {
public:
  std::array<std::array<float, 3>, 3> vertices;
  char letter;
  Triangle(std::array<std::array<float, 3>, 3> points, char fillLetter) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        vertices[i][j] = (float)points[i][j];
        letter = fillLetter;
      }
    }
  }

  void draw(char buffer[], float zBuffer[], int width, int height, std::array<float, 3> cameraPos, int focalLength) {
    std::array<int, 2> vertex1 = get2dPos(vertices[0], cameraPos, focalLength);
    std::array<int, 2> vertex2 = get2dPos(vertices[1], cameraPos, focalLength);
    std::array<int, 2> vertex3 = get2dPos(vertices[2], cameraPos, focalLength);

    int top = (int)std::min(vertex1[1], std::min(vertex2[1], vertex3[1]));
    int bottom = (int)std::max(vertex1[1], std::max(vertex2[1], vertex3[1]));

    auto values = getPoints(vertex1, vertices[0][1], vertex2, vertices[1][1]);
    std::vector<std::array<int,2>> points = std::get<0>(values);
    std::vector<float> depths = std::get<1>(values);

    auto values2 = getPoints(vertex1, vertices[0][1], vertex3, vertices[2][1]);
    std::vector<std::array<int,2>> points2 = std::get<0>(values2);
    std::vector<float> depths2 = std::get<1>(values2);
    points.insert(points.end(), points2.begin(), points2.end());
    depths.insert(depths.end(), depths2.begin(), depths2.end());

    auto values3 = getPoints(vertex3, vertices[2][1], vertex2, vertices[1][1]);
    std::vector<std::array<int,2>> points3 = std::get<0>(values3);
    std::vector<float> depths3 = std::get<1>(values3);
    points.insert(points.end(), points3.begin(), points3.end());
    depths.insert(depths.end(), depths3.begin(), depths3.end());

    for (int y = top; y <= bottom; ++y) {
      int minX = std::numeric_limits<int>::max();
      int maxX = std::numeric_limits<int>::min();
      float minDepth = 0;
      float maxDepth = 0;

      for (int i = 0; i < points.size(); ++i) {
        if (points[i][1] == y) {
          if (points[i][0] < minX) {
            minX = points[i][0];
            minDepth = depths[i];
          }
          if (points[i][0] > maxX) {
            maxX = points[i][0];
            maxDepth = depths[i];
          }
          points.erase(points.begin() + i);
          depths.erase(depths.begin() + i);
          i--;
        }
      }

      for (int x = minX; x <= maxX; ++x) {
        std::array<int, 2> point = {x, y};
        float depth;
        if (minX == maxX) {
          depth = minDepth;
        } else {
          depth = ((x - minX) / (maxX - minX) * (minDepth - maxDepth) + minDepth); // linear interpolation
        }
        float ooz = 1 / std::abs(depth);
        addPoint(buffer, zBuffer, width, height, point, ooz, letter);
      }
    }
  }
};


int main() {
  int width = 40;
  int height = 40;
  int focalLength = 100;
  char buffer[width * height];
  float zBuffer[width * height];
  emptyBuffer(buffer, width, height);
  emptyBuffer(zBuffer, width, height);

  std::array<float, 3> cameraPos = {0.0f, -80.0f, 0.0f};
  std::array<float, 3> point1 = {0, 0, 0};
  std::array<float, 3> point2 = {0, 0, 8};
  std::array<float, 3> point3 = {std::sqrt(32), -std::sqrt(32), 0};
  std::array<float, 3> point4 = {0, 0, 8};
  std::array<float, 3> point5 = {std::sqrt(32), -std::sqrt(32), 0};
  std::array<float, 3> point6 = {std::sqrt(32), -std::sqrt(32), 8};
  std::array<float, 3> point7 = {-20, 40, 20};
  std::array<float, 3> point8 = {-20, 40, -20};
  std::array<float, 3> point9 = {20, 40, 20};
  std::array<std::array<float, 3>, 3> points = {point1, point2, point3};
  std::array<std::array<float, 3>, 3> points2 = {point4, point5, point6};
  std::array<std::array<float, 3>, 3> points3 = {point7, point8, point9};
  Triangle trig1(points, '@');
  Triangle trig2(points2, '@');
  Triangle trig3(points3, '.');

  Triangle trigs[3] = {trig1, trig2, trig3};

  float a = 0; // yaw
  float b = 0; // pitch
  float c = 0.1; // roll
  while (1) {
    emptyBuffer(buffer, width, height);
    emptyBuffer(zBuffer, width, height);
    clearScreen();

     for (int o = 0; o < sizeof(trigs) / sizeof(Triangle); ++o) {
      Triangle trig = trigs[o];
       for (int i = 0; i < 3; ++i) {
         if (o != 2) {
          float x = trig.vertices[i][0];
          float y = trig.vertices[i][1];
          float z = trig.vertices[i][2];

          trig.vertices[i][0] = x * (std::cos(a) * std::cos(b)) + y * (std::cos(a) * std::sin(b) * std::sin(c) - std::sin(a) * std::cos(c)) + z * (std::cos(a) * std::sin(b) * std::cos(c) + std::sin(a) * std::sin(c));
          trig.vertices[i][1] = x * (std::sin(a) * std::cos(b)) + y * (std::sin(a) * std::sin(b) * std::sin(c) + std::cos(a) * std::cos(c)) + z * (std::sin(a) * std::sin(b) * std::cos(c) - std::cos(a) * std::sin(c));
          trig.vertices[i][2] = x * (-std::sin(b)) + y * (std::cos(b) * std::sin(c)) + z * (std::cos(b) * std::cos(c));
         }
      }
      trig.draw(buffer, zBuffer, width, height, cameraPos, focalLength);
      trigs[o] = trig;
    }

    drawBuffer(buffer, width, height);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
