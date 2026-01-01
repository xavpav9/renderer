#include <thread>
#include <chrono>

#include "renderer.hpp"

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

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
