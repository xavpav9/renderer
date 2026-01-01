#pragma once
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <array>
#include <tuple>

const float PI = 3.14159265358979323846;

void clearScreen();

std::array<int, 2> get2dPos(std::array<float, 3> pos, int focalLength);

std::tuple<std::vector<std::array<int,2>>, std::vector<float>> getPoints(std::array<int, 2> startPos, float startDepth, std::array<int, 2> endPos, float endDepth);

class Screen {
public:
  int width;
  int height;
  std::vector<char> buffer;
  std::vector<float> zBuffer;

  Screen(int w, int h);
  void emptyBuffer();
  void emptyZBuffer();
  void drawBuffer();
  void addPoint(std::array<int, 2> point, float ooz, char letter);
};

class Triangle {
public:
  std::array<std::array<float, 3>, 3> vertices;
  char letter;
  Triangle(std::array<std::array<float, 3>, 3> points, char fillLetter); 
  void draw(Screen& screen, std::array<float, 3> cameraPos, std::array<float, 3> cameraRot, int focalLength);
  void rotate(float yaw, float roll, float pitch);
  void translate(float x, float y, float z);
  std::array<std::array<float,3>, 3> rotateVertices(std::array<std::array<float,3>,3> oldVertices, float yaw, float pitch, float roll);
  std::array<std::array<float,3>,3> translateVertices(std::array<std::array<float,3>,3> oldVertices, float x, float y, float z);
};

