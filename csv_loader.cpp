/* csv_loader.cpp -------------------------------------------------------- */
#define _CRT_SECURE_NO_WARNINGS
#include "csv_loader.h"
#include <vincentlaucsb-csv-parser/csv.hpp>          /* vcpkg port : vincentlaucsb-csv-parser      */
#include <cstring>          /* std::strncpy                               */
#include <charconv>         /* std::from_chars (C++20)                    */

/* Remplace la virgule décimale par un point puis std::stod -------------- */
static inline double to_double_comma_ok(const csv::CSVField& f)
{
    std::string_view sv = f.get_sv();          // string_view (no alloc)
    std::string tmp(sv);                       // petite copie sur la pile
    for (char& c : tmp) if (c == ',') c = '.';
    return std::stod(tmp);
}

bool load_quotes_csv(const std::string& path,
    std::vector<OptionChainQuote>& out_quotes)
{
    using namespace csv;                      // Reader, CSVRow…
    CSVFormat fmt;
    fmt.delimiter(';').header_row(0).variable_columns(false);

    CSVReader reader(path, fmt);

    /* Pré-dimensionne le vector pour éviter les reallocations ------------ */
    out_quotes.clear();
    out_quotes.reserve(std::distance(reader.begin(), reader.end()));

    for (const CSVRow& r : reader)
    {
        OptionChainQuote q{};                 // zero-initialisé

        /* --- conversions rapides --------------------------------------- */
        q.quote_unixtime = r[0].get<long long>();
        std::strncpy(q.quote_readtime, r[1].get<std::string>().c_str(),
            sizeof q.quote_readtime);
        std::strncpy(q.quote_date, r[2].get<std::string>().c_str(),
            sizeof q.quote_date);

        q.quote_time_hours = to_double_comma_ok(r[3]);
        q.underlying_last = to_double_comma_ok(r[4]);
        std::strncpy(q.expire_date, r[5].get<std::string>().c_str(),
            sizeof q.expire_date);
        q.expire_unixtime = r[6].get<long long>();
        q.dte = to_double_comma_ok(r[7]);
        q.c_delta = to_double_comma_ok(r[8]);
        q.c_gamma = to_double_comma_ok(r[9]);
        q.c_vega = to_double_comma_ok(r[10]);
        q.c_theta = to_double_comma_ok(r[11]);
        q.c_rho = to_double_comma_ok(r[12]);
        q.c_iv = to_double_comma_ok(r[13]);
        q.c_volume = r[14].get<int>();
        q.c_last = to_double_comma_ok(r[15]);
        std::strncpy(q.c_size, r[16].get<std::string>().c_str(),
            sizeof q.c_size);
        q.c_bid = to_double_comma_ok(r[17]);
        q.c_ask = to_double_comma_ok(r[18]);
        q.strike = to_double_comma_ok(r[19]);
        q.p_bid = to_double_comma_ok(r[20]);
        q.p_ask = to_double_comma_ok(r[21]);
        std::strncpy(q.p_size, r[22].get<std::string>().c_str(),
            sizeof q.p_size);
        q.p_last = to_double_comma_ok(r[23]);
        q.p_delta = to_double_comma_ok(r[24]);
        q.p_gamma = to_double_comma_ok(r[25]);
        q.p_vega = to_double_comma_ok(r[26]);
        q.p_theta = to_double_comma_ok(r[27]);
        q.p_rho = to_double_comma_ok(r[28]);
        q.p_iv = to_double_comma_ok(r[29]);
        q.p_volume = r[30].get<int>();
        q.strike_distance = to_double_comma_ok(r[31]);
        q.strike_distance_pct = to_double_comma_ok(r[32]);
        q.call_put_parity_criterion = to_double_comma_ok(r[33]);
        q.c_mid = to_double_comma_ok(r[34]);
        q.p_mid = to_double_comma_ok(r[35]);
        q.c_bidaskspread = to_double_comma_ok(r[36]);
        q.p_bidaskspread = to_double_comma_ok(r[37]);
        q.yte = to_double_comma_ok(r[38]);
        q.calc = to_double_comma_ok(r[39]);
        q.volume = r[40].get<int>();
        q.forward = to_double_comma_ok(r[41]);
        q.log_moneyness = to_double_comma_ok(r[42]);
        q.fwd_pct = to_double_comma_ok(r[43]);
        q.log_money_fwd_pct = to_double_comma_ok(r[44]);

        out_quotes.emplace_back(std::move(q));
    }

    return true;
}
