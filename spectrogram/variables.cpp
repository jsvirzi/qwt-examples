#include <stdint.h>

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include "variables.h"

double JsvHistogram::image_entropy(uint8_t *b, int width, int height) {
    return 0.0;
}

JsvHistogram::JsvHistogram(double x_min, double x_max, double y_min, double y_max, unsigned int history) {
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
    canvas->WaitPrimitive();

    entropy = new double[history + 1];
    data_phase = 0;
    history_mask = history - 1;
    history_length = history;
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
