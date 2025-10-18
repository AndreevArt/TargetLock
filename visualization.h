#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "shooting_metrics.h"
#include "hole_detector.h"

class Visualization {
public:
    Visualization(double pixels_per_cm);

    void drawShootingResult(cv::Mat& image,
        const std::vector<cv::Point2f>& holes,
        const std::vector<DetectedHole>& all_detections,
        const cv::Point2f& stp,
        const ShootingMetrics& metrics);

    void drawSTPProcess(cv::Mat& image,
        const std::vector<cv::Point2f>& holes,
        const cv::Point2f& stp);

private:
    double pixels_per_cm_;

    // Вспомогательная функция для масштабирования
    int scaleToPixels(double cm);

    void drawHoles(cv::Mat& image, const std::vector<cv::Point2f>& holes);
    void drawSTP(cv::Mat& image, const cv::Point2f& stp);
    void drawMetrics(cv::Mat& image, const ShootingMetrics& metrics, int total_shots);
    void drawGroupCircle(cv::Mat& image, const cv::Point2f& stp, double radius);
    void drawTargetCenter(cv::Mat& image, const cv::Point2f& center);
    void drawCenterLine(cv::Mat& image, const cv::Point2f& stp, const cv::Point2f& center, double distance_cm);
};

#endif