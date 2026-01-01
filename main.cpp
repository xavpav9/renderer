#include <thread>
#include <chrono>

#include "renderer.hpp"

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

class Model {
public:
  std::vector<Triangle> faces;
  Model(std::vector<std::array<std::array<float,3>,3>> sides, std::vector<char> letters) {
    for (int i = 0; i < sides.size(); ++i) {
      Triangle trig(sides[i], letters[i]);
      faces.push_back(trig);
    }
  }

  void rotate(float yaw, float pitch, float roll) {
    for (int i = 0; i < faces.size(); ++i) {
      Triangle face = faces[i];
      face.rotate(yaw, pitch, roll);
      faces[i] = face;
    }
  }

  void translate(float x, float y, float z) {
    for (int i = 0; i < faces.size(); ++i) {
      Triangle face = faces[i];
      face.translate(x, y, z);
      faces[i] = face;
    }
  }

  void draw(Screen& screen, std::array<float,3> cameraPos, std::array<float,3> cameraRot, int focalLength) {
    for (int i = 0; i < faces.size(); ++i) {
      faces[i].draw(screen, cameraPos, cameraRot, focalLength);
    }
  }
};

int main() {
  int width = 50;
  int height = 50;
  Screen mainScreen = Screen(width, height);

  std::vector<Model> models;

  int focalLength = 100;
  std::array<float, 3> cameraPos = {0, -300, -10};
  std::array<float, 3> cameraRot = {0, 0, PI / 20};

  std::array<float, 3> point1;
  std::array<float, 3> point2;
  std::array<float, 3> point3;
  std::array<std::array<float, 3>, 3> face;

  // cube
  std::vector<std::array<std::array<float,3>,3>> faces;
  std::vector<char> letters;

  std::array<float,3> c1 = {20, 20, 20};
  std::array<float,3> c2 = {-20, 20, 20};
  std::array<float,3> c3 = {20, -20, 20};
  std::array<float,3> c4 = {-20, -20, 20};
  std::array<float,3> c5 = {20, 20, -20};
  std::array<float,3> c6 = {-20, 20, -20};
  std::array<float,3> c7 = {20, -20, -20};
  std::array<float,3> c8 = {-20, -20, -20};

  face = {c1, c2, c3};
  faces.push_back(face);
  face = {c2, c3, c4};
  faces.push_back(face);
  face = {c1, c2, c6};
  faces.push_back(face);
  face = {c1, c5, c6};
  faces.push_back(face);
  face = {c1, c3, c7};
  faces.push_back(face);
  face = {c1, c5, c7};
  faces.push_back(face);
  face = {c3, c4, c8};
  faces.push_back(face);
  face = {c3, c7, c8};
  faces.push_back(face);
  face = {c2, c4, c8};
  faces.push_back(face);
  face = {c2, c6, c8};
  faces.push_back(face);
  face = {c5, c7, c8};
  faces.push_back(face);
  face = {c5, c6, c8};
  faces.push_back(face);

  for (int i = 0; i < 2; ++i) letters.push_back('@');
  for (int i = 0; i < 2; ++i) letters.push_back('$');
  for (int i = 0; i < 2; ++i) letters.push_back('.');
  for (int i = 0; i < 2; ++i) letters.push_back('~');
  for (int i = 0; i < 2; ++i) letters.push_back('{');
  for (int i = 0; i < 2; ++i) letters.push_back('*');


  Model cube(faces, letters);

  models.push_back(cube);


  while (1) {
    mainScreen.emptyBuffer();
    mainScreen.emptyZBuffer();

     for (int i = 0; i < models.size(); ++i) {
      Model model = models[i];

      model.rotate(0.02, 0.02, 0.02);
      model.draw(mainScreen, cameraPos, cameraRot, focalLength);

      models[i] = model;
    }

    clearScreen();
    mainScreen.drawBuffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  return 0;
}
