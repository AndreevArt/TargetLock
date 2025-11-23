#include "pm.h"
#include <iostream>
#include <algorithm>

using namespace cv;
using namespace std;

vector<Point2f> PMWeapon::detectHoles(const Mat& image) {
    auto all_detections = detector_.detectHoles(image, true);

    if (all_detections.empty()) {
        cerr << "No holes detected!" << endl;
        return vector<Point2f>();
    }

    vector<Point2f> hole_centers;
    int expected_shots = (all_detections.size() >= 10) ? 10 : 4;
    expected_shots = min(expected_shots, (int)all_detections.size());

    for (int i = 0; i < expected_shots; i++) {
        hole_centers.push_back(all_detections[i].center);
    }

    return hole_centers;
}

ShootingMetrics PMWeapon::calculateMetrics(const vector<Point2f>& holes, double pixels_per_cm, const Mat& image) {
    Point2f target_center = metrics_calc_.findTargetCenter(image);
    Point2f stp = metrics_calc_.calculateSTP(holes);
    ShootingMetrics metrics = metrics_calc_.calculateMetrics(holes, pixels_per_cm);

    metrics.target_center = target_center;
    metrics.distance_to_center_cm = round((norm(stp - target_center) / pixels_per_cm) * 100.0) / 100.0;

    return metrics;
}