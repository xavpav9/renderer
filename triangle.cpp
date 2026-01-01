#include "renderer.hpp"

Triangle::Triangle(std::array<std::array<float, 3>, 3> points, char fillLetter) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      vertices[i][j] = (float)points[i][j];
      letter = fillLetter;
    }
  }
}

void Triangle::draw(Screen& screen, std::array<float, 3> cameraPos, std::array<float, 3> cameraRot, int focalLength) {
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

void Triangle::rotate(float yaw, float roll, float pitch) {
  for (int i = 0; i < 3; ++i) {
    float x = vertices[i][0];
    float y = vertices[i][1];
    float z = vertices[i][2];

    vertices[i][0] = x * (std::cos(yaw) * std::cos(pitch)) + y * (std::cos(yaw) * std::sin(pitch) * std::sin(roll) - std::sin(yaw) * std::cos(roll)) + z * (std::cos(yaw) * std::sin(pitch) * std::cos(roll) + std::sin(yaw) * std::sin(roll));
    vertices[i][1] = x * (std::sin(yaw) * std::cos(pitch)) + y * (std::sin(yaw) * std::sin(pitch) * std::sin(roll) + std::cos(yaw) * std::cos(roll)) + z * (std::sin(yaw) * std::sin(pitch) * std::cos(roll) - std::cos(yaw) * std::sin(roll));
    vertices[i][2] = x * (-std::sin(pitch)) + y * (std::cos(pitch) * std::sin(roll)) + z * (std::cos(pitch) * std::cos(roll));
  }
}

void Triangle::translate(float x, float y, float z) {
  for (int i = 0; i < 3; ++i) {
    vertices[i][0] += x;
    vertices[i][1] += y;
    vertices[i][2] += z;
  }
}

std::array<std::array<float,3>,3> Triangle::translateVertices(std::array<std::array<float,3>,3> oldVertices, float x, float y, float z) {
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

std::array<std::array<float,3>, 3> Triangle::rotateVertices(std::array<std::array<float,3>,3> oldVertices, float yaw, float pitch, float roll) {
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
