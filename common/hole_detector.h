#ifndef HOLE_DETECTOR_H
#define HOLE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

struct DetectedHole {
    cv::Point2f center;
    int pixel_count;
};

class HoleDetector {
public:
    HoleDetector();
    std::vector<DetectedHole> detectHoles(const cv::Mat& image, bool debug = false);
    double calculatePixelsPerCM(const cv::Mat& image);

private:
    std::vector<DetectedHole> findRedClusters(const cv::Mat& image, bool debug);
    std::vector<DetectedHole> mergeCloseHoles(const std::vector<DetectedHole>& holes, double merge_px);
    std::pair<std::vector<DetectedHole>, std::vector<DetectedHole>>
        splitByHookZone(const std::vector<DetectedHole>& holes, double hook_zone_cm, double pixels_per_cm);
};

#endif