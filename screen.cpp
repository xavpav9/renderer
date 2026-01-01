#include "renderer.hpp"

Screen::Screen(int w, int h) {
  width = w;
  height = h;
  for (int i = 0; i < w * h; ++i) {
    buffer.push_back(' ');
    zBuffer.push_back(0);
  }
}

void Screen::emptyBuffer() {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = ' ';
  }
}

void Screen::emptyZBuffer() {
  for (int i = 0; i < width * height; ++i) {
    zBuffer[i] = 0;
  }
}

void Screen::drawBuffer() {
  for (int i = 0; i < width * height; ++i) {
    std::cout << buffer[i];
    if ((i + 1) % width == 0) {
      std::cout << std::endl;
    }
  }
}

void Screen::addPoint(std::array<int, 2> point, float ooz /* one over z - for z-buffer */, char letter) {
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
