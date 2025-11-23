#ifndef SHOOTING_METRICS_H
#define SHOOTING_METRICS_H

#include <opencv2/opencv.hpp>
#include <vector>

struct ShootingMetrics {
    double precision;               // Среднее расстояние до СТП
    double group_radius;            // Макс расстояние до СТП  
    cv::Point2f stp;                // Средняя точка попадания
    double precision_cm;            // Кучность в см
    double group_radius_cm;         // Радиус группы в см
    double distance_to_center_cm;   // Расстояние от СТП до центра мишени
    cv::Point2f target_center;      // Координаты центра мишени
};

class ShootingMetricsCalculator {
public:
    ShootingMetrics calculateMetrics(const std::vector<cv::Point2f>& holes, double pixels_per_cm);
    cv::Point2f calculateSTP(const std::vector<cv::Point2f>& holes);
    cv::Point2f findTargetCenter(const cv::Mat& image);  // Новая функция

private:
    cv::Point2f calculateSTP4Shots(const std::vector<cv::Point2f>& holes);
};

#endif