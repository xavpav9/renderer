#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <cmath>

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

void emptyBuffer(char buffer[], int width, int height) {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = ' ';
  }
}

std::vector<int> getPoints(int startPos[2], int endPos[2]) {
  std::vector<int> points;
  int axis;
  float gradient = (float)(startPos[1] - endPos[1]) / (float)(startPos[0] - endPos[0]);

  if (gradient < -1 || gradient > 1) {
    axis = 1;
  } else {
    axis = 0;
  }

  int initialPos[2];
  int finishPos[2];

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
      points.push_back(x);
      points.push_back(y);
    }
  } else {
    for (int y = initialPos[1]; y <= finishPos[1]; ++y) {
      int x = (int)((y - initialPos[1]) / gradient + initialPos[0]);
      points.push_back(x);
      points.push_back(y);
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

void addPoint(char buffer[], int width, int height, int point[]) {
  point[0] += int(width / 2);
  point[1] += int(height / 2);
  if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
    buffer[width * point[1] + point[0]] = '@';
  }
}

void drawLine(char buffer[], int width, int height, int start[], int end[]) {
  std::vector<int> points = getPoints(start, end);

  for (int i = 0; i < points.size(); i += 2) {
    int point[2] = {points[i], points[i+1]};
    addPoint(buffer, width, height, point);
  }
}

class Triangle {
public:
  float vertices[6];
  Triangle(int points[6]) {
    for (int i = 0; i < 6; ++i) {
      vertices[i] = (float)points[i];
    }
  }

  void draw(char buffer[], int width, int height) {
    int top = (int)std::min(vertices[1], std::min(vertices[3], vertices[5]));
    int bottom = (int)std::max(vertices[1], std::max(vertices[3], vertices[5]));

    int vertex1[2] = {(int)vertices[0], (int)vertices[1]};
    int vertex2[2] = {(int)vertices[2], (int)vertices[3]};
    int vertex3[2] = {(int)vertices[4], (int)vertices[5]};
    std::vector<int> points = getPoints(vertex1, vertex2);
    std::vector<int> points2 = getPoints(vertex1, vertex3);
    points.insert(points.end(), points2.begin(), points2.end());
    std::vector<int> points3 = getPoints(vertex3, vertex2);
    points.insert(points.end(), points3.begin(), points3.end());

    for (int y = top; y <= bottom; ++y) {
      int minX = std::numeric_limits<int>::max();
      int maxX = std::numeric_limits<int>::min();

      for (int i = 0; i < points.size(); i += 2) {
        if (points[i+1] == y) {
          if (points[i] < minX) {
            minX = points[i];
          }
          if (points[i] > maxX) {
            maxX = points[i];
          }
          points.erase(points.begin() + i);
          points.erase(points.begin() + i);
          i -= 2;
        }
      }
      for (int x = minX; x <= maxX; ++x) {
        int point[2] = {x, y};
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

  int start[2] = {0,0};
  int end[2] = {10,20};
  int points[6] = {-20,0,0,0,0,-20};
  int points2[6] = {0,20,0,0,20,0};
  Triangle trig1(points);
  Triangle trig2(points2);

  Triangle trigs[2] = {trig1, trig2};

  while (1) {
    emptyBuffer(buffer, width, height);
    clearScreen();

    for (int o = 0; o < sizeof(trigs) / sizeof(Triangle); ++o) {
      Triangle trig = trigs[o];
      for (int i = 0; i < 6; i += 2) {
        float oldX = trig.vertices[i];
        trig.vertices[i] = std::cos(0.1) * trig.vertices[i] - std::sin(0.1) * trig.vertices[i + 1];
        trig.vertices[i + 1] = std::sin(0.1) * oldX + std::cos(0.1) * trig.vertices[i + 1];
      }
      trig.draw(buffer, width, height);
      trigs[o] = trig;
    }

    drawBuffer(buffer, width, height);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
