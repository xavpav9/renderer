#include <thread>
#include <chrono>

#include "renderer.hpp"

void clearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

class Model {
public:
  std::vector<Triangle> faces;
  // initialise triangles
  Model(std::vector<std::array<std::array<float,3>,3>> sides, std::vector<char>& fillLetters) {
    for (int i = 0; i < sides.size(); ++i) {
      Triangle trig(sides[i], fillLetters);
      faces.push_back(trig);
    }
  }

  // rotate each triangle by the rotation angles ( in rad )
  void rotate(float yaw, float pitch, float roll) {
    for (int i = 0; i < faces.size(); ++i) {
      Triangle face = faces[i];
      face.rotate(yaw, pitch, roll);
      faces[i] = face;
    }
  }

  // translate each triangle by the translation vector
  void translate(float x, float y, float z) {
    for (int i = 0; i < faces.size(); ++i) {
      Triangle face = faces[i];
      face.translate(x, y, z);
      faces[i] = face;
    }
  }

  // draw each triangle to the screen buffer
  void draw(Screen& screen, std::array<float,3> cameraPos, std::array<float,3> cameraRot, int focalLength, std::vector<std::array<float,3>> lightSources) {
    for (int i = 0; i < faces.size(); ++i) {
      faces[i].draw(screen, cameraPos, cameraRot, focalLength, lightSources);
    }
  }
};

// /*
Model makeCube(std::array<float, 3> centre, float sideLength, std::vector<char> letters) {
  // create a cube
  std::array<std::array<float, 3>, 3> face;

  std::vector<std::array<std::array<float,3>,3>> faces;

  std::array<float,3> c1 = {sideLength + centre[0], sideLength + centre[1], sideLength + centre[2]};
  std::array<float,3> c2 = {-sideLength + centre[0], sideLength + centre[1], sideLength + centre[2]};
  std::array<float,3> c3 = {sideLength + centre[0], -sideLength + centre[1], sideLength + centre[2]};
  std::array<float,3> c4 = {-sideLength + centre[0], -sideLength + centre[1], sideLength + centre[2]};
  std::array<float,3> c5 = {sideLength + centre[0], sideLength + centre[1], -sideLength + centre[2]};
  std::array<float,3> c6 = {-sideLength + centre[0], sideLength + centre[1], -sideLength + centre[2]};
  std::array<float,3> c7 = {sideLength + centre[0], -sideLength + centre[1], -sideLength + centre[2]};
  std::array<float,3> c8 = {-sideLength + centre[0], -sideLength + centre[1], -sideLength + centre[2]};

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

  Model cube(faces, letters);

  return cube;
}

// */

int main() {
  int width = 20;
  int height = 20;
  Screen mainScreen = Screen(width, height);

  std::vector<Model> models;

  // set up light source and camera position
  int focalLength = 100;
  std::array<float, 3> cameraPos = {0, -400, 0};
  std::array<float, 3> cameraRot = {0, 0, 0};

  std::array<float, 3> lightSource = {0, -200, 0};
  std::vector<std::array<float,3>> lightSources = { lightSource };

  std::vector<char> letters = {'@', '%', '#', '*', '+', '=', '-', ':', '.'};
  // std::vector<char> letters = {'$', '@', 'B', '%', '8', '&', 'W', 'M', '#', '*', 'o', 'a', 'h', 'k', 'b', 'd', 'p', 'q', 'w', 'm', 'Z', 'O', '0', 'Q', 'L', 'C', 'J', 'U', 'Y', 'X', 'z', 'c', 'v', 'u', 'n', 'x', 'r', 'j', 'f', 't', '/', '\\', '|', '(', ')', '1', '{', '}', '[', ']', '?', '-', '_', '+', '~', '<', '>', 'i', '!', 'l', 'I', ';', ':', ',', '\"', '^', '`', '\'', '.'};

  float sideLength = 10;
  std::array<float, 3> centre = { 0, 0, -10 };
  Model cube = makeCube(centre, sideLength, letters);
  std::array<float, 3> centre2 = { 0, 0, 30 };
  Model cube2 = makeCube(centre2, sideLength, letters);

  models.push_back(cube);
  models.push_back(cube2);


  /*
  // square

  std::array<float, 3> point1;
  std::array<float, 3> point2;
  std::array<float, 3> point3;

  */

  while (1) {
    // render loop
    mainScreen.emptyBuffer();
    mainScreen.emptyZBuffer();

    for (int i = 0; i < models.size(); ++i) {
      Model model = models[i];

      model.rotate(0.01, 0, 0);
      model.draw(mainScreen, cameraPos, cameraRot, focalLength, lightSources);

      models[i] = model;
    }

    clearScreen();
    mainScreen.drawBuffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }

  return 0;
}
