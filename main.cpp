#include <opencv2/opencv.hpp>
#include <iostream>
#include "hole_detector.h"
#include "shooting_metrics.h"
#include "visualization.h"

using namespace cv;
using namespace std;

int main() {
    cout << "=== SHOOTING ANALYZER ===" << endl;

    Mat image = imread("target.jpg");
    if (image.empty()) {
        cerr << "Cannot load target.jpg!" << endl;
        return -1;
    }

    cout << "Image: " << image.cols << "x" << image.rows << endl;

    // Детекция пробоин
    HoleDetector detector;
    auto all_detections = detector.detectHoles(image, true);

    if (all_detections.empty()) {
        cerr << "No holes detected!" << endl;
        return -1;
    }

    // Получение центров
    vector<Point2f> hole_centers;
    int expected_shots = (all_detections.size() >= 10) ? 10 : 4;
    expected_shots = min(expected_shots, (int)all_detections.size());

    for (int i = 0; i < expected_shots; i++) {
        hole_centers.push_back(all_detections[i].center);
    }

    // Вычисление метрик
    double pixels_per_cm = detector.calculatePixelsPerCM(image);
    ShootingMetricsCalculator metrics_calc;

    // НАХОДИМ ЦЕНТР МИШЕНИ
    Point2f target_center = metrics_calc.findTargetCenter(image);

    Point2f stp = metrics_calc.calculateSTP(hole_centers);
    ShootingMetrics metrics = metrics_calc.calculateMetrics(hole_centers, pixels_per_cm);

    // ДОБАВЛЯЕМ РАССТОЯНИЕ ДО ЦЕНТРА
    metrics.target_center = target_center;
    metrics.distance_to_center_cm = round((norm(stp - target_center) / pixels_per_cm) * 100.0) / 100.0;

    // Вывод результатов
    cout << "\n=== SHOOTING METRICS ===" << endl;
    cout << "Shots: " << expected_shots << endl;
    cout << "STP: (" << stp.x << ", " << stp.y << ")" << endl;
    cout << "Target Center: (" << target_center.x << ", " << target_center.y << ")" << endl;
    cout << "Distance to center: " << fixed << setprecision(2) << metrics.distance_to_center_cm << " cm" << endl;
    cout << "Precision: " << fixed << setprecision(1) << metrics.precision_cm << " cm" << endl;
    cout << "Group Radius: " << fixed << setprecision(1) << metrics.group_radius_cm << " cm" << endl;

    // Визуализация
    Visualization visualizer(pixels_per_cm);
    Mat result = image.clone();

    // Передаем ВСЕ параметры в визуализацию
    visualizer.drawShootingResult(result, hole_centers, all_detections, stp, metrics);

    imwrite("shooting_result.jpg", result);

    namedWindow("Shooting Analysis", WINDOW_NORMAL);
    resizeWindow("Shooting Analysis", 1000, 800);
    imshow("Shooting Analysis", result);

    waitKey(0);
    return 0;
}