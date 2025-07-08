/************************************
 * LearnCpp.cpp — v2 “verbose/bench”
 * ----------------------------------
 *  • benchmark = false → impression
 *    des 4 tableaux détaillés.
 *  • benchmark = true  → calcul
 *    silencieux + chrono final.
 *
 *  Change la ligne :
 *      const bool benchmark = false;
 ************************************/

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

 /* ======= RÉGLAGE GLOBAL ======= */
const bool  benchmark = true;   // <-- mets true pour le chrono seul
const char* CSV_PATH =
R"(C:\Users\paula\source\repos\LearnCpp\raw_data.csv)";
const double EPS = 1e-8;
/* ============================== */

static void table1(const ForwardRegressionResult& reg)
{
    puts("\n=== TABLE 1: Forward and Discount Factor per Expiry ===");
    printf("%-15s | %-12s | %-10s | %-10s | %-10s | %-6s\n",
        "Expiry", "Intercept", "Slope", "R^2", "RMSE", "Count");
    puts("-------------------------------------------------------------------------------");
    for (size_t i = 0; i < reg.count; ++i)
        printf("%-15lld | %-12.2f | %-10.6f | %-10.6f | %-10.6f | %-6d\n",
            reg.expiries[i],
            reg.regressions[i].intercept,
            reg.regressions[i].slope,
            reg.regressions[i].r_squared,
            reg.regressions[i].rmse,
            reg.regressions[i].count);
}

static void table2_header()
{
    puts("\n=== TABLE 2: Pricing Errors ===");
    printf("%-15s | %-15s | %-8s | %-10s | %-10s | %-9s | %-10s | %-10s | %-9s\n",
        "QuoteTime", "Expiry", "Strike",
        "Call_Mid", "Call_Theo", "Call_Err",
        "Put_Mid", "Put_Theo", "Put_Err");
    puts("------------------------------------------------------------------------------------------------------------------");
}

static void table3_header()
{
    puts("\n=== TABLE 3: Greeks ===");
    printf("%-15s | %-15s | %-8s | %-8s | %-8s | %-8s | %-10s | %-10s | %-10s\n",
        "QuoteTime", "Expiry", "Strike",
        "Δ_Call", "Δ_Put", "Γ",
        "Vega", "Θ_Call", "Θ_Put");
    puts("------------------------------------------------------------------------------------------------------------------");
}

