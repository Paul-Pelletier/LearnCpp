#pragma once
#ifndef OPTION_DATA_H
#define OPTION_DATA_H

#include <stdio.h>
#include <stdint.h>

struct OptionChainQuote {
    int64_t quote_unixtime;
    char quote_readtime[20];
    char quote_date[12];
    double quote_time_hours;
    double underlying_last;
    char expire_date[12];
    int64_t expire_unixtime;
    double dte;
    double c_delta, c_gamma, c_vega, c_theta, c_rho, c_iv;
    int c_volume;
    double c_last;
    char c_size[16];
    double c_bid, c_ask;
    double strike;
    double p_bid, p_ask;
    char p_size[16];
    double p_last;
    double p_delta, p_gamma, p_vega, p_theta, p_rho, p_iv;
    int p_volume;
    double strike_distance, strike_distance_pct;
    double call_put_parity_criterion;
    double c_mid, p_mid;
    double c_bidaskspread, p_bidaskspread;
    double yte, calc;
    int volume;
    double forward, log_moneyness, fwd_pct, log_money_fwd_pct;
};

int get_csv_rows_count(FILE* file);
double parse_double(const char* s);
void fill_all_option_chain_quotes(struct OptionChainQuote* quotes, int rowsToFill, FILE* file);

#endif
