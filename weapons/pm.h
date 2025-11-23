#ifndef PM_WEAPON_H
#define PM_WEAPON_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "../common/hole_detector.h"
#include "../common/shooting_metrics.h"

class PMWeapon {
public:
    std::vector<cv::Point2f> detectHoles(const cv::Mat& image);
    ShootingMetrics calculateMetrics(const std::vector<cv::Point2f>& holes, double pixels_per_cm, const cv::Mat& image);

private:
    HoleDetector detector_;
    ShootingMetricsCalculator metrics_calc_;
};

#endif