#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "analysis.h"

static const unsigned int max_history_size = 2048;
static double log_factorial_lut[256];

double Analysis::image_entropy(uint8_t *b, int width, int height) {
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

void Analysis::image_occupancy_states(uint8_t *b, int width, int height) {
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

bool Analysis::next_image() {
    if (first_frame) {
        printf("C: rows = %d. cols = %d. channels = %d. size = %d\n", frame_mat.rows, frame_mat.cols, frame_mat.channels());
        printf("G: rows = %d. cols = %d. channels = %d. size = %d\n", gray_mat.rows, gray_mat.cols, gray_mat.channels());
        first_frame = 0;
    }
    video_capture >> frame_mat;
    cvtColor(frame_mat, gray_mat, cv::COLOR_RGB2GRAY);
    if (frame_mat.empty()) {
        image_health = false;
        return image_health;
    }

    imshow("gray", gray_mat);
    char ch = (char) cv::waitKey(10);
    if (ch == 0x1b) { video_run_flag = false; }
}

bool Analysis::stop_image() {
    video_capture.release();
    video_capture = NULL;
}

Analysis::Analysis(const char *filename) {

    int i = 0;

    int frame_index = 0;
    printf("opened video\n");
    video_capture = cv::VideoCapture(filename);
    image_health = video_capture.isOpened();
    if (image_health) { printf("opening video\n"); }
    else { printf("failed to open file\n"); }

    cv::namedWindow("gray");

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

    int argc = 1;
    char *arg1 = "this_main_app";
    char **argv = &arg1; // = { "this_main_app" };
    theApp = new TApplication("theApp", &argc, argv);
    canvas = new TCanvas("canvas", "canvas", 800, 600);

    entropy = new double[max_history_size];
    data_phase = 0;
    history_mask = max_history_size - 1;
    history_length = max_history_size;

    first_frame = 1;
}



Analysis::~Analysis() {
    if (entropy != NULL) { delete [] entropy; entropy = NULL; }
    if (canvas != NULL) { delete canvas; canvas = NULL; }
    if (h_occupancy != NULL) { delete h_occupancy; h_occupancy = NULL; }
    if (g_occupancy != NULL) { delete g_occupancy; g_occupancy = NULL; }
    if (g_occupancy_integral != NULL) { delete g_occupancy_integral; g_occupancy_integral = NULL; }
    // if (video_capture != NULL) { delete video_capture; video_capture = NULL; }
}

void Analysis::updateData(double entropy_value) {
    entropy[data_phase] = entropy_value;
    data_phase = (data_phase + 1) % history_length;
}
