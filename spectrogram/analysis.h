#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <stdint.h>

class Analysis {
public:
    Analysis(const char *filename);
    ~Analysis();
    double image_entropy(void);
    void image_occupancy_states(void);
    bool next_image();
    bool stop_image();
    bool image_health;
    TH1I *h_occupancy;
    TGraphErrors *g_occupancy;
    TGraphErrors *g_occupancy_integral;
    TCanvas *canvas;
    TApplication *theApp;
    double *entropy;
    unsigned int data_phase;
    unsigned int history_length;
    unsigned int history_mask;
    void updateData(double entropy);
    unsigned int max_occupancy;
    double occ_axis[256];
    cv::VideoCapture video_capture;
    cv::Mat frame_mat;
    cv::Mat gray_mat;
    int first_frame;
    bool video_run_flag;
    unsigned int image_height, image_width;
    unsigned int frame_index;
};

#endif

