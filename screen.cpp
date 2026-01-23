#include "renderer.hpp"

// initialise buffer and zbuffer
Screen::Screen(int w, int h) {
  width = w;
  height = h;
  for (int i = 0; i < w * h; ++i) {
    buffer.push_back(" ");
    zBuffer.push_back(0);
  }
}

// function to empty buffer width spaces (background char)
void Screen::emptyBuffer() {
  for (int i = 0; i < width * height; ++i) {
    buffer[i] = " ";
  }
}

// function to empty zbuffer (0 will be replaced by any positive ooz)
void Screen::emptyZBuffer() {
  for (int i = 0; i < width * height; ++i) {
    zBuffer[i] = 0;
  }
}

// draws the buffer to the screen
void Screen::drawBuffer() {
  for (int i = 0; i < width * height; ++i) {
    std::cout << buffer[i];
    if ((i + 1) % width == 0) {
      std::cout << std::endl;
    }
  }
}

bool Screen::isInScreen(std::array<int, 2> vertex) {
  if (std::abs(vertex[0]) > width / 2 || std::abs(vertex[1]) > height / 2) return false;
  else return true;
}

// Adds a point to the buffer depending on its ooz
void Screen::addPoint(std::array<int, 2> point, float ooz /* one over z - for z-buffer */, char letter, std::string colour, float averageOoz) {
  point[1] = -point[1]; // flipped y coord
  // moves the origin to the centre of the screen
  point[0] += (int)(width / 2);
  point[1] += (int)(height / 2);
  if (point[0] < width && point[0] >= 0 && point[1] >= 0 && point[1] < height) {
    float currentOoz = zBuffer[width * point[1] + point[0]];
    if (ooz > currentOoz) {
      buffer[width * point[1] + point[0]] = colour + letter + "\033[0m";
      zBuffer[width * point[1] + point[0]] = ooz;
    } else if (ooz == currentOoz) {
      if (averageOoz > currentOoz) {
        buffer[width * point[1] + point[0]] = colour + letter + "\033[0m";
        zBuffer[width * point[1] + point[0]] = ooz;
      }
    }
  }
}
