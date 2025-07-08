#define _CRT_SECURE_NO_WARNINGS            /* pour MSVC : désactive les warnings "unsafe" */
#include "option_data.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _MSC_VER
#define COPY_STR(dst, src) strncpy_s((dst), sizeof(dst), (src), _TRUNCATE)
#else
#define COPY_STR(dst, src) do {                       \
        strncpy((dst), (src), sizeof(dst)-1);           \
        (dst)[sizeof(dst)-1] = '\0';                    \
  } while(0)
#endif

/* ────────────────────────────────────────────────────────────── */
/*                      utilitaires internes                     */
/* ────────────────────────────────────────────────────────────── */
static double parse_double(const char* s)
{
    char tmp[64];
    size_t n = strlen(s);
    if (n >= sizeof(tmp)) n = sizeof(tmp) - 1;
    memcpy(tmp, s, n);
    tmp[n] = '\0';

    /* remplace les virgules décimales éventuelles */
    for (char* p = tmp; *p; ++p)
        if (*p == ',') *p = '.';

    return atof(tmp);
}

/* ────────────────────────────────────────────────────────────── */
int get_csv_rows_count(FILE* file)
{
    rewind(file);
    int rows = 0, c;
    while ((c = fgetc(file)) != EOF)
        if (c == '\n') ++rows;
    rewind(file);
    return rows;
}

/* ────────────────────────────────────────────────────────────── */
void fill_all_option_chain_quotes(OptionChainQuote* q,
    int rows_to_fill,
    FILE* file)
{
    rewind(file);

    char buffer[10000];
    fgets(buffer, sizeof(buffer), file);            /* saute l'entête */

    int row = 0;
    while (row < rows_to_fill && fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\r\n")] = 0;        /* strip EOL */

        char* token = strtok(buffer, ";");
        int   field = 0;

        while (token)
        {
            switch (field)
            {
            case  0: q[row].quote_unixtime = strtoll(token, NULL, 10); break;
            case  1: COPY_STR(q[row].quote_readtime, token);          break;
            case  2: COPY_STR(q[row].quote_date, token);              break;
            case  3: q[row].quote_time_hours = parse_double(token);   break;
            case  4: q[row].underlying_last = parse_double(token);    break;
            case  5: COPY_STR(q[row].expire_date, token);             break;
            case  6: q[row].expire_unixtime = strtoll(token, NULL, 10); break;
            case  7: q[row].dte = parse_double(token);                break;
            case  8: q[row].c_delta = parse_double(token);            break;
            case  9: q[row].c_gamma = parse_double(token);            break;
            case 10: q[row].c_vega = parse_double(token);            break;
            case 11: q[row].c_theta = parse_double(token);            break;
            case 12: q[row].c_rho = parse_double(token);            break;
            case 13: q[row].c_iv = parse_double(token);            break;
            case 14: q[row].c_volume = atoi(token);                    break;
            case 15: q[row].c_last = parse_double(token);            break;
            case 16: COPY_STR(q[row].c_size, token);                  break;
            case 17: q[row].c_bid = parse_double(token);            break;
            case 18: q[row].c_ask = parse_double(token);            break;
            case 19: q[row].strike = parse_double(token);            break;
            case 20: q[row].p_bid = parse_double(token);            break;
            case 21: q[row].p_ask = parse_double(token);            break;
            case 22: COPY_STR(q[row].p_size, token);                  break;
            case 23: q[row].p_last = parse_double(token);            break;
            case 24: q[row].p_delta = parse_double(token);            break;
            case 25: q[row].p_gamma = parse_double(token);            break;
            case 26: q[row].p_vega = parse_double(token);            break;
            case 27: q[row].p_theta = parse_double(token);            break;
            case 28: q[row].p_rho = parse_double(token);            break;
            case 29: q[row].p_iv = parse_double(token);            break;
            case 30: q[row].p_volume = atoi(token);                    break;
            case 31: q[row].strike_distance = parse_double(token); break;
            case 32: q[row].strike_distance_pct = parse_double(token); break;
            case 33: q[row].call_put_parity_criterion = parse_double(token); break;
            case 34: q[row].c_mid = parse_double(token);             break;
            case 35: q[row].p_mid = parse_double(token);             break;
            case 36: q[row].c_bidaskspread = parse_double(token);     break;
            case 37: q[row].p_bidaskspread = parse_double(token);     break;
            case 38: q[row].yte = parse_double(token);             break;
            case 39: q[row].calc = parse_double(token);             break;
            case 40: q[row].volume = atoi(token);                     break;
            case 41: q[row].forward = parse_double(token);             break;
            case 42: q[row].log_moneyness = parse_double(token);      break;
            case 43: q[row].fwd_pct = parse_double(token);      break;
            case 44: q[row].log_money_fwd_pct = parse_double(token);  break;
            default: break;
            }
            token = strtok(NULL, ";");
            ++field;
        }
        ++row;
    }
    rewind(file);
}
