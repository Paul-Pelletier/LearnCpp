#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include <math.h>

#include "option_data.h"
#include "regression.h"
#include "black_pricer.h"

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);

    const double epsilon = 1e-8;
    const char* path = "C:\\Users\\paula\\source\\repos\\LearnCpp\\raw_data.csv";

    clock_t start = clock();

    /* ------------------------------------------------------------------ */
    /*  Lecture CSV                                                       */
    /* ------------------------------------------------------------------ */
    FILE* file = fopen(path, "r");
    if (!file) { perror("Erreur d'ouverture du fichier"); return 1; }

    int rowsCount = get_csv_rows_count(file);
    struct OptionChainQuote* quotes =
        (struct OptionChainQuote*)malloc(rowsCount * sizeof(struct OptionChainQuote));
    if (!quotes) { perror("Erreur alloc"); fclose(file); return 1; }

    fill_all_option_chain_quotes(quotes, rowsCount - 1, file);
    fclose(file);

    /* ------------------------------------------------------------------
       Correctif : le fournisseur a inversé θ_call / θ_put dans le CSV   */
    for (int k = 0; k < rowsCount; ++k) {
        double tmp = quotes[k].c_theta;  /* lu comme call */
        quotes[k].c_theta = quotes[k].p_theta; /* remet le vrai call  */
        quotes[k].p_theta = tmp;              /* remet le vrai put   */
    }
    /* ------------------------------------------------------------------ */

    struct ForwardRegressionResult regressionResult =
        compute_forward_regressions(quotes, rowsCount);

    /* =======================  TABLE 1  =============================== */
    printf("\n=== TABLE 1: Forward and Discount Factor per Expiry ===\n");
    printf("%-15s | %-12s | %-10s | %-10s | %-10s | %-6s\n",
        "Expiry", "Intercept", "Slope", "R^2", "RMSE", "Count");
    printf("-------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < regressionResult.count; ++i)
        printf("%-15lld | %-12.2f | %-10.6f | %-10.6f | %-10.6f | %-6d\n",
            regressionResult.expiries[i],
            regressionResult.regressions[i].intercept,
            regressionResult.regressions[i].slope,
            regressionResult.regressions[i].r_squared,
            regressionResult.regressions[i].rmse,
            regressionResult.regressions[i].count);

    /* =======================  TABLE 2  =============================== */
    printf("\n=== TABLE 2: Pricing Errors ===\n");
    printf("%-15s | %-15s | %-8s | %-10s | %-10s | %-9s | %-10s | %-10s | %-9s\n",
        "QuoteTime", "Expiry", "Strike",
        "Call_Mid", "Call_Theo", "Call_Err",
        "Put_Mid", "Put_Theo", "Put_Err");
    printf("----------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < rowsCount; ++i) {
        struct OptionChainQuote* q = &quotes[i];
        if (q->c_iv <= 0.0 || q->p_iv <= 0.0 || q->dte <= 0.0) continue;

        struct LinearRegressionStats* stats = NULL;
        for (size_t j = 0; j < regressionResult.count; ++j)
            if (regressionResult.expiries[j] == q->expire_unixtime) { stats = &regressionResult.regressions[j]; break; }
        if (!stats) continue;

        double stddev_c = q->c_iv * sqrt(stats->maturity);
        double stddev_p = q->p_iv * sqrt(stats->maturity);

        double call_theo = black_scholes_price(stats->forward, q->strike, stddev_c,
            stats->discount_factor, 1);
        double put_theo = black_scholes_price(stats->forward, q->strike, stddev_p,
            stats->discount_factor, 0);

        double call_err = fabs(call_theo - q->c_mid);
        double put_err = fabs(put_theo - q->p_mid);

        if ((q->c_mid > 0.0 && call_err < 10.0) || (q->p_mid > 0.0 && put_err < 10.0))
            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-9.4f | %-10.4f | %-10.4f | %-9.4f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                q->c_mid, call_theo, call_err,
                q->p_mid, put_theo, put_err);
    }

    /* =======================  TABLE 3  =============================== */
    printf("\n=== TABLE 3: Greeks ===\n");
    printf("%-15s | %-15s | %-8s | %-8s | %-8s | %-8s | %-10s | %-10s | %-10s\n",
        "QuoteTime", "Expiry", "Strike",
        "Δ_Call", "Δ_Put", "Γ", "Vega", "Θ_Call", "Θ_Put");
    printf("----------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < rowsCount; ++i) {
        struct OptionChainQuote* q = &quotes[i];

        struct LinearRegressionStats* stats = NULL;
        for (size_t j = 0; j < regressionResult.count; ++j)
            if (regressionResult.expiries[j] == q->expire_unixtime) { stats = &regressionResult.regressions[j]; break; }
        if (!stats || q->dte <= 0.0) continue;

        double stddev_c = fmax(epsilon, q->c_iv * sqrt(stats->maturity));
        double stddev_p = fmax(epsilon, q->p_iv * sqrt(stats->maturity));

        double delta_c = black_scholes_delta(stats->forward, q->strike, stddev_c,
            stats->discount_factor, 1);
        double delta_p = black_scholes_delta(stats->forward, q->strike, stddev_p,
            stats->discount_factor, 0);
        double gamma = black_scholes_gamma(stats->forward, q->strike, stddev_c,
            stats->discount_factor);
        double vega = black_scholes_vega(stats->forward, q->strike, stddev_c,
            stats->discount_factor, stats->maturity);
        double theta_c = black_scholes_theta(stats->forward, q->strike, q->c_iv,
            stats->discount_factor, stats->maturity, 1);
        double theta_p = black_scholes_theta(stats->forward, q->strike, q->p_iv,
            stats->discount_factor, stats->maturity, 0);

        printf("%-15lld | %-15lld | %-8.2f | %-8.4f | %-8.4f | %-8.6f | %-10.4f | %-10.2f | %-10.2f\n",
            q->quote_unixtime, q->expire_unixtime, q->strike,
            delta_c, delta_p, gamma, vega, theta_c, theta_p);
    }

    /* =======================  TABLE 4  =============================== */
    printf("\n=== TABLE 4: Theo vs CSV Greeks ===\n");
    printf("%-15s | %-15s | %-8s | %-10s | %-10s | %-6s | %-10s | %-10s | %-10s | %-10s\n",
        "QuoteTime", "Expiry", "Strike",
        "Fwd", "ln(K/F)", "Greek",
        "Theo_Call", "CSV_Call", "Theo_Put", "CSV_Put");
    printf("-----------------------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < rowsCount; ++i) {
        struct OptionChainQuote* q = &quotes[i];
        if (q->dte <= 0.0) continue;

        struct LinearRegressionStats* stats = NULL;
        for (size_t j = 0; j < regressionResult.count; ++j)
            if (regressionResult.expiries[j] == q->expire_unixtime) { stats = &regressionResult.regressions[j]; break; }
        if (!stats) continue;

        double ln_kf = log(q->strike / stats->forward);
        double vol_c = fmax(epsilon, q->c_iv * sqrt(stats->maturity));
        double vol_p = fmax(epsilon, q->p_iv * sqrt(stats->maturity));

        double delta_c = black_scholes_delta(stats->forward, q->strike, vol_c, stats->discount_factor, 1);
        double delta_p = black_scholes_delta(stats->forward, q->strike, vol_p, stats->discount_factor, 0);
        double gamma = black_scholes_gamma(stats->forward, q->strike, vol_c, stats->discount_factor);
        double vega = black_scholes_vega(stats->forward, q->strike, vol_c, stats->discount_factor, stats->maturity);
        double theta_c = black_scholes_theta(stats->forward, q->strike, q->c_iv, stats->discount_factor, stats->maturity, 1);
        double theta_p = black_scholes_theta(stats->forward, q->strike, q->p_iv, stats->discount_factor, stats->maturity, 0);

        /* Delta */
        printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.5f | %-10.5f | %-10.5f | %-10.5f\n",
            q->quote_unixtime, q->expire_unixtime, q->strike,
            stats->forward, ln_kf, "Delta",
            delta_c, q->c_delta, delta_p, q->p_delta);
        /* Gamma */
        printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.8f | %-10.8f | %-10.8f | %-10.8f\n",
            q->quote_unixtime, q->expire_unixtime, q->strike,
            stats->forward, ln_kf, "Gamma",
            gamma, q->c_gamma, gamma, q->p_gamma);
        /* Vega */
        printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.2f | %-10.2f | %-10.2f | %-10.2f\n",
            q->quote_unixtime, q->expire_unixtime, q->strike,
            stats->forward, ln_kf, "Vega",
            vega, q->c_vega, vega, q->p_vega);
        /* Theta */
        printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.8f | %-10.8f | %-10.8f | %-10.8f\n",
            q->quote_unixtime, q->expire_unixtime, q->strike,
            stats->forward, ln_kf, "Theta",
            theta_c, q->c_theta, theta_p, q->p_theta);
    }

    /* ------------------------------------------------------------------ */
    clock_t end = clock();
    printf("\nTotal execution time: %.6f seconds\n",
        (double)(end - start) / CLOCKS_PER_SEC);

    free(quotes);
    free(regressionResult.expiries);
    free(regressionResult.regressions);
    return 0;
}
