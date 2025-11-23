#include "visualization.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace cv;
using namespace std;

Visualization::Visualization(double pixels_per_cm) : pixels_per_cm_(pixels_per_cm) {}

// Вспомогательная функция для масштабирования размеров
int Visualization::scaleToPixels(double cm) {
    return max(1, (int)round(cm * pixels_per_cm_));
}

void Visualization::drawShootingResult(Mat& image,
    const vector<Point2f>& holes,
    const vector<DetectedHole>& all_detections,
    const Point2f& stp,
    const ShootingMetrics& metrics) {
    
    // Рисуем все детекции (светло-серым) - очень тонкие
    for (const auto& detection : all_detections) {
        circle(image, detection.center, scaleToPixels(0.15), Scalar(180, 180, 180), scaleToPixels(0.03));
    }
    //Рисуем центр мишени
    drawTargetCenter(image, metrics.target_center);
    // Рисуем используемые пробоины (яркими цветами)
    drawHoles(image, holes);

    // Рисуем круг группы (используем метрику!)
    drawGroupCircle(image, stp, metrics.group_radius);

    // Для 4 выстрелов рисуем процесс вычисления STP
    if (holes.size() == 4) {
        drawSTPProcess(image, holes, stp);
    }
    //Линия от СТП до центра мишени
    drawCenterLine(image, stp, metrics.target_center, metrics.distance_to_center_cm);
    // Рисуем метрики
   // drawMetrics(image, metrics, holes.size());
    // Рисуем СТП
    drawSTP(image, stp);
}

void Visualization::drawSTPProcess(Mat& image, const vector<Point2f>& holes, const Point2f& stp) {
    Scalar line_color(255, 255, 0);  // Желтый для линий
    Scalar step_color(0, 255, 255);  // Желтый для промежуточных точек

    // Находим ближайшую пару
    double min_dist = numeric_limits<double>::max();
    pair<Point2f, Point2f> closest_pair;

    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            double dist = norm(holes[i] - holes[j]);
            if (dist < min_dist) {
                min_dist = dist;
                closest_pair = { holes[i], holes[j] };
            }
        }
    }

    Point2f M1 = (closest_pair.first + closest_pair.second) * 0.5f;
    
    // Тонкие линии 
    line(image, closest_pair.first, closest_pair.second, line_color, scaleToPixels(0.08));
    circle(image, M1, scaleToPixels(0.1), step_color, -1);

    // Находим третью точку
    Point2f C;
    double min_dist_to_M1 = numeric_limits<double>::max();

    for (const auto& hole : holes) {
        if (hole != closest_pair.first && hole != closest_pair.second) {
            double dist = norm(hole - M1);
            if (dist < min_dist_to_M1) {
                min_dist_to_M1 = dist;
                C = hole;
            }
        }
    }

    Point2f M2 = M1 + (C - M1) * (1.0f / 3.0f);
    line(image, M1, C, line_color, scaleToPixels(0.08));
    circle(image, M2, scaleToPixels(0.1), step_color, -1);

    // Находим четвертую точку
    Point2f D;
    for (const auto& hole : holes) {
        if (hole != closest_pair.first && hole != closest_pair.second && hole != C) {
            D = hole;
            break;
        }
    }

    line(image, M2, D, line_color, scaleToPixels(0.08));

    // Линии от точек до STP
    /*for (const auto& hole : holes) {
        line(image, hole, stp, Scalar(0, 255, 255), scaleToPixels(0.04));
    }*/
}

void Visualization::drawMetrics(Mat& image, const ShootingMetrics& metrics, int total_shots) {
    Scalar text_color(0, 0, 0);
    
    double title_scale = pixels_per_cm_ * 0.06; 
    double info_scale = pixels_per_cm_ * 0.04;   
    int title_thickness = max(1, scaleToPixels(0.06));
    int info_thickness = max(1, scaleToPixels(0.05));

    stringstream metrics_text;
    metrics_text << "Выстрелы: " << total_shots;
    metrics_text << " | Группа: " << fixed << setprecision(1) << metrics.group_radius_cm << "см";
    metrics_text << " | Кучность: " << fixed << setprecision(1) << metrics.precision_cm << "см";

    // Позиционируем текст - ближе к краю
    int title_y = scaleToPixels(1.5);  
    int info_y = title_y + scaleToPixels(0.8);  
    
    putText(image, "АНАЛИЗ СТРЕЛЬБЫ", Point(scaleToPixels(0.5), title_y),
        FONT_HERSHEY_COMPLEX, title_scale, text_color, title_thickness);
    putText(image, metrics_text.str(), Point(scaleToPixels(0.5), info_y),
        FONT_HERSHEY_COMPLEX, info_scale, text_color, info_thickness);

}

