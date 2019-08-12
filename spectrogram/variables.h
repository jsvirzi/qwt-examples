#ifndef VARIABLES_H
#define VARIABLES_H

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>

#include <stdint.h>

class JsvHistogram {
public:
    JsvHistogram(double x_min, double x_max, double y_min, double y_max, unsigned int history);
    ~JsvHistogram();
    double image_entropy(uint8_t *b, int width, int height);
    TH1I *hist;
    TCanvas *canvas;
    TApplication *theApp;
    double *entropy;
    unsigned int data_phase;
    unsigned int history_length;
    unsigned int history_mask;
    void updateDataPhase(double entropy);
};

#endif

