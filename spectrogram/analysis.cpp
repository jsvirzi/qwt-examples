#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>
#include <TLatex.h>

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

void Analysis::image_occupancy_states(double ratio)
{
    unsigned int i;
    uint8_t *b = gray_mat.data;
    image_width = gray_mat.cols;
    image_height = gray_mat.rows;
    unsigned int rows = image_height;
    unsigned int cols = image_width;
    if (h_occupancy != NULL) { delete h_occupancy; }

    h_occupancy = new TH1I("occupancy", "occupancy", 256, 0, 256);
    for (unsigned int row = 0; row < rows; ++row) {
        uint8_t *p = &b[row * cols];
        for (int col = 0; col < cols; ++col) {
            uint8_t occ = *p++;
            h_occupancy->Fill(occ);
        }
    }

    double integral = 0.0;
    for (i = 0; i < 256; ++i) {
        unsigned int bin = i + 1;
        occupancy[i] = h_occupancy->GetBinContent(bin);
        occupancy_integral[i] = integral + occupancy[i];
        integral = occupancy_integral[i];
        g_occupancy->SetPoint(i, occ_axis[i], occupancy[i]);
        g_occupancy_integral->SetPoint(i, occ_axis[i], occupancy_integral[i] / 256.0);
    }
    occupancy[i] = 0.0;
    occupancy_integral[i] = integral + occupancy[i];

    double x = ratio * 128;
    double y_min = 0.0, y_max = occupancy_integral[255] / 32.0;
    TLine *line = new TLine(x, y_min, x, y_max);
    line->SetLineColor(38);
    line->SetLineWidth(4);

//    g_occupancy->SetPoint(i, occ_axis[i], occupancy[i]);
//    g_occupancy_integral->SetPoint(i, occ_axis[i], occupancy_integral[i]);

    canvas->cd();
    h_occupancy->Draw("hist");
    g_occupancy->Draw("L");
    g_occupancy_integral->Draw("L");
    line->Draw();
    canvas->Draw();
    canvas->Update();
//    canvas->WaitPrimitive();
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
    for (i = 0; i <= 256; ++i) { occ_axis[i] = (double) i; }
    max_occupancy = 256;
    occupancy = new double[max_occupancy + 1];
    occupancy_integral = new double[max_occupancy + 1];
    g_occupancy = new TGraphErrors(max_occupancy, occ_axis, occupancy);
    g_occupancy_integral = new TGraphErrors(max_occupancy, occ_axis, occupancy_integral);
    TGraphErrors *g = g_occupancy;
    g->SetLineColor(kRed);
    g->SetLineWidth(3);
    g = g_occupancy_integral;
    g->SetLineColor(kBlue);
    g->SetLineWidth(3);
    h_occupancy = NULL;

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
