/************************************
 * LearnCpp.cpp — v3 “verbose/bench”
 * ----------------------------------
 *  • benchmark = false → impression
 *    des 4 tableaux (diagnostics).
 *  • benchmark = true  → exécution
 *    silencieuse, seul le chrono.
 *
 *  • USE_CSV_LOADER = true  → parseur
 *    vincentlaucsb/csv-parser (C++)
 *  • USE_CSV_LOADER = false → ancien
 *    parseur C avec fgets/strtok.
 *
 *  Change simplement les deux
 *  constantes ci-dessous puis F5.
 ************************************/

#define _CRT_SECURE_NO_WARNINGS        // MSVC : désactive warnings « unsafe »

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include <math.h>

#include <vector>                      // std::vector  (CSV loader)
#include <string_view>                 // R"(…)" path

#include "option_data.h"
#include "regression.h"
#include "black_pricer.h"
#include "csv_loader.h"                // load_quotes_csv (si activé)
#include <thread>          //  <-- AJOUTE


 /* ===== RÉGLAGES GLOBAUX ===================================== */
constexpr bool   benchmark = true;   // ← true : chrono seul
constexpr bool   USE_CSV_LOADER = true;   // ← passe à false pour l’ancien parseur
constexpr char   CSV_PATH[] =
R"(C:\Users\paula\source\repos\LearnCpp\raw_data1.csv)";
constexpr double EPS = 1e-8;
/* ============================================================ */


/* ────────── Helpers d’affichage des en-têtes ─────────────── */
static void table1_header()
{
    puts("\n=== TABLE 1: Forward and Discount Factor per Expiry ===");
    printf("%-15s | %-12s | %-10s | %-10s | %-10s | %-6s\n",
        "Expiry", "Intercept", "Slope", "R^2", "RMSE", "Count");
    puts("-------------------------------------------------------------------------------");
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
        "Δ_Call", "Δ_Put", "Γ", "Vega", "Θ_Call", "Θ_Put");
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
/* ──────────────────────────────────────────────────────────── */


int main()
{
    SetConsoleOutputCP(CP_UTF8);

    /************ 1) Chargement du CSV *************************/
    clock_t start = clock();

    std::vector<OptionChainQuote> vecQuotes;   // utilisé si CSV loader
    OptionChainQuote* quotes = nullptr;   // pointeur commun
    int rows = 0;
    bool quotes_from_malloc = false;     // pour savoir s’il faut free()

    if (USE_CSV_LOADER)
    {
        if (!load_quotes_csv(CSV_PATH, vecQuotes))
        {
            fprintf(stderr, "Erreur load_quotes_csv()\n");
            return 1;
        }
        rows = static_cast<int>(vecQuotes.size());
        quotes = vecQuotes.data();             // mémoire gérée par le vector
    }
    else
    {
        FILE* file = fopen(CSV_PATH, "r");
        if (!file) { perror("fopen CSV"); return 1; }

        rows = get_csv_rows_count(file);
        quotes = static_cast<OptionChainQuote*>(
            malloc(rows * sizeof(OptionChainQuote)));
        if (!quotes) { perror("malloc"); fclose(file); return 1; }

        fill_all_option_chain_quotes(quotes, rows - 1, file);
        fclose(file);
        quotes_from_malloc = true;             // on devra free(quotes)
    }

    /************ 2) Régression Forward / DF *******************/
    ForwardRegressionResult reg = compute_forward_regressions(quotes, rows);

    if (!benchmark)
    {
        table1_header();
        for (size_t i = 0; i < reg.count; ++i)
            printf("%-15lld | %-12.2f | %-10.6f | %-10.6f | %-10.6f | %-6d\n",
                reg.expiries[i],
                reg.regressions[i].intercept,
                reg.regressions[i].slope,
                reg.regressions[i].r_squared,
                reg.regressions[i].rmse,
                reg.regressions[i].count);
        table2_header();
    }

    /************ 3) TABLE 2 : erreurs de pricing **************/
    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->c_iv <= 0.0 && q->p_iv <= 0.0) continue;

        const LinearRegressionStats* s = nullptr;
        for (size_t j = 0; j < reg.count; ++j)
            if (reg.expiries[j] == q->expire_unixtime) { s = &reg.regressions[j]; break; }
        if (!s) continue;

        double std_call = q->c_iv * sqrt(s->maturity);
        double std_put = q->p_iv * sqrt(s->maturity);

        double call_theo = black_scholes_price(
            s->forward, q->strike, std_call, s->discount_factor, /*call*/1);
        double put_theo = black_scholes_price(
            s->forward, q->strike, std_put, s->discount_factor, /*put */0);

        if (!benchmark)
            printf("%-15lld | %-15lld | %-8.2f | %-10.4f | %-10.4f | %-9.4f | %-10.4f | %-10.4f | %-9.4f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                q->c_mid, call_theo, fabs(call_theo - q->c_mid),
                q->p_mid, put_theo, fabs(put_theo - q->p_mid));
    }

    /************ 4) TABLE 3 : grecs ***************************/
    if (!benchmark) table3_header();

    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->dte <= 0.0) continue;

        const LinearRegressionStats* s = nullptr;
        for (size_t j = 0; j < reg.count; ++j)
            if (reg.expiries[j] == q->expire_unixtime) { s = &reg.regressions[j]; break; }
        if (!s) continue;

        double std_call = fmax(EPS, q->c_iv * sqrt(s->maturity));
        double std_put = fmax(EPS, q->p_iv * sqrt(s->maturity));

        double dC = black_scholes_delta(s->forward, q->strike, std_call, s->discount_factor, 1);
        double dP = black_scholes_delta(s->forward, q->strike, std_put, s->discount_factor, 0);
        double g = black_scholes_gamma(s->forward, q->strike, std_call, s->discount_factor);
        double v = black_scholes_vega(s->forward, q->strike, std_call, s->discount_factor, s->maturity);
        double tC = black_scholes_theta(s->forward, q->strike, q->c_iv, s->discount_factor, s->maturity, 1);
        double tP = black_scholes_theta(s->forward, q->strike, q->p_iv, s->discount_factor, s->maturity, 0);

        if (!benchmark)
            printf("%-15lld | %-15lld | %-8.2f | %-8.4f | %-8.4f | %-8.6f | %-10.4f | %-10.2f | %-10.2f\n",
                q->quote_unixtime, q->expire_unixtime, q->strike,
                dC, dP, g, v, tC, tP);
    }

    /************ 5) TABLE 4 : comparaison CSV ******************/
    if (!benchmark) table4_header();

    for (int i = 0; i < rows; ++i)
    {
        OptionChainQuote* q = &quotes[i];
        if (q->dte <= 0.0) continue;

        const LinearRegressionStats* s = nullptr;
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
                q->quote_unixtime, q->expire_unixtime, q->strike,
                s->forward, ln_k_f, "Theta",
                tC, q->c_theta, tP, q->p_theta);
        }
    }

    /************ 6) Chronométrage global ***********************/
    double elapsed = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
    printf("\nTotal execution time: %.6f seconds%s\n",
        elapsed, benchmark ? "  (benchmark mode)" : "");

    /* Libération : seulement si la mémoire vient de malloc()   */
    if (quotes_from_malloc)
        free(quotes);

    free(reg.expiries);
    free(reg.regressions);
    return 0;
}
