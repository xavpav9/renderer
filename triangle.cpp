#include "renderer.hpp"

Triangle::Triangle(std::array<std::array<float, 3>, 3> points, std::vector<char>& fillLetters, std::string triangleColour) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      vertices[i][j] = (float)points[i][j];
    }
  }
  letters = fillLetters; // lightest first
  colour = triangleColour;
}

void Triangle::draw(Screen& screen, std::array<float, 3> cameraPos, std::array<float, 3> cameraRot, int focalLength, std::vector<std::array<float,4>> lightSources) {
  
  // Calculate new position due to camera position.
  std::array<std::array<float,3>,3> cameraAdjustedVertices = rotateVertices(translateVertices(vertices, -cameraPos[0], -cameraPos[1], -cameraPos[2]), -cameraRot[0], -cameraRot[1], -cameraRot[2]);
  std::array<int, 2> vertex1 = get2dPos(cameraAdjustedVertices[0], focalLength);
  std::array<int, 2> vertex2 = get2dPos(cameraAdjustedVertices[1], focalLength);
  std::array<int, 2> vertex3 = get2dPos(cameraAdjustedVertices[2], focalLength);

  if (std::max(cameraAdjustedVertices[0][1], std::max(cameraAdjustedVertices[1][1], cameraAdjustedVertices[2][1])) < 0) {
    return;
  }

  // Calculate highest and lowest y (negative y cartesian 2d) point of triangle.

  int top = std::round(std::min(vertex1[1], std::min(vertex2[1], vertex3[1])));
  int bottom = std::round(std::max(vertex1[1], std::max(vertex2[1], vertex3[1])));

  if (top > screen.height / 2) return;
  if (bottom < -screen.height / 2) return;
  if (top < -screen.height / 2) top = -screen.height / 2;
  if (bottom > screen.height / 2) bottom = screen.height / 2;


  // Calculate points and depths using getPoints().

  auto values = getPoints(screen, vertex1, cameraAdjustedVertices[0][1], vertex2, cameraAdjustedVertices[1][1]);
  std::vector<std::array<int,2>> points = std::get<0>(values);
  std::vector<float> depths = std::get<1>(values);

  auto values2 = getPoints(screen, vertex1, cameraAdjustedVertices[0][1], vertex3, cameraAdjustedVertices[2][1]);
  std::vector<std::array<int,2>> points2 = std::get<0>(values2);
  std::vector<float> depths2 = std::get<1>(values2);
  points.insert(points.end(), points2.begin(), points2.end());
  depths.insert(depths.end(), depths2.begin(), depths2.end());

  auto values3 = getPoints(screen, vertex3, cameraAdjustedVertices[2][1], vertex2, cameraAdjustedVertices[1][1]);
  std::vector<std::array<int,2>> points3 = std::get<0>(values3);
  std::vector<float> depths3 = std::get<1>(values3);
  points.insert(points.end(), points3.begin(), points3.end());
  depths.insert(depths.end(), depths3.begin(), depths3.end());

  // Calculate Light Strength

  std::array<float, 3> vectorA = cameraAdjustedVertices[0];
  std::array<float, 3> vectorB = cameraAdjustedVertices[1];
  std::array<float, 3> vectorC = cameraAdjustedVertices[2];
  
  std::array<float, 3> vectorAB;
  std::array<float, 3> vectorBC;

  // -> calculate 2 vectors from sides
  for (int i = 0; i < 3; ++i) {
    vectorAB[i] = vectorA[i] - vectorB[i];
    vectorBC[i] = vectorC[i] - vectorB[i];
  }

  // -> find the cross product
  std::array<float, 3> perpendicularVector;
  for (int i = 0; i < 3; ++i) {
    perpendicularVector[i] = vectorAB[(i+1) % 3] * vectorBC[(i+2) % 3] - vectorAB[(i+2) % 3] * vectorBC[(i+1) % 3];
  }

  if (perpendicularVector[1] >= 0) {
    for (int i = 0; i < 3; ++i) {
      perpendicularVector[i] = -perpendicularVector[i];
    }
  }

  // -> dot product in order to get angle
  float lightStrength = (float)letters.size() - 1;
  for (auto lightSource : lightSources) {
    std::array<float, 3> lightPos;
    for (int i = 0; i < 3; ++i) {
      lightPos[i] = lightSource[i];
    }
    std::array<float, 3> cameraAdjustedLightSource = rotateVertex(translateVertex(lightPos, -cameraPos[0], -cameraPos[1], -cameraPos[2]), -cameraRot[0], -cameraRot[1], -cameraRot[2]);

    // -> get vector between light and one of the triangle vertices
    std::array<float, 3> lightVector;
    for (int i = 0; i < 3; ++i) {
      lightVector[i] = cameraAdjustedLightSource[i] - vectorB[i];
    }

    float dotProduct = 0;
    for (int i = 0; i < 3; ++i) {
      dotProduct += (perpendicularVector[i] * lightVector[i]);
    }

    float perpendicularVectorMagnitude = 0;
    float lightSourceMagnitude = 0;
    for (int i = 0; i < 3; ++i) {
      perpendicularVectorMagnitude += std::pow(perpendicularVector[i],2);
      lightSourceMagnitude += std::pow(lightVector[i],2);
    }
    perpendicularVectorMagnitude = std::sqrt(perpendicularVectorMagnitude);
    lightSourceMagnitude = std::sqrt(lightSourceMagnitude);

    // calculate the lightstrength using basic formula I made up
    float angle = std::acos((dotProduct) / (perpendicularVectorMagnitude * lightSourceMagnitude));
      float change = 1;

      /*
      The smaller the change, the stronger the final light
      lightSource[3] is the strength of the light
      lightSourceMagnitude is also the distance from B to the lightSource
      */

      if (lightSource[3] != 0) change = (angle / (PI / 2)) / lightSource[3] * lightSourceMagnitude / 200;

      if (change < 1) lightStrength *= change;
  }

  int lightPowerIndex = std::round(lightStrength);
  char letter = letters[lightPowerIndex];

  // fill in the shape with horizontal lines
  float totalDepth = 0;
  for (float depth : depths) totalDepth += depth;
  float averageDepth = totalDepth / depths.size();
  float averageOoz = 1 / averageDepth; // used to determine which point to show when ooz is the same

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

    if (maxX < -screen.width / 2) continue;
    if (minX > screen.width / 2) continue;
    if (maxX > screen.width / 2) maxX = screen.width / 2;
    if (minX < -screen.width / 2) minX = -screen.width / 2;

    for (int x = minX; x <= maxX; ++x) {
      std::array<int, 2> point = {x, y};

      // get the depth of the point
      float depth;
      if (minX == maxX) {
        depth = minDepth;
      } else {
        depth = (float)(x - minX) / (float)(maxX - minX) * (maxDepth - minDepth) + minDepth; // linear interpolation
      }

      float ooz = 1 / depth;
      screen.addPoint(point, ooz, letter, colour, averageOoz);
    }
  }
}

