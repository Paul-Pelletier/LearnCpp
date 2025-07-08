#include "regression.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <set>

// Fonction de régression linéaire unique
LinearRegressionStats compute_regression_stats(const OptionChainQuote* data, int size, int64_t expiry, int64_t quote_unixtime) {
    LinearRegressionStats stats = { 0 };
    double* x = (double*)malloc(size * sizeof(double));
    double* y = (double*)malloc(size * sizeof(double));
    int count = 0;

    for (int i = 0; i < size; ++i) {
        if (data[i].expire_unixtime == expiry && data[i].c_mid > 0 && data[i].p_mid > 0) {
            x[count] = data[i].strike;
            y[count] = data[i].c_mid - data[i].p_mid;
            count++;
        }
    }

    if (count < 2) {
        stats.count = count;
        free(x);
        free(y);
        return stats;
    }

    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (int i = 0; i < count; ++i) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    double mean_x = sum_x / count;
    double mean_y = sum_y / count;
    double denom = sum_x2 - count * mean_x * mean_x;

    if (denom == 0.0) {
        stats.intercept = mean_y;
        stats.slope = 0.0;
        stats.count = count;
        free(x);
        free(y);
        return stats;
    }

    stats.slope = (sum_xy - count * mean_x * mean_y) / denom;
    stats.intercept = mean_y - stats.slope * mean_x;

    double ss_total = 0.0, ss_res = 0.0;
    for (int i = 0; i < count; ++i) {
        double y_hat = stats.intercept + stats.slope * x[i];
        double err = y[i] - y_hat;
        ss_res += err * err;
        ss_total += (y[i] - mean_y) * (y[i] - mean_y);
    }

    double mse = ss_res / count;
    stats.rmse = sqrt(mse);
    stats.r_squared = (ss_total == 0.0) ? 0.0 : 1.0 - (ss_res / ss_total);
    stats.count = count;

    double maturity = (expiry - quote_unixtime) / 31556952.0;
    if (maturity <= 0.0) maturity = 0.000001;

    stats.maturity = maturity;
    stats.discount_factor = -stats.slope;
    stats.forward = stats.discount_factor != 0.0 ? stats.intercept / (-stats.slope) : 0.0;
    stats.zero_rate = -log(stats.discount_factor) / maturity;

    free(x);
    free(y);
    return stats;
}

// Fonction de régression pour chaque échéance unique
ForwardRegressionResult compute_forward_regressions(const OptionChainQuote* quotes, int n) {
    std::set<int64_t> unique_expiries;
    for (int i = 0; i < n; ++i) {
        unique_expiries.insert(quotes[i].expire_unixtime);
    }

    ForwardRegressionResult result;
    result.count = unique_expiries.size();
    result.expiries = (int64_t*)malloc(result.count * sizeof(int64_t));
    result.regressions = (LinearRegressionStats*)malloc(result.count * sizeof(LinearRegressionStats));

    size_t index = 0;
    for (int64_t expiry : unique_expiries) {
        result.expiries[index] = expiry;
        result.regressions[index] = compute_regression_stats(quotes, n, expiry, quotes[0].quote_unixtime);
        ++index;
    }

    return result;
}
