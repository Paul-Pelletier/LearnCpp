#define _CRT_SECURE_NO_WARNINGS
#include "option_data.h"
#include <string.h>
#include <stdlib.h>

int get_csv_rows_count(FILE* file) {
    rewind(file);
    int result = 0;
    int c = 0;
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') result++;
    }
    rewind(file);
    return result;
}

double parse_double(const char* s) {
    char tmp[64];
    strncpy(tmp, s, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';
    for (char* p = tmp; *p; ++p) {
        if (*p == ',') *p = '.';
    }
    return atof(tmp);
}

void fill_all_option_chain_quotes(struct OptionChainQuote* quotes, int rowsToFill, FILE* file) {
    rewind(file);
    char buffer[10000];
    fgets(buffer, sizeof(buffer), file); // Skip header

    int rowCounter = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && rowCounter < rowsToFill) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        char* token = strtok(buffer, ";");
        int field = 0;

        while (token != NULL) {
            switch (field) {
            case 0: quotes[rowCounter].quote_unixtime = strtoll(token, NULL, 10); break;
            case 1: strncpy(quotes[rowCounter].quote_readtime, token, sizeof(quotes[rowCounter].quote_readtime)); break;
            case 2: strncpy(quotes[rowCounter].quote_date, token, sizeof(quotes[rowCounter].quote_date)); break;
            case 3: quotes[rowCounter].quote_time_hours = parse_double(token); break;
            case 4: quotes[rowCounter].underlying_last = parse_double(token); break;
            case 5: strncpy(quotes[rowCounter].expire_date, token, sizeof(quotes[rowCounter].expire_date)); break;
            case 6: quotes[rowCounter].expire_unixtime = strtoll(token, NULL, 10); break;
            case 7: quotes[rowCounter].dte = parse_double(token); break;
            case 8: quotes[rowCounter].c_delta = parse_double(token); break;
            case 9: quotes[rowCounter].c_gamma = parse_double(token); break;
            case 10: quotes[rowCounter].c_vega = parse_double(token); break;
            case 11: quotes[rowCounter].c_theta = parse_double(token); break;
            case 12: quotes[rowCounter].c_rho = parse_double(token); break;
            case 13: quotes[rowCounter].c_iv = parse_double(token); break;
            case 14: quotes[rowCounter].c_volume = atoi(token); break;
            case 15: quotes[rowCounter].c_last = parse_double(token); break;
            case 16: strncpy(quotes[rowCounter].c_size, token, sizeof(quotes[rowCounter].c_size)); break;
            case 17: quotes[rowCounter].c_bid = parse_double(token); break;
            case 18: quotes[rowCounter].c_ask = parse_double(token); break;
            case 19: quotes[rowCounter].strike = parse_double(token); break;
            case 20: quotes[rowCounter].p_bid = parse_double(token); break;
            case 21: quotes[rowCounter].p_ask = parse_double(token); break;
            case 22: strncpy(quotes[rowCounter].p_size, token, sizeof(quotes[rowCounter].p_size)); break;
            case 23: quotes[rowCounter].p_last = parse_double(token); break;
            case 24: quotes[rowCounter].p_delta = parse_double(token); break;
            case 25: quotes[rowCounter].p_gamma = parse_double(token); break;
            case 26: quotes[rowCounter].p_vega = parse_double(token); break;
            case 27: quotes[rowCounter].p_theta = parse_double(token); break;
            case 28: quotes[rowCounter].p_rho = parse_double(token); break;
            case 29: quotes[rowCounter].p_iv = parse_double(token); break;
            case 30: quotes[rowCounter].p_volume = atoi(token); break;
            case 31: quotes[rowCounter].strike_distance = parse_double(token); break;
            case 32: quotes[rowCounter].strike_distance_pct = parse_double(token); break;
            case 33: quotes[rowCounter].call_put_parity_criterion = parse_double(token); break;
            case 34: quotes[rowCounter].c_mid = parse_double(token); break;
            case 35: quotes[rowCounter].p_mid = parse_double(token); break;
            case 36: quotes[rowCounter].c_bidaskspread = parse_double(token); break;
            case 37: quotes[rowCounter].p_bidaskspread = parse_double(token); break;
            case 38: quotes[rowCounter].yte = parse_double(token); break;
            case 39: quotes[rowCounter].calc = parse_double(token); break;
            case 40: quotes[rowCounter].volume = atoi(token); break;
            case 41: quotes[rowCounter].forward = parse_double(token); break;
            case 42: quotes[rowCounter].log_moneyness = parse_double(token); break;
            case 43: quotes[rowCounter].fwd_pct = parse_double(token); break;
            case 44: quotes[rowCounter].log_money_fwd_pct = parse_double(token); break;
            default: break;
            }
            field++;
            token = strtok(NULL, ";");
        }

        rowCounter++;
    }
    rewind(file);
}