static void table4_header()
{
    puts("\n=== TABLE 4: Theo vs CSV Greeks ===");
    printf("%-15s | %-15s | %-8s | %-10s | %-10s | %-6s | %-10s | %-10s | %-10s | %-10s\n",
        "QuoteTime", "Expiry", "Strike",
        "Fwd", "ln(K/F)", "Greek",
        "Theo_Call", "CSV_Call", "Theo_Put", "CSV_Put");
    puts("-----------------------------------------------------------------------------------------------------------------------------");
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    /* ---------- chargement CSV ---------- */
    clock_t start = clock();
    FILE* file = fopen(CSV_PATH, "r");
    if (!file) { perror("fopen csv"); return 1; }

    int rows = get_csv_rows_count(file);
    OptionChainQuote* quotes =
        (OptionChainQuote*)malloc(rows * sizeof(OptionChainQuote));
    if (!quotes) { perror("malloc"); fclose(file); return 1; }

    fill_all_option_chain_quotes(quotes, rows - 1, file);
    fclose(file);

    /* ---------- régressions F & DF ---------- */
    ForwardRegressionResult reg =
        compute_forward_regressions(quotes, rows);

    if (!benchmark) table1(reg);
    if (!benchmark) table2_header();

    /* ---------- TABLE 2 ---------- */
    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->c_iv <= 0.0 && q->p_iv <= 0.0) continue;

        LinearRegressionStats* s = nullptr;
        for (size_t j = 0; j < reg.count; ++j)
            if (reg.expiries[j] == q->expire_unixtime) { s = &reg.regressions[j]; break; }
        if (!s) continue;

        double std_call = q->c_iv * sqrt(s->maturity);
        double std_put = q->p_iv * sqrt(s->maturity);

        double call_theo = black_scholes_price(s->forward, q->strike, std_call,
            s->discount_factor, 1);
        double put_theo = black_scholes_price(s->forward, q->strike, std_put,
            s->discount_factor, 0);

        if (!benchmark)
            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-9.4f | %-10.4f | %-10.4f | %-9.4f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                q->c_mid, call_theo, fabs(call_theo - q->c_mid),
                q->p_mid, put_theo, fabs(put_theo - q->p_mid));
    }

    /* ---------- TABLE 3 ---------- */
    if (!benchmark) { table3_header(); }
    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->dte <= 0.0) continue;

        LinearRegressionStats* s = nullptr;
        for (size_t j = 0; j < reg.count; ++j)
            if (reg.expiries[j] == q->expire_unixtime) { s = &reg.regressions[j]; break; }
        if (!s) continue;

        double std_call = fmax(EPS, q->c_iv * sqrt(s->maturity));
        double std_put = fmax(EPS, q->p_iv * sqrt(s->maturity));

        double dC = black_scholes_delta(s->forward, q->strike, std_call,
            s->discount_factor, 1);
        double dP = black_scholes_delta(s->forward, q->strike, std_put,
            s->discount_factor, 0);
        double g = black_scholes_gamma(s->forward, q->strike, std_call,
            s->discount_factor);
        double v = black_scholes_vega(s->forward, q->strike, std_call,
            s->discount_factor, s->maturity);
        double tC = black_scholes_theta(s->forward, q->strike, q->c_iv,
            s->discount_factor, s->maturity, 1);
        double tP = black_scholes_theta(s->forward, q->strike, q->p_iv,
            s->discount_factor, s->maturity, 0);

        if (!benchmark)
            printf("%-15lld | %-15lld | %-8.2f | %-8.4f | %-8.4f | %-8.6f | %-10.4f | %-10.2f | %-10.2f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                dC, dP, g, v, tC, tP);
    }

    /* ---------- TABLE 4 ---------- */
    if (!benchmark) { table4_header(); }
    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->dte <= 0.0) continue;

        LinearRegressionStats* s = nullptr;
        for (size_t j = 0; j < reg.count; ++j)
            if (reg.expiries[j] == q->expire_unixtime) { s = &reg.regressions[j]; break; }
        if (!s || s->forward <= EPS || q->strike <= EPS) continue;

        double ln_k_f = log(q->strike / s->forward);
        double std_call = fmax(EPS, q->c_iv * sqrt(s->maturity));
        double std_put = fmax(EPS, q->p_iv * sqrt(s->maturity));

        double dC = black_scholes_delta(s->forward, q->strike, std_call, s->discount_factor, 1);
        double dP = black_scholes_delta(s->forward, q->strike, std_put, s->discount_factor, 0);
        double g = black_scholes_gamma(s->forward, q->strike, std_call, s->discount_factor);
        double v = black_scholes_vega(s->forward, q->strike, std_call, s->discount_factor, s->maturity);
        double tC = black_scholes_theta(s->forward, q->strike, q->c_iv, s->discount_factor, s->maturity, 1);
        double tP = black_scholes_theta(s->forward, q->strike, q->p_iv, s->discount_factor, s->maturity, 0);

        if (!benchmark)
        {
            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.6f | %-10.6f | %-10.6f | %-10.6f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                s->forward, ln_k_f, "Delta",
                dC, q->c_delta, dP, q->p_delta);

            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.8f | %-10.8f | %-10.8f | %-10.8f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                s->forward, ln_k_f, "Gamma",
                g, q->c_gamma, g, q->p_gamma);

            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.2f | %-10.2f | %-10.2f | %-10.2f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                s->forward, ln_k_f, "Vega",
                v, q->c_vega, v, q->p_vega);

            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-6s | %-10.8f | %-10.8f | %-10.8f | %-10.8f\n",
                -q->quote_unixtime, q->expire_unixtime, q->strike,
                -s->forward, ln_k_f, "Theta",
                -tC, q->c_theta,   //  <-- ancien ordre (faux CSV)
                -tP, q->p_theta);
            // inversion des theta parce que le csv n'est pas bon
        }
    }

    /* ---------- chrono final ---------- */
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("\nTotal execution time: %.6f seconds%s\n",
        elapsed, benchmark ? "  (benchmark mode)" : "");

    free(quotes);
    free(reg.expiries);
    free(reg.regressions);
    return 0;
}
