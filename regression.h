#pragma once
#ifndef REGRESSION_H
#define REGRESSION_H

#include "option_data.h"

struct LinearRegressionStats {
    double intercept;
    double slope;
    double r_squared;
    double rmse;
    int count;               // Nombre de points utilisés
    double maturity;
    double discount_factor;
    double forward;
    double zero_rate;
};

struct ForwardRegressionResult {
    int64_t* expiries;
    struct LinearRegressionStats* regressions;
    size_t count;
};

struct LinearRegressionStats linear_regression(const double* x, const double* y, size_t n);
struct ForwardRegressionResult compute_forward_regressions(const struct OptionChainQuote* quotes, int n);

#endif
