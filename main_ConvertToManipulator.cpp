#include "Kinematics/Kinematics.h"

#include <iostream> // cout
#include <fstream> // input/output with file
#include <string> // STL string
#include <vector> // STL vector
#include <sstream> // isstringstream

using namespace std;

#define INPUT_TXT_FILENAME "results/RESULT.txt"
#define OUTPUT_TXT_FILENAME "results/MANIPULATOR_PATH.txt"
#define OUTPUT_POINTS_TXT_FILENAME "results/MANIPULATOR_PATH_POINTS.txt"

// ��� ���������� ����������� � �����������
#define DESK_DISTANCE_X 40 // ������ �� ������ ����� �� X. ����� ��������������� X
#define DESK_MIN_Y -15 // ����������� ��������� � ������� �����
#define DESK_MAX_Y 15
#define DESK_MIN_Z 10
#define DESK_MAX_Z 50
#define DESK_DISTANCE_X_TO_MARKER 8 // ���������� �� ������� ������� ������ �� �����, ����� �� �������� �� ���
#define MAX_POINTS_DIST 4 // ������������ ���������� ����� �������. ���� ��� ������, ������ ����� ������� �� ��������� �����

#define DESK_Y_SIZE (DESK_MAX_Y - DESK_MIN_Y)
#define DESK_Z_SIZE (DESK_MAX_Z - DESK_MIN_Z)

#define JOINTS_COUNT 6 // ���-�� ������ � ������������

#define JOINT_POS_MIN_DEG 0.0
#define JOINT_POS_MAX_DEG 360.0
const float minPoses[] = {
    JOINT_POS_MIN_DEG, // min 1
    73, // min 2
    83, // min 3
    78, // min 4
    3,  // min 5
    120, // min 6
};
const float maxPoses[] = {
    JOINT_POS_MAX_DEG, // max 1
    283, // max 2
    275, // max 3
    253, // max 4
    245, // max 5
    193, // max 6
};
const float defaultPoses[] = {135, 155, 132, 164, 180, 170};


vector<float> convertToDesk(float addDeskX, float x, float minX, float xSize, float y, float minY, float ySize) {
    float yDesk = float(x - minX) / xSize * DESK_Y_SIZE + DESK_MIN_Y; // ������������ ���������� ��� ������ �����
    float zDesk = float(y - minY) / ySize * DESK_Z_SIZE + DESK_MIN_Z;
    vector<float> res;
    res.push_back(DESK_DISTANCE_X + addDeskX);
    res.push_back(yDesk);
    res.push_back(zDesk);
    return res;
}


void outInFile(vector<vector<float>> list, const char* filename) {
  // ������� ��������� � ����
  ofstream out(filename);
  for (auto pos: list) {
    for (auto val: pos) {
      out << val << " ";
    }
    out << endl;
  }
  out.close();
}


int main() {
  // ��������� �������� �� �����
  vector<vector<pair<int, int>>> contours;

  ifstream in(INPUT_TXT_FILENAME);
  if (!in.is_open()) {
    cout << "File not opened" << endl;
    exit(-1);
  }

  int minX, minY, maxX, maxY;
  bool firstValueGotten;
  while (in.good()) {
    string line;
    getline(in, line);

    istringstream streamLine(line);

    vector<pair<int, int>> contour;
    int x, y;
    while (streamLine >> x >> y) {
      contour.push_back(pair<int, int>(x, y));

      if (!firstValueGotten) { // ������� ������� ������� � �������� �� X � Y
        minX = maxX = x;
        minY = maxY = y;
        firstValueGotten = true;
      } else {
        minX = min(x, minX);
        maxX = max(x, maxX);
        minY = min(y, minY);
        maxY = max(y, maxY);
      }
    }
    if (contour.size() >= 2) {
      contours.push_back(contour);
    }
  }
  in.close();
  // ������� �� �����

  cout << "Total contours:" << contours.size() << endl;

  cout << "X:" << minX << " - " << maxX << endl;
  cout << "Y:" << minY << " - " << maxY << endl;
  float xSize = maxX - minX;
  float ySize = maxY - minY;


  // ����������� �� �������� � ������� 3D ����� �� �����
  vector<vector<float>> manipulatorPoints;
  for (auto contour: contours) {
    manipulatorPoints.push_back(convertToDesk(
        DESK_DISTANCE_X_TO_MARKER,
        contour.front().first, minX, xSize,
        contour.front().second, minY, ySize
    )); // ��������� ����� "���" ������ � �������
    for (auto point: contour) {
      manipulatorPoints.push_back(convertToDesk(
          0,
          point.first, minX, xSize,
          point.second, minY, ySize
      )); // ��������� ����� �� �������
    }
    manipulatorPoints.push_back(convertToDesk(
        DESK_DISTANCE_X_TO_MARKER,
        contour.back().first, minX, xSize,
        contour.back().second, minY, ySize
    )); // ��������� ����� "���" ��������� � ��������� �����
  }

  // ����������� �� �������� � ��������� ����� ����� ����� �������� ����������
  vector<vector<float>> manipulatorPointsFilled;
  for (auto it = manipulatorPoints.begin(); it != (manipulatorPoints.end() - 1); it++) {
    // ����� ��� ����� � ���������
    vector<float> point = *it;
    vector<float> pointNext = *(it + 1);
    // ����� ���������� (�� X �� ���������, ������ ��� ��� ������ ����������/���������� ������)
//    const float dx = point[0] - pointNext[0];
    const float dy = pointNext[1] - point[1];
    const float dz = pointNext[2] - point[2];
    const float dist = sqrt(/*(dx * dx) +*/ (dy * dy) + (dz * dz));
    manipulatorPointsFilled.push_back(point);
    for (float d = MAX_POINTS_DIST; d < dist; d += MAX_POINTS_DIST) {
      manipulatorPointsFilled.push_back(vector<float>({
          point[0],
          point[1] + dy / dist * d,
          point[2] + dz / dist * d
      }));
    }
  }
  manipulatorPointsFilled.push_back(manipulatorPoints.back()); // �� �������� �������� ����� ��������� �����
  // ������� ����� � ����
  outInFile(manipulatorPointsFilled, OUTPUT_POINTS_TXT_FILENAME);


  // ����������� �� ������ � ���������� �� � ��������� ������������
  vector<vector<float>> manipulatorPath;
  for (auto point: manipulatorPointsFilled) {
    float tmpPoses[JOINTS_COUNT];
    getAnglesByTargetPoint(point[0], point[1], point[2], defaultPoses, tmpPoses, JOINTS_COUNT, minPoses, maxPoses); // �������� ��������� ������ ������������ ��� �����
    vector<float> res;
    for (size_t i = 0; i < JOINTS_COUNT; i++) { // �������� � ������
      res.push_back(tmpPoses[i]);
    }
    manipulatorPath.push_back(res); // ���������� ������ �������� � ����� � ������� ������
  }

  // ������� ��������� � ����
  outInFile(manipulatorPath, OUTPUT_TXT_FILENAME);
  return 0;
}
