// greeks_engine.cpp  
#define _CRT_SECURE_NO_WARNINGS
#include "greeks_engine.h"
#include <chrono>
#include <set>
#include <cstring>     // memcpy

//-------------------------------------------------------------------
// I/O – lecture CSV (reprend votre implémentation telle quelle)
//-------------------------------------------------------------------
bool load_dataset(const char* csv_path,
    std::vector<OptionChainQuote>& quotes_out,
    ForwardRegressionResult& fregr_out)
{
    FILE* f = fopen(csv_path, "r");
    if (!f) return false;

    int n = get_csv_rows_count(f);
    quotes_out.resize(static_cast<size_t>(n));

    fill_all_option_chain_quotes(quotes_out.data(), n - 1, f);
    fclose(f);

    // correctif fournisseur : inversion theta
    for (auto& q : quotes_out) std::swap(q.c_theta, q.p_theta);

    fregr_out = compute_forward_regressions(quotes_out.data(), n);
    return true;
}

//-------------------------------------------------------------------
// Calcul brut des greeks (0 printf, 0 allocation dans la boucle)
//-------------------------------------------------------------------
std::vector<GreekRow> compute_greeks(const std::vector<OptionChainQuote>& q,
    const ForwardRegressionResult& fr,
    double eps)
{
    std::vector<GreekRow> out;
    out.reserve(q.size());

    for (const auto& quote : q)
    {
        if (quote.dte <= 0.0) continue;

        // Cherche la régression associée
        const LinearRegressionStats* stats = nullptr;
        for (size_t j = 0; j < fr.count; ++j)
            if (fr.expiries[j] == quote.expire_unixtime) {
                stats = &fr.regressions[j]; break;
            }
        if (!stats) continue;

        double vol_c = std::max(eps, quote.c_iv * sqrt(stats->maturity));
        double vol_p = std::max(eps, quote.p_iv * sqrt(stats->maturity));

        GreekRow row{};
        row.quote_ts = quote.quote_unixtime;
        row.expiry_ts = quote.expire_unixtime;
        row.strike = quote.strike;

        row.delta_call = black_scholes_delta(stats->forward, quote.strike,
            vol_c, stats->discount_factor, 1);
        row.delta_put = black_scholes_delta(stats->forward, quote.strike,
            vol_p, stats->discount_factor, 0);
        row.gamma = black_scholes_gamma(stats->forward, quote.strike,
            vol_c, stats->discount_factor);
        row.vega = black_scholes_vega(stats->forward, quote.strike,
            vol_c, stats->discount_factor,
            stats->maturity);
        row.theta_call = black_scholes_theta(stats->forward, quote.strike,
            quote.c_iv, stats->discount_factor,
            stats->maturity, 1);
        row.theta_put = black_scholes_theta(stats->forward, quote.strike,
            quote.p_iv, stats->discount_factor,
            stats->maturity, 0);

        out.push_back(row);
    }
    return out;
}