void Triangle::rotate(float yaw, float pitch, float roll) {
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
    newVertices[i] = translateVertex(oldVertices[i], x, y, z);
  }
  return newVertices;
}

std::array<std::array<float,3>, 3> Triangle::rotateVertices(std::array<std::array<float,3>,3> oldVertices, float yaw, float pitch, float roll) {
  std::array<std::array<float,3>, 3> newVertices;
  for (int i = 0; i < 3; ++i) {
    newVertices[i] = rotateVertex(oldVertices[i], yaw, pitch, roll);
  }

  return newVertices;
}

std::array<float,3> Triangle::translateVertex(std::array<float,3> oldVertex, float x, float y, float z) {
  std::array<float,3> newVertex;
  newVertex[0] = oldVertex[0] + x;
  newVertex[1] = oldVertex[1] + y;
  newVertex[2] = oldVertex[2] + z;
  return newVertex;
}

std::array<float,3> Triangle::rotateVertex(std::array<float,3> oldVertex, float yaw, float pitch, float roll) {
  std::array<float,3> newVertex;
  float x = oldVertex[0];
  float y = oldVertex[1];
  float z = oldVertex[2];

  newVertex[0] = x * (std::cos(yaw) * std::cos(pitch)) + y * (std::cos(yaw) * std::sin(pitch) * std::sin(roll) - std::sin(yaw) * std::cos(roll)) + z * (std::cos(yaw) * std::sin(pitch) * std::cos(roll) + std::sin(yaw) * std::sin(roll));
  newVertex[1] = x * (std::sin(yaw) * std::cos(pitch)) + y * (std::sin(yaw) * std::sin(pitch) * std::sin(roll) + std::cos(yaw) * std::cos(roll)) + z * (std::sin(yaw) * std::sin(pitch) * std::cos(roll) - std::cos(yaw) * std::sin(roll));
  newVertex[2] = x * (-std::sin(pitch)) + y * (std::cos(pitch) * std::sin(roll)) + z * (std::cos(pitch) * std::cos(roll));

  return newVertex;
}
