/*************************************
 * option_data.cpp   (C++20)
 *  - parsing CSV ultra-rapide
 *  - conversions via std::from_chars
 *************************************/
#define  _CRT_SECURE_NO_WARNINGS
#include "option_data.h"

#include <charconv>      // std::from_chars
#include <string.h>      // strlen, strncpy, strtok, strcspn
#include <stdlib.h>      // FILE*, fgets, fgetc
#include <array>
#include <cerrno>        // std::errc

 /* ---------- utilitaires ----------- */

 // remplace atof et gère la virgule décimale
static double fast_atof(const char* s)
{
    std::array<char, 64> buf{};
    std::size_t i = 0;
    for (; s[i] && i < buf.size() - 1; ++i)
        buf[i] = (s[i] == ',') ? '.' : s[i];

    double value = 0.0;
    auto res = std::from_chars(buf.data(), buf.data() + i, value);
    return (res.ec == std::errc()) ? value : 0.0;
}

static inline int32_t fast_atoi(const char* s)
{
    int32_t v = 0;
    std::from_chars(s, s + strlen(s), v);
    return v;
}

static inline int64_t fast_atoll(const char* s)
{
    int64_t v = 0;
    std::from_chars(s, s + strlen(s), v);
    return v;
}

/* ---------- fonctions exportées ----------- */

int get_csv_rows_count(FILE* file)
{
    rewind(file);
    int rows = 0, c = 0;
    while ((c = fgetc(file)) != EOF)
        if (c == '\n') ++rows;
    rewind(file);
    return rows;
}

void fill_all_option_chain_quotes(OptionChainQuote* q,
    int rowsToFill,
    FILE* file)
{
    rewind(file);
    char buffer[10'000];
    fgets(buffer, sizeof(buffer), file);           // saute l’en-tête

    int row = 0;
    while (row < rowsToFill && fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        char* token = strtok(buffer, ";");
        int field = 0;

        while (token)
        {
            switch (field)
            {
            case  0: q[row].quote_unixtime = fast_atoll(token); break;
            case  1: strncpy(q[row].quote_readtime, token, sizeof q[row].quote_readtime);  break;
            case  2: strncpy(q[row].quote_date, token, sizeof q[row].quote_date);      break;
            case  3: q[row].quote_time_hours = fast_atof(token);  break;
            case  4: q[row].underlying_last = fast_atof(token);  break;
            case  5: strncpy(q[row].expire_date, token, sizeof q[row].expire_date);     break;
            case  6: q[row].expire_unixtime = fast_atoll(token); break;
            case  7: q[row].dte = fast_atof(token);  break;
            case  8: q[row].c_delta = fast_atof(token);  break;
            case  9: q[row].c_gamma = fast_atof(token);  break;
            case 10: q[row].c_vega = fast_atof(token);  break;
            case 11: q[row].c_theta = fast_atof(token);  break;
            case 12: q[row].c_rho = fast_atof(token);  break;
            case 13: q[row].c_iv = fast_atof(token);  break;
            case 14: q[row].c_volume = fast_atoi(token);  break;
            case 15: q[row].c_last = fast_atof(token);  break;
            case 16: strncpy(q[row].c_size, token, sizeof q[row].c_size);          break;
            case 17: q[row].c_bid = fast_atof(token);  break;
            case 18: q[row].c_ask = fast_atof(token);  break;
            case 19: q[row].strike = fast_atof(token);  break;
            case 20: q[row].p_bid = fast_atof(token);  break;
            case 21: q[row].p_ask = fast_atof(token);  break;
            case 22: strncpy(q[row].p_size, token, sizeof q[row].p_size);          break;
            case 23: q[row].p_last = fast_atof(token);  break;
            case 24: q[row].p_delta = fast_atof(token);  break;
            case 25: q[row].p_gamma = fast_atof(token);  break;
            case 26: q[row].p_vega = fast_atof(token);  break;
            case 27: q[row].p_theta = fast_atof(token);  break;
            case 28: q[row].p_rho = fast_atof(token);  break;
            case 29: q[row].p_iv = fast_atof(token);  break;
            case 30: q[row].p_volume = fast_atoi(token);  break;
            case 31: q[row].strike_distance = fast_atof(token);  break;
            case 32: q[row].strike_distance_pct = fast_atof(token);  break;
            case 33: q[row].call_put_parity_criterion = fast_atof(token);  break;
            case 34: q[row].c_mid = fast_atof(token);  break;
            case 35: q[row].p_mid = fast_atof(token);  break;
            case 36: q[row].c_bidaskspread = fast_atof(token);  break;
            case 37: q[row].p_bidaskspread = fast_atof(token);  break;
            case 38: q[row].yte = fast_atof(token);  break;
            case 39: q[row].calc = fast_atof(token);  break;
            case 40: q[row].volume = fast_atoi(token);  break;
            case 41: q[row].forward = fast_atof(token);  break;
            case 42: q[row].log_moneyness = fast_atof(token);  break;
            case 43: q[row].fwd_pct = fast_atof(token);  break;
            case 44: q[row].log_money_fwd_pct = fast_atof(token);  break;
            default: break;
            }
            ++field;
            token = strtok(nullptr, ";");
        }
        ++row;
    }
    rewind(file);
}
