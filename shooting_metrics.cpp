#include "shooting_metrics.h"
#include <numeric>
#include <algorithm>

using namespace cv;
using namespace std;

ShootingMetrics ShootingMetricsCalculator::calculateMetrics(const vector<Point2f>& holes, double pixels_per_cm) {
    ShootingMetrics metrics;
    if (holes.empty()) return metrics;

    metrics.stp = calculateSTP(holes);

    // Вычисляем расстояния до СТП
    vector<double> distances;
    for (const auto& hole : holes) {
        distances.push_back(norm(hole - metrics.stp));
    }

    metrics.precision = accumulate(distances.begin(), distances.end(), 0.0) / holes.size();
    metrics.group_radius = *max_element(distances.begin(), distances.end());

    // Округляем до 2 знаков после запятой
    metrics.precision_cm = round((metrics.precision / pixels_per_cm) * 100.0) / 100.0;
    metrics.group_radius_cm = round((metrics.group_radius / pixels_per_cm) * 100.0) / 100.0;

    return metrics;
}

Point2f ShootingMetricsCalculator::findTargetCenter(const Mat& image) {
    Mat gray, binary;
    cvtColor(image, gray, COLOR_BGR2GRAY);

    // Простая бинаризация черного
    threshold(gray, binary, 80, 255, THRESH_BINARY_INV);

    // Морфология для объединения
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);

    // Ожидаемый центр (примерно 15см от левого края, 28см от верха)
    double expected_x = image.cols / 2.0;
    double expected_y = image.rows * 0.666;  // 28/42 ≈ 0.666

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return Point2f(expected_x, expected_y);
    }

    // Просто ищем самый большой черный объект
    double max_area = 0;
    Point2f best_center(expected_x, expected_y);

    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > max_area && area > 1000) {  // Минимальная площадь
            max_area = area;
            Moments m = moments(contour);
            best_center = Point2f(m.m10 / m.m00, m.m01 / m.m00);
        }
    }

    return best_center;
}

Point2f ShootingMetricsCalculator::calculateSTP(const vector<Point2f>& holes) {
    if (holes.size() != 4) {
        // Простой центр масс для не-4 выстрелов
        Point2f sum(0, 0);
        for (const auto& hole : holes) sum += hole;
        return sum * (1.0f / holes.size());
    }
    return calculateSTP4Shots(holes);
}

Point2f ShootingMetricsCalculator::calculateSTP4Shots(const vector<Point2f>& holes) {
    // Находим ближайшую пару
    double min_dist = numeric_limits<double>::max();
    pair<int, int> closest_pair = { 0, 1 };

    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            double dist = norm(holes[i] - holes[j]);
            if (dist < min_dist) {
                min_dist = dist;
                closest_pair = { i, j };
            }
        }
    }

    Point2f A = holes[closest_pair.first];
    Point2f B = holes[closest_pair.second];
    Point2f M1 = (A + B) * 0.5f;

    // Находим третью точку (ближайшую к M1)
    Point2f C;
    double min_dist_to_M1 = numeric_limits<double>::max();

    for (int i = 0; i < 4; i++) {
        if (i != closest_pair.first && i != closest_pair.second) {
            double dist = norm(holes[i] - M1);
            if (dist < min_dist_to_M1) {
                min_dist_to_M1 = dist;
                C = holes[i];
            }
        }
    }

    Point2f M2 = M1 + (C - M1) * (1.0f / 3.0f);

    // Находим четвертую точку
    Point2f D;
    for (int i = 0; i < 4; i++) {
        if (i != closest_pair.first && i != closest_pair.second && holes[i] != C) {
            D = holes[i];
            break;
        }
    }

    return M2 + (D - M2) * (1.0f / 4.0f);
}