#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "analysis.h"

static const unsigned int max_history_size = 32768;
static double log_factorial_lut[256];

double Analysis::image_entropy(void) {
    uint8_t *b = gray_mat.data;
    image_width = gray_mat.cols;
    image_height = gray_mat.rows;
    unsigned int rows = image_height;
    unsigned int cols = image_width;
    uint32_t total_occ = 0;
    uint32_t N = cols * rows;
    double log_entropy = 0.0;
    for (int row = 0; row < rows; ++row) {
        uint8_t *p = &b[row * cols];
        for (int col = 0; col < cols; ++col) {
            uint8_t byte = *p++;
            total_occ += byte;
            log_entropy += log_factorial_lut[byte];
        }
    }
    unsigned int mean_occupancy = floor(total_occ / N);
    printf("mean occupancy = %d\n", mean_occupancy);
    if (mean_occupancy >= 256) {
        printf("*** mean occupancy = %d\n", mean_occupancy);
        mean_occupancy &= 0xff;
    }
    double log_mean_entropy = N * log_factorial_lut[mean_occupancy];
    double logr = log_mean_entropy - log_entropy;
    printf("log(entropy) = %lf = %lf - %lf\n", logr, log_entropy, log_mean_entropy);
    return logr;
}

void Analysis::image_occupancy_states(void)
{
    uint8_t *b = gray_mat.data;
    image_width = gray_mat.cols;
    image_height = gray_mat.rows;
    unsigned int rows = image_height;
    unsigned int cols = image_width;
    if (h_occupancy != NULL) { delete h_occupancy; }
    if (g_occupancy != NULL) { delete g_occupancy; }
    if (g_occupancy_integral != NULL) { delete g_occupancy_integral; }

    double occupancy[256], occupancy_integral[256], integral = 0.0;

    h_occupancy = new TH1I("occupancy", "occupancy", 256, 0, 256);
    for (int row = 0; row < rows; ++row) {
        uint8_t *p = &b[row * cols];
        for (int col = 0; col < cols; ++col) {
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

#if 1
    canvas->cd();
    h_occupancy->Draw("hist");
//    g_occupancy->Draw("L");
//    g_occupancy_integral->Draw("L");
    canvas->Draw();
    canvas->Update();
//    canvas->WaitPrimitive();
#endif
}

bool Analysis::next_image() {
    video_capture >> frame_mat;
    cvtColor(frame_mat, gray_mat, cv::COLOR_RGB2GRAY);
    if (frame_mat.empty()) {
        image_health = false;
        return image_health;
    }

    ++frame_index;

    if (first_frame) {
        printf("C: rows = %d. cols = %d. channels = %d\n", frame_mat.rows, frame_mat.cols, frame_mat.channels());
        printf("G: rows = %d. cols = %d. channels = %d\n", gray_mat.rows, gray_mat.cols, gray_mat.channels());
        first_frame = 0;
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

    frame_index = 0;
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

    for (i = 0; i < 256; ++i) { occ_axis[i] = (double) i; }

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
    canvas = new TCanvas("canvas", "canvas", 1600, 1200);

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
