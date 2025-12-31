#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <cmath>
#include <array>

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

void emptyBuffer(char buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = ' ';
  }
}

std::vector<std::array<int,2>> getPoints(std::array<int, 2> startPos, std::array<int, 2> endPos) {
  std::vector<std::array<int,2>> points;
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
    }
  } else {
    for (int y = initialPos[1]; y <= finishPos[1]; ++y) {
      int x = (int)((y - initialPos[1]) / gradient + initialPos[0]);
      std::array<int, 2> point = {x, y};
      points.push_back(point);
    }
  }

  return points;
}

void drawBuffer(char buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    std::cout << buffer[i];
    if ((i + 1) % width == 0) {
      std::cout << std::endl;
    }
  }
}

void addPoint(char buffer[], int width, int height, std::array<int, 2> point) {
  point[0] += (int)(width / 2);
  point[1] += (int)(height / 2);
  if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
    buffer[width * point[1] + point[0]] = '@';
  }
}

void drawLine(char buffer[], int width, int height, std::array<int, 2> start, std::array<int, 2> end) {
  std::vector<std::array<int,2>> points = getPoints(start, end);

  for (int i = 0; i < points.size(); ++i) {
    addPoint(buffer, width, height, points[i]);
  }
}

class Triangle {
public:
  std::array<std::array<float, 2>, 3> vertices;
  Triangle(std::array<std::array<int, 2>, 3> points) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 2; ++j) {
        vertices[i][j] = (float)points[i][j];
      }
    }
  }

  void draw(char buffer[], int width, int height) {
    int top = (int)std::min(vertices[0][1], std::min(vertices[1][1], vertices[2][1]));
    int bottom = (int)std::max(vertices[0][1], std::max(vertices[1][1], vertices[2][1]));


    std::array<int, 2> vertex1 = {(int)vertices[0][0], (int)vertices[0][1]};
    std::array<int, 2> vertex2 = {(int)vertices[1][0], (int)vertices[1][1]};
    std::array<int, 2> vertex3 = {(int)vertices[2][0], (int)vertices[2][1]};
    std::vector<std::array<int,2>> points = getPoints(vertex1, vertex2);
    std::vector<std::array<int,2>> points2 = getPoints(vertex1, vertex3);
    points.insert(points.end(), points2.begin(), points2.end());
    std::vector<std::array<int,2>> points3 = getPoints(vertex3, vertex2);
    points.insert(points.end(), points3.begin(), points3.end());

    for (int y = top; y <= bottom; ++y) {
      int minX = std::numeric_limits<int>::max();
      int maxX = std::numeric_limits<int>::min();

      for (int i = 0; i < points.size(); ++i) {
        if (points[i][1] == y) {
          if (points[i][0] < minX) {
            minX = points[i][0];
          }
          if (points[i][0] > maxX) {
            maxX = points[i][0];
          }
          points.erase(points.begin() + i);
          i--;
        }
      }

      for (int x = minX; x <= maxX; ++x) {
        std::array<int, 2> point = {x, y};
        addPoint(buffer, width, height, point);
      }
    }
  }
};


int main() {
  int width = 40;
  int height = 40;
  char buffer[width * height];
  emptyBuffer(buffer, width, height);

  std::array<int, 2> point1 = {-20, 0};
  std::array<int, 2> point2 = {0, 20};
  std::array<int, 2> point3 = {0, -20};
  std::array<int, 2> point4 = {20, 0};
  std::array<int, 2> point5 = {0, 20};
  std::array<int, 2> point6 = {0, -20};
  std::array<std::array<int, 2>, 3> points = {point1, point2, point3};
  std::array<std::array<int, 2>, 3> points2 = {point4, point5, point6};
  Triangle trig1(points);
  Triangle trig2(points2);

  Triangle trigs[2] = {trig1, trig2};

  while (1) {
    emptyBuffer(buffer, width, height);
    clearScreen();

    for (int o = 0; o < sizeof(trigs) / sizeof(Triangle); ++o) {
      Triangle trig = trigs[o];
      for (int i = 0; i < 3; ++i) {
        float oldX = trig.vertices[i][0];
        trig.vertices[i][0] = std::cos(0.1) * trig.vertices[i][0] - std::sin(0.1) * trig.vertices[i][1];
        trig.vertices[i][1] = std::sin(0.1) * oldX + std::cos(0.1) * trig.vertices[i][1];
      }
      trig.draw(buffer, width, height);
      trigs[o] = trig;
    }

    drawBuffer(buffer, width, height);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
