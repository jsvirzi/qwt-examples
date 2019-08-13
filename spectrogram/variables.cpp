#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "variables.h"

static double log_factorial_lut[256];

double JsvHistogram::image_entropy(uint8_t *b, int width, int height) {
    uint32_t total_occ = 0;
    uint32_t N = width * height;
    double log_entropy = 0.0;
    for (int row = 0; row < height; ++row) {
        uint8_t *p = &b[row * width];
        for (int col = 0; col < width; ++col) {
            uint8_t byte = *p++;
            total_occ += byte;
            log_entropy += log_factorial_lut[byte];
        }
    }
    unsigned int mean_occupancy = floor(total_occ / N);
    printf("mean occupancy = %d\n", mean_occupancy);
    if (mean_occupancy >= 256) {
        printf("mean occupancy = %d\n", mean_occupancy);
        mean_occupancy &= 0xff;
    }
    double mean_entropy = N * log_factorial_lut[mean_occupancy];
    double logr = mean_entropy - log_entropy;
    return logr;
}

void JsvHistogram::image_occupancy_states(uint8_t *b, int width, int height) {
    if (h_occupancy == NULL) { delete h_occupancy; }
    double occupancy[256], occupancy_integral[256], integral = 0.0;

    h_occupancy = new TH1I("occupancy", "occupancy", 256, 0, 256);
    for (int row = 0; row < height; ++row) {
        uint8_t *p = &b[row * width];
        for (int col = 0; col < width; ++col) {
            uint8_t occ = *p++;
            h_occupancy->Fill(occ);
        }
    }

    if (g_occupancy == NULL) { delete g_occupancy; }
    g_occupancy = new TGraphErrors(max_occupancy, occ_axis, occupancy);
    for (unsigned int i = 0; i < 256; ++i) {
        unsigned int bin = i + 1;
        occupancy[i] = h_occupancy->GetBinContent(bin);
        occupancy_integral[i] = integral + occupancy[i];
    }
}

JsvHistogram::JsvHistogram(double x_min, double x_max, double y_min, double y_max, unsigned int history) {

    int i = 0;

    int frame_index = 0;
    printf("opened video\n");
    cv::VideoCapture video_capture("/home/jsvirzi/projects/0_1565280852557.mp4");
    if (video_capture.isOpened()) {
        printf("opening video\n");
    } else {
        printf("failed to open file\n");
        return;
    }

    while (1) {
        cv::Mat frame;
        printf("capture frame %d\n", frame_index++);
        video_capture >> frame;
        if (frame.empty()) { printf("we're done\n"); break; }
        imshow("Frame", frame);
        char ch = (char) cv::waitKey(10);
        if (ch == 0x1b) { break; }
    }

    video_capture.release();

    h_occupancy = NULL;
    g_occupancy = NULL;
    g_occupancy_integral = NULL;
    max_occupancy = 256;

    for (i = 0; i < 256; ++i) {
        occ_axis[i] = (double) i;
    }

    const double c = 0.5 * log(2.0 * M_PI);

    i = 0;
    log_factorial_lut[i++] = log(1.0);
    log_factorial_lut[i++] = log(2 * 1.0);
    log_factorial_lut[i++] = log(3 * 2 * 1.0);
    log_factorial_lut[i++] = log(4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(8 * 7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    for (; i < 256; ++i) {
        double eye = (double) i;
        log_factorial_lut[i] = c + 0.5 * log(eye) + eye * log(eye) - eye;
    }

    fprintf(stdout, "log facs = %lf vs %lf\n", log_factorial_lut[8], c + 0.5 * log(9.0) + 9.0 * log(9.0) - 9.0);

    if (history == 0) { return; }

    history = x_min + x_max + y_min + y_max;
    int argc = 1;
    char *arg1 = "this_main_app";
    char **argv = &arg1; // = { "this_main_app" };
    theApp = new TApplication("theApp", &argc, argv);
    canvas = new TCanvas("canvas", "canvas", 800, 600);
//    hist = new TH1I("hist", "this", 10, 0, 10);
//    hist->Fill(5.0);
//    hist->Fill(4.2);
//    hist->Fill(8.2);
//    hist->Draw("HIST");
    canvas->Draw();
    canvas->Update();

    printf("entropy = %d\n", history);

    entropy = new double[history + 1];
    data_phase = 0;
    history_mask = history - 1;
    history_length = history;

    canvas->WaitPrimitive();
}



JsvHistogram::~JsvHistogram() {
    if (entropy != NULL) { delete [] entropy; entropy = NULL; }
    if (canvas != NULL) { delete canvas; canvas = NULL; }
    if (h_occupancy != NULL) { delete h_occupancy; h_occupancy = NULL; }
    if (g_occupancy != NULL) { delete g_occupancy; g_occupancy = NULL; }
    if (g_occupancy_integral != NULL) { delete g_occupancy_integral; g_occupancy_integral = NULL; }
    if (video_capture != NULL) { delete video_capture; video_capture = NULL; }
}

void JsvHistogram::updateDataPhase(double entropy_value) {
    entropy[data_phase] = entropy_value;
    data_phase = (data_phase + 1) % history_length;
}
