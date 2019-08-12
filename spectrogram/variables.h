#ifndef VARIABLES_H
#define VARIABLES_H

#include <TApplication.h>
#include <TH1I.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>

#include <stdint.h>

class JsvHistogram {
public:
    JsvHistogram(double x_min, double x_max, double y_min, double y_max, unsigned int history);
    ~JsvHistogram();
    double image_entropy(uint8_t *b, int width, int height);
    void image_occupancy_states(uint8_t *b, int width, int height);
    TH1I *h_occupancy;
    TGraphErrors *g_occupancy;
    TGraphErrors *g_occupancy_integral;
    TCanvas *canvas;
    TApplication *theApp;
    double *entropy;
    unsigned int data_phase;
    unsigned int history_length;
    unsigned int history_mask;
    void updateDataPhase(double entropy);
    unsigned int max_occupancy;
    double occ_axis[256];
};

#endif