void Visualization::drawHoles(Mat& image, const vector<Point2f>& holes) {
    vector<Scalar> colors = {
        Scalar(0, 255, 0),    // Зеленый
        Scalar(255, 0, 0),    // Синий  
        Scalar(0, 255, 255),  // Желтый
        Scalar(255, 0, 255)   // Пурпурный
    };

    for (size_t i = 0; i < holes.size(); i++) {
        Scalar color = colors[i % colors.size()];
        const auto& hole = holes[i];

        // Внешний круг 
        circle(image, hole, scaleToPixels(0.2), color, scaleToPixels(0.06));
        // Внутренний круг 
        circle(image, hole, scaleToPixels(0.08), color, -1);

        // Номера выстрелов (уменьшенный размер)
        //double text_scale = pixels_per_cm_ * 0.020;  
        //int text_thickness = max(1, scaleToPixels(0.05));
        //putText(image, to_string(i + 1), hole + Point2f(scaleToPixels(0.25), scaleToPixels(-0.25)),
        //    FONT_HERSHEY_SIMPLEX, text_scale, color, text_thickness);
    }
}

void Visualization::drawSTP(Mat& image, const Point2f& stp) {
    Scalar point_color(0, 0, 255);   // Красный для СТП

    // Круг СТП 
    circle(image, stp, scaleToPixels(0.3), point_color, scaleToPixels(0.15));
    // Маленький круг внутри 
    circle(image, stp, scaleToPixels(0.125), point_color, -1);
    
    // Подпись СТП 
    double text_scale = pixels_per_cm_ * 0.02;  
    int text_thickness = max(1, scaleToPixels(0.07));
    putText(image, "STP", stp + Point2f(scaleToPixels(0.35), scaleToPixels(-0.35)),
        FONT_HERSHEY_SIMPLEX, text_scale, point_color, text_thickness);
}

void Visualization::drawGroupCircle(Mat& image, const Point2f& stp, double radius) {
    Scalar group_color(0, 0, 255);   // Красный для круга группы

    // Круг группы 
    circle(image, stp, (int)round(radius), group_color, scaleToPixels(0.12), LINE_AA);

    // Подпись радиуса в левом верхнем углу
    double radius_cm = radius / pixels_per_cm_;
    stringstream radius_text;
    radius_text << "Group radius: " << fixed << setprecision(1) << radius_cm << "cm";

    double text_scale = pixels_per_cm_ * 0.035;
    int text_thickness = max(1, scaleToPixels(0.05));

    // Позиция в левом верхнем углу
    Point text_position(scaleToPixels(1.0), scaleToPixels(2.0));

    putText(image, radius_text.str(), text_position,
        FONT_HERSHEY_SIMPLEX, text_scale, group_color, text_thickness);
}

// рисуем центр мишени
void Visualization::drawTargetCenter(Mat& image, const Point2f& center) {
    Scalar center_color(255, 255, 255);  // Белый цвет
    Scalar cross_color(0, 0, 0);         // Черный для контраста

    // Белый кружок
    circle(image, center, scaleToPixels(0.4), center_color, scaleToPixels(0.1));

    // Черный крестик внутри
    float cross_size = scaleToPixels(0.3);
    line(image, center - Point2f(cross_size, 0), center + Point2f(cross_size, 0), cross_color, scaleToPixels(0.05));
    line(image, center - Point2f(0, cross_size), center + Point2f(0, cross_size), cross_color, scaleToPixels(0.05));

    // Подпись
    double text_scale = pixels_per_cm_ * 0.035;
    putText(image, "CENTER", center + Point2f(scaleToPixels(0.5), scaleToPixels(-0.5)),
        FONT_HERSHEY_SIMPLEX, text_scale, center_color, scaleToPixels(0.05));
}

//  линия от СТП до центра
void Visualization::drawCenterLine(Mat& image, const Point2f& stp, const Point2f& center, double distance_cm) {
    Scalar line_color(255, 255, 255);  // Белый цвет

    // Линия от СТП до центра
    line(image, stp, center, line_color, scaleToPixels(0.08));

    // Текст с расстоянием посередине линии
    Point2f mid_point = (stp + center) * 0.5f;
    stringstream distance_text;
    distance_text << fixed << setprecision(2) << distance_cm << "cm";

    double text_scale = pixels_per_cm_ * 0.03;
    putText(image, distance_text.str(), mid_point + Point2f(scaleToPixels(0.2), scaleToPixels(0.2)),
        FONT_HERSHEY_SIMPLEX, text_scale, line_color, scaleToPixels(0.04));
}