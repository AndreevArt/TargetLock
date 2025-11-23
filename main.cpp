#include <opencv2/opencv.hpp>
#include <iostream>
#include "weapons/pm.h"
#include "common/visualization.h"

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

    // Используем модуль ПМ
    PMWeapon pm;

    // Детекция пробоин
    auto hole_centers = pm.detectHoles(image);
    if (hole_centers.empty()) {
        cerr << "No holes detected!" << endl;
        return -1;
    }

    // Вычисление метрик
    HoleDetector detector;
    double pixels_per_cm = detector.calculatePixelsPerCM(image);
    ShootingMetrics metrics = pm.calculateMetrics(hole_centers, pixels_per_cm, image);

    // Визуализация
    Visualization visualizer(pixels_per_cm);
    Mat result = image.clone();

    // Получаем все детекции для визуализации
    auto all_detections = detector.detectHoles(image, false);

    visualizer.drawShootingResult(result, hole_centers, all_detections, metrics.stp, metrics);

    imwrite("shooting_result.jpg", result);

    namedWindow("Shooting Analysis", WINDOW_NORMAL);
    resizeWindow("Shooting Analysis", 1000, 800);
    imshow("Shooting Analysis", result);

    waitKey(0);
    return 0;
}