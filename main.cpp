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

std::array<int, 2> get2dPos(std::array<float, 3> pos, std::array<float, 3> cameraPos, int focalLength) {
  if (cameraPos[1] == pos[1]) {
    cameraPos[1] += 1;
  }
  int newX = ((cameraPos[0] - pos[0]) * (focalLength) / (cameraPos[1] - pos[1]));
  int newY = ((cameraPos[2] - pos[2]) * (focalLength) / (cameraPos[1] - pos[1]));
  std::array<int, 2> newPos = {newX, newY};
  return newPos;
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
  point[1] = -point[1];
  point[0] += (int)(width / 2);
  point[1] += (int)(height / 2);
  if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
    buffer[width * point[1] + point[0]] = '@';
  }
}

void drawLine(char buffer[], int width, int height, std::array<float, 3> start, std::array<float, 3> end, std::array<float, 3> cameraPos, int focalLength) {
  std::array<int, 2> start2d = get2dPos(start, cameraPos, focalLength);
  std::array<int, 2> end2d = get2dPos(end, cameraPos, focalLength);
  std::vector<std::array<int,2>> points = getPoints(start2d, end2d);

  for (int i = 0; i < points.size(); ++i) {
    addPoint(buffer, width, height, points[i]);
  }
}

class Triangle {
public:
  std::array<std::array<float, 3>, 3> vertices;
  Triangle(std::array<std::array<float, 3>, 3> points) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        vertices[i][j] = (float)points[i][j];
      }
    }
  }

  void draw(char buffer[], int width, int height, std::array<float, 3> cameraPos, int focalLength) {
    std::array<int, 2> vertex1 = get2dPos(vertices[0], cameraPos, focalLength);
    std::array<int, 2> vertex2 = get2dPos(vertices[1], cameraPos, focalLength);
    std::array<int, 2> vertex3 = get2dPos(vertices[2], cameraPos, focalLength);

    int top = (int)std::min(vertex1[1], std::min(vertex2[1], vertex3[1]));
    int bottom = (int)std::max(vertex1[1], std::max(vertex2[1], vertex3[1]));

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
  int focalLength = 100;
  char buffer[width * height];
  emptyBuffer(buffer, width, height);

  std::array<float, 3> cameraPos = {0.0f, -80.0f, 0.0f};
  std::array<float, 3> point1 = {0, 0, 0};
  std::array<float, 3> point2 = {0, 0, 8};
  std::array<float, 3> point3 = {std::sqrt(32), -std::sqrt(32), 0};
  std::array<float, 3> point4 = {0, 0, 8};
  std::array<float, 3> point5 = {std::sqrt(32), -std::sqrt(32), 0};
  std::array<float, 3> point6 = {std::sqrt(32), -std::sqrt(32), 8};
  std::array<std::array<float, 3>, 3> points = {point1, point2, point3};
  std::array<std::array<float, 3>, 3> points2 = {point4, point5, point6};
  Triangle trig1(points);
  Triangle trig2(points2);

  Triangle trigs[2] = {trig1, trig2};

  float a = 0; // yaw
  float b = 0; // pitch
  float c = 0.1; // roll
  while (1) {
    emptyBuffer(buffer, width, height);
    clearScreen();

     for (int o = 0; o < sizeof(trigs) / sizeof(Triangle); ++o) {
      Triangle trig = trigs[o];
       for (int i = 0; i < 3; ++i) {
        float x = trig.vertices[i][0];
        float y = trig.vertices[i][1];
        float z = trig.vertices[i][2];

        trig.vertices[i][0] = x * (std::cos(a) * std::cos(b)) + y * (std::cos(a) * std::sin(b) * std::sin(c) - std::sin(a) * std::cos(c)) + z * (std::cos(a) * std::sin(b) * std::cos(c) + std::sin(a) * std::sin(c));
        trig.vertices[i][1] = x * (std::sin(a) * std::cos(b)) + y * (std::sin(a) * std::sin(b) * std::sin(c) + std::cos(a) * std::cos(c)) + z * (std::sin(a) * std::sin(b) * std::cos(c) - std::cos(a) * std::sin(c));
        trig.vertices[i][2] = x * (-std::sin(b)) + y * (std::cos(b) * std::sin(c)) + z * (std::cos(b) * std::cos(c));

      }
      trig.draw(buffer, width, height, cameraPos, focalLength);
      trigs[o] = trig;
    }

    drawBuffer(buffer, width, height);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
