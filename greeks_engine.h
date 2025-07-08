// greeks_engine.h  -----------------------------------------------------------
#pragma once
#include "option_data.h"
#include "regression.h"
#include "black_pricer.h"
#include <vector>

struct GreekRow {
    int64_t quote_ts;
    int64_t expiry_ts;
    double  strike;

    double delta_call, delta_put;
    double gamma;
    double vega;
    double theta_call, theta_put;
};

// Charge le csv, renvoie toutes les quotes + régressions (passe-plat pratique)
bool load_dataset(const char* csv_path,
    std::vector<OptionChainQuote>& quotes_out,
    ForwardRegressionResult& fregr_out);

// Calcule l’ensemble des greeks (sans print) et les renvoie
std::vector<GreekRow> compute_greeks(const std::vector<OptionChainQuote>& quotes,
    const ForwardRegressionResult& fregr,
    double epsilon = 1e-8);
