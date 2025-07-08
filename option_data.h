#pragma once
#include <stdio.h>     // FILE
#include <stdint.h>    // int64_t / uint32_t

/* ────────────────────────────────────────────────────────────── */
/*              Structures & protos : linkage C                  */
/* ────────────────────────────────────────────────────────────── */
#ifdef __cplusplus
extern "C" {
#endif

    /* ─── Une ligne du CSV ───────────────────────────────────────── */
    typedef struct OptionChainQuote
    {
        /* horodatage / métadonnées */
        int64_t quote_unixtime;
        char    quote_readtime[32];
        char    quote_date[16];
        double  quote_time_hours;

        /* sous-jacent */
        double  underlying_last;

        /* expiration */
        char    expire_date[16];
        int64_t expire_unixtime;
        double  dte;                     /* days-to-expiry */

        /* call greeks & iv */
        double c_delta;
        double c_gamma;
        double c_vega;
        double c_theta;
        double c_rho;
        double c_iv;
        int    c_volume;

        /* call prix & carnet */
        double c_last;
        char   c_size[16];
        double c_bid;
        double c_ask;

        /* strike (commun) */
        double strike;

        /* put carnet */
        double p_bid;
        double p_ask;
        char   p_size[16];

        /* put prix & greeks */
        double p_last;
        double p_delta;
        double p_gamma;
        double p_vega;
        double p_theta;
        double p_rho;
        double p_iv;
        int    p_volume;

        /* métriques dérivées */
        double strike_distance;
        double strike_distance_pct;
        double call_put_parity_criterion;
        double c_mid;
        double p_mid;
        double c_bidaskspread;
        double p_bidaskspread;
        double yte;
        double calc;
        int    volume;
        double forward;
        double log_moneyness;
        double fwd_pct;
        double log_money_fwd_pct;
    } OptionChainQuote;


    /* ─── API CSV ─────────────────────────────────────────────────── */
    int  get_csv_rows_count(FILE* file);
    void fill_all_option_chain_quotes(OptionChainQuote* quotes,
        int rows_to_fill,
        FILE* file);

#ifdef __cplusplus
} /* extern "C" */
#endif
