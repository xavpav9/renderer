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
  std::vector<std::string> buffer;
  std::vector<float> zBuffer;

  Screen(int w, int h);
  void emptyBuffer();
  void emptyZBuffer();
  void drawBuffer();
  bool isInScreen(std::array<int, 2> vertex);
  void addPoint(std::array<int, 2> point, float ooz, char letter, std::string colour);
};

class Triangle {
public:
  std::array<std::array<float, 3>, 3> vertices;
  std::vector<char> letters; // lightest first
  std::string colour;
  Triangle(std::array<std::array<float, 3>, 3> points, std::vector<char>& fillLetters, std::string triangleColour); 
  void draw(Screen& screen, std::array<float, 3> cameraPos, std::array<float, 3> cameraRot, int focalLength, std::vector<std::array<float,4>> lightSources);
  void rotate(float yaw, float roll, float pitch);
  void translate(float x, float y, float z);

  std::array<std::array<float,3>, 3> rotateVertices(std::array<std::array<float,3>,3> oldVertices, float yaw, float pitch, float roll);
  std::array<std::array<float,3>,3> translateVertices(std::array<std::array<float,3>,3> oldVertices, float x, float y, float z);

  std::array<float,3> rotateVertex(std::array<float,3> oldVertex, float yaw, float pitch, float roll);
  std::array<float,3> translateVertex(std::array<float,3> oldVertex, float x, float y, float z);
};

