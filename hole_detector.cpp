#include "hole_detector.h"
#include <iostream>
#include <algorithm>

using namespace cv;
using namespace std;

HoleDetector::HoleDetector() {}

vector<DetectedHole> HoleDetector::detectHoles(const Mat& image, bool debug) {
    // Автоматический расчет масштаба
    double PIXELS_PER_CM = calculatePixelsPerCM(image);

    // Параметры
    const double HOOK_ZONE_CM = 7.0;
    const double MERGE_RADIUS_CM = 1.5;
    const int MIN_SHOTS = 4;
    const int MAX_SHOTS = 10;

    // Детекция красных кластеров
    auto holes = findRedClusters(image, debug);
    cout << "Found " << holes.size() << " red clusters" << endl;

    if (holes.empty()) return holes;

    // Объединение близких пробоин
    auto merged = mergeCloseHoles(holes, MERGE_RADIUS_CM * PIXELS_PER_CM);
    cout << "After merging: " << merged.size() << " candidates" << endl;

    // Разделение на нижние и верхние (крючки)
    auto split_result = splitByHookZone(merged, HOOK_ZONE_CM, PIXELS_PER_CM);
    auto lower_holes = split_result.first;
    auto upper_holes = split_result.second;

    cout << "Lower: " << lower_holes.size() << ", Upper: " << upper_holes.size() << endl;

    // Формирование финального списка
    vector<DetectedHole> final_candidates = lower_holes;

    // Добавляем из верхних если нужно
    for (size_t i = 0; i < upper_holes.size() && final_candidates.size() < MIN_SHOTS; ++i) {
        final_candidates.push_back(upper_holes[i]);
    }

    //// Если всё ещё мало, добавляем любые из объединенных
    //for (const auto& h : merged) {
    //    if (final_candidates.size() >= MIN_SHOTS) break;
    //    bool exists = false;
    //    for (const auto& f : final_candidates) {
    //        if (norm(f.center - h.center) < 1e-3) {
    //            exists = true;
    //            break;
    //        }
    //    }
    //    if (!exists) final_candidates.push_back(h);
    //}

    // Ограничение максимального количества
    if (final_candidates.size() > MAX_SHOTS) {
        final_candidates.resize(MAX_SHOTS);
    }

    cout << "Final: " << final_candidates.size() << " holes" << endl;
    return final_candidates;
}

double HoleDetector::calculatePixelsPerCM(const Mat& image) {
    const double A3_WIDTH_MM = 300.0;
    const double A3_HEIGHT_MM = 420.0;

    double px_per_mm_width = image.cols / A3_WIDTH_MM;
    double px_per_mm_height = image.rows / A3_HEIGHT_MM;
    double px_per_cm = (px_per_mm_width + px_per_mm_height) / 2.0 * 10.0;

    cout << "Pixels per cm: " << px_per_cm << endl;
    return px_per_cm;
}

vector<DetectedHole> HoleDetector::findRedClusters(const Mat& image, bool debug) {
    vector<DetectedHole> holes;

    Mat hsv;
    cvtColor(image, hsv, COLOR_BGR2HSV);

    // Диапазон для красного цвета
    Mat mask1, mask2, red_mask;
    inRange(hsv, Scalar(0, 100, 50), Scalar(10, 255, 255), mask1);
    inRange(hsv, Scalar(170, 100, 50), Scalar(180, 255, 255), mask2);
    bitwise_or(mask1, mask2, red_mask);

    if (debug) imwrite("red_mask.jpg", red_mask);

    // Находим связные компоненты
    Mat labels, stats, centroids;
    int num_components = connectedComponentsWithStats(red_mask, labels, stats, centroids);

    // Собираем информацию о кластерах
    for (int i = 1; i < num_components; i++) {
        int area = stats.at<int>(i, CC_STAT_AREA);
        if (area < 15 || area > 5000) continue;

        Point2f center(centroids.at<double>(i, 0), centroids.at<double>(i, 1));
        holes.push_back({ center, area });
    }

    // Сортируем по размеру
    sort(holes.begin(), holes.end(), [](const DetectedHole& a, const DetectedHole& b) {
        return a.pixel_count > b.pixel_count;
        });

    return holes;
}

vector<DetectedHole> HoleDetector::mergeCloseHoles(const vector<DetectedHole>& holes, double merge_px) {
    vector<DetectedHole> merged;
    vector<bool> used(holes.size(), false);

    for (size_t i = 0; i < holes.size(); ++i) {
        if (used[i]) continue;

        Point2f accum = holes[i].center;
        int accum_pixels = holes[i].pixel_count;
        int cnt = 1;
        used[i] = true;

        for (size_t j = i + 1; j < holes.size(); ++j) {
            if (used[j]) continue;
            if (norm(holes[i].center - holes[j].center) <= merge_px) {
                accum += holes[j].center;
                accum_pixels += holes[j].pixel_count;
                cnt++;
                used[j] = true;
            }
        }
        merged.push_back({ accum * (1.0f / cnt), accum_pixels });
    }

    sort(merged.begin(), merged.end(), [](const DetectedHole& a, const DetectedHole& b) {
        return a.pixel_count > b.pixel_count;
        });

    return merged;
}

pair<vector<DetectedHole>, vector<DetectedHole>>
HoleDetector::splitByHookZone(const vector<DetectedHole>& holes, double hook_zone_cm, double pixels_per_cm) {
    vector<DetectedHole> lower, upper;
    double cutoff_px = hook_zone_cm * pixels_per_cm;

    for (const auto& h : holes) {
        if (h.center.y < cutoff_px) upper.push_back(h);
        else lower.push_back(h);
    }

    return make_pair(lower, upper);
}