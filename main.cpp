#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <cmath>
#include <array>
#include <tuple>

const float PI = 3.14159265358979323846;

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

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

class Screen {
public:
  int width;
  int height;
  std::vector<char> buffer;
  std::vector<float> zBuffer;

  Screen(int w, int h) {
    width = w;
    height = h;
    for (int i = 0; i < w * h; ++i) {
      buffer.push_back(' ');
      zBuffer.push_back(0);
    }
  }

  void emptyBuffer() {
    for (int i = 0; i < width * height; ++i) {
      buffer[i] = ' ';
    }
  }

  void emptyZBuffer() {
    for (int i = 0; i < width * height; ++i) {
      zBuffer[i] = 0;
    }
  }

  void drawBuffer() {
    for (int i = 0; i < width * height; ++i) {
      std::cout << buffer[i];
      if ((i + 1) % width == 0) {
        std::cout << std::endl;
      }
    }
  }

  void addPoint(std::array<int, 2> point, float ooz /* one over z - for z-buffer */, char letter) {
    point[1] = -point[1]; // flipped y coord
    point[0] += (int)(width / 2);
    point[1] += (int)(height / 2);
    if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
      if (ooz > zBuffer[width * point[1] + point[0]]) {
        buffer[width * point[1] + point[0]] = letter;
        zBuffer[width * point[1] + point[0]] = ooz;
      }
    }
  }
};

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

  void draw(Screen& screen, std::array<float, 3> cameraPos, std::array<float, 3> cameraRot, int focalLength) {
    std::array<std::array<float,3>,3> cameraAdjustedVertices = translateVertices(rotateVertices(vertices, cameraRot[0], cameraRot[1], cameraRot[2]), -cameraPos[0], -cameraPos[1], -cameraPos[2]);
    std::array<int, 2> vertex1 = get2dPos(cameraAdjustedVertices[0], focalLength);
    std::array<int, 2> vertex2 = get2dPos(cameraAdjustedVertices[1], focalLength);
    std::array<int, 2> vertex3 = get2dPos(cameraAdjustedVertices[2], focalLength);

    int top = (int)std::min(vertex1[1], std::min(vertex2[1], vertex3[1]));
    int bottom = (int)std::max(vertex1[1], std::max(vertex2[1], vertex3[1]));

    auto values = getPoints(vertex1, cameraAdjustedVertices[0][1], vertex2, cameraAdjustedVertices[1][1]);
    std::vector<std::array<int,2>> points = std::get<0>(values);
    std::vector<float> depths = std::get<1>(values);

    auto values2 = getPoints(vertex1, cameraAdjustedVertices[0][1], vertex3, cameraAdjustedVertices[2][1]);
    std::vector<std::array<int,2>> points2 = std::get<0>(values2);
    std::vector<float> depths2 = std::get<1>(values2);
    points.insert(points.end(), points2.begin(), points2.end());
    depths.insert(depths.end(), depths2.begin(), depths2.end());

    auto values3 = getPoints(vertex3, cameraAdjustedVertices[2][1], vertex2, cameraAdjustedVertices[1][1]);
    std::vector<std::array<int,2>> points3 = std::get<0>(values3);
    std::vector<float> depths3 = std::get<1>(values3);
    points.insert(points.end(), points3.begin(), points3.end());
    depths.insert(depths.end(), depths3.begin(), depths3.end());

    for (int y = top; y <= bottom; ++y) { // y referring to up/down (2d)
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
        float ooz = 1 / depth;
        screen.addPoint(point, ooz, letter);
      }
    }
  }

  void rotate(float yaw, float roll, float pitch) {
    for (int i = 0; i < 3; ++i) {
      float x = vertices[i][0];
      float y = vertices[i][1];
      float z = vertices[i][2];

      vertices[i][0] = x * (std::cos(yaw) * std::cos(pitch)) + y * (std::cos(yaw) * std::sin(pitch) * std::sin(roll) - std::sin(yaw) * std::cos(roll)) + z * (std::cos(yaw) * std::sin(pitch) * std::cos(roll) + std::sin(yaw) * std::sin(roll));
      vertices[i][1] = x * (std::sin(yaw) * std::cos(pitch)) + y * (std::sin(yaw) * std::sin(pitch) * std::sin(roll) + std::cos(yaw) * std::cos(roll)) + z * (std::sin(yaw) * std::sin(pitch) * std::cos(roll) - std::cos(yaw) * std::sin(roll));
      vertices[i][2] = x * (-std::sin(pitch)) + y * (std::cos(pitch) * std::sin(roll)) + z * (std::cos(pitch) * std::cos(roll));
    }
  }

  void translate(float x, float y, float z) {
    for (int i = 0; i < 3; ++i) {
      vertices[i][0] += x;
      vertices[i][1] += y;
      vertices[i][2] += z;
    }
  }

  std::array<std::array<float,3>,3> translateVertices(std::array<std::array<float,3>,3> oldVertices, float x, float y, float z) {
    std::array<std::array<float,3>, 3> newVertices;
    for (int i = 0; i < 3; ++i) {
      std::array<float,3> newVertex;
      newVertex[0] = oldVertices[i][0] + x;
      newVertex[1] = oldVertices[i][1] + y;
      newVertex[2] = oldVertices[i][2] + z;
      newVertices[i] = newVertex;
    }
    return newVertices;
  }

  std::array<std::array<float,3>, 3> rotateVertices(std::array<std::array<float,3>,3> oldVertices, float yaw, float pitch, float roll) {
    std::array<std::array<float,3>, 3> newVertices;
    for (int i = 0; i < 3; ++i) {
      std::array<float,3> newVertex;
      float x = oldVertices[i][0];
      float y = oldVertices[i][1];
      float z = oldVertices[i][2];

      newVertex[0] = x * (std::cos(yaw) * std::cos(pitch)) + y * (std::cos(yaw) * std::sin(pitch) * std::sin(roll) - std::sin(yaw) * std::cos(roll)) + z * (std::cos(yaw) * std::sin(pitch) * std::cos(roll) + std::sin(yaw) * std::sin(roll));
      newVertex[1] = x * (std::sin(yaw) * std::cos(pitch)) + y * (std::sin(yaw) * std::sin(pitch) * std::sin(roll) + std::cos(yaw) * std::cos(roll)) + z * (std::sin(yaw) * std::sin(pitch) * std::cos(roll) - std::cos(yaw) * std::sin(roll));
      newVertex[2] = x * (-std::sin(pitch)) + y * (std::cos(pitch) * std::sin(roll)) + z * (std::cos(pitch) * std::cos(roll));
      
      newVertices[i] = newVertex;
    }

    return newVertices;
  }
};


int main() {
  int width = 60;
  int height = 40;

  Screen mainScreen = Screen(width, height);

  int focalLength = 100;

  std::array<float, 3> cameraPos = {0, -150, 0};
  std::array<float, 3> cameraRot = {0, 0, PI};
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

  while (1) {
    mainScreen.emptyBuffer();
    mainScreen.emptyZBuffer();

     for (int i = 0; i < 3; ++i) {
      Triangle trig = trigs[i];
      if (i != 2) trig.rotate(0.1, 0, 0);
      if (i == 2) trig.translate(0, -0.4, 0);

      trig.draw(mainScreen, cameraPos, cameraRot, focalLength);
      trigs[i] = trig;
    }

    clearScreen();
    mainScreen.drawBuffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
