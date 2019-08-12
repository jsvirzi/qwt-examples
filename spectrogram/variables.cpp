#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include <math.h>

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

JsvHistogram::JsvHistogram(double x_min, double x_max, double y_min, double y_max, unsigned int history) {

    const double c = 0.5 * log(2.0 * M_PI);

    int i = 0;
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
    hist = new TH1I("hist", "this", 10, 0, 10);
    hist->Fill(5.0);
    hist->Fill(4.2);
    hist->Fill(8.2);
    hist->Draw("HIST");
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
    if (hist != NULL) { delete hist; hist = NULL; }
}

void JsvHistogram::updateDataPhase(double entropy_value) {
    entropy[data_phase] = entropy_value;
    data_phase = (data_phase + 1) % history_length;
}
