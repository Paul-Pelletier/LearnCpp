// LearnCpp.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <time.h>

struct OptionChainQuote {
    int64_t quote_unixtime;
    char quote_readtime[20];     // "YYYY-MM-DD HH:MM"
    char quote_date[12];         // "YYYY-MM-DD"
    double quote_time_hours;
    double underlying_last;
    char expire_date[12];
    int64_t expire_unixtime;
    double dte;
    double c_delta;
    double c_gamma;
    double c_vega;
    double c_theta;
    double c_rho;
    double c_iv;
    int c_volume;
    double c_last;
    char c_size[16];             // e.g., "25 x 25"
    double c_bid;
    double c_ask;
    double strike;
    double p_bid;
    double p_ask;
    char p_size[16];
    double p_last;
    double p_delta;
    double p_gamma;
    double p_vega;
    double p_theta;
    double p_rho;
    double p_iv;
    int p_volume;
    double strike_distance;
    double strike_distance_pct;
    double call_put_parity_criterion;   // Attention: changé en double
    double c_mid;
    double p_mid;
    double c_bidaskspread;
    double p_bidaskspread;
    double yte;
    double calc;
    int volume;
    double forward;
    double log_moneyness;
    double fwd_pct;
    double log_money_fwd_pct;
};

const char* field_names[] = {
    "quote_unixtime",
    "quote_readtime",
    "quote_date",
    "quote_time_hours",
    "underlying_last",
    "expire_date",
    "expire_unixtime",
    "dte",
    "c_delta",
    "c_gamma",
    "c_vega",
    "c_theta",
    "c_rho",
    "c_iv",
    "c_volume",
    "c_last",
    "c_size",
    "c_bid",
    "c_ask",
    "strike",
    "p_bid",
    "p_ask",
    "p_size",
    "p_last",
    "p_delta",
    "p_gamma",
    "p_vega",
    "p_theta",
    "p_rho",
    "p_iv",
    "p_volume",
    "strike_distance",
    "strike_distance_pct",
    "call_put_parity_criterion",
    "c_mid",
    "p_mid",
    "c_bidaskspread",
    "p_bidaskspread",
    "yte",
    "calc",
    "volume",
    "forward",
    "log_moneyness",
    "fwd_pct",
    "log_money_fwd_pct"
};

int get_csv_rows_count(FILE* file) {
    rewind(file);
    int result = 0;
    int c = 0;
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            result++;
        }
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

    // Skip header
    fgets(buffer, sizeof(buffer), file);

    int rowCounter = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && rowCounter < rowsToFill) {
        buffer[strcspn(buffer, "\r\n")] = 0;

        char* token = strtok(buffer, ";");
        int field = 0;

        while (token != NULL) {
            switch (field) {
            case 0: quotes[rowCounter].quote_unixtime = strtoll(token, NULL, 10); break;
            case 1: strncpy(quotes[rowCounter].quote_readtime, token, sizeof(quotes[rowCounter].quote_readtime));
                quotes[rowCounter].quote_readtime[sizeof(quotes[rowCounter].quote_readtime) - 1] = '\0'; break;
            case 2: strncpy(quotes[rowCounter].quote_date, token, sizeof(quotes[rowCounter].quote_date));
                quotes[rowCounter].quote_date[sizeof(quotes[rowCounter].quote_date) - 1] = '\0'; break;
            case 3: quotes[rowCounter].quote_time_hours = parse_double(token); break;
            case 4: quotes[rowCounter].underlying_last = parse_double(token); break;
            case 5: strncpy(quotes[rowCounter].expire_date, token, sizeof(quotes[rowCounter].expire_date));
                quotes[rowCounter].expire_date[sizeof(quotes[rowCounter].expire_date) - 1] = '\0'; break;
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
            case 16: strncpy(quotes[rowCounter].c_size, token, sizeof(quotes[rowCounter].c_size));
                quotes[rowCounter].c_size[sizeof(quotes[rowCounter].c_size) - 1] = '\0'; break;
            case 17: quotes[rowCounter].c_bid = parse_double(token); break;
            case 18: quotes[rowCounter].c_ask = parse_double(token); break;
            case 19: quotes[rowCounter].strike = parse_double(token); break;
            case 20: quotes[rowCounter].p_bid = parse_double(token); break;
            case 21: quotes[rowCounter].p_ask = parse_double(token); break;
            case 22: strncpy(quotes[rowCounter].p_size, token, sizeof(quotes[rowCounter].p_size));
                quotes[rowCounter].p_size[sizeof(quotes[rowCounter].p_size) - 1] = '\0'; break;
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

        if (field < 45) {
            printf("Warning: line %d has only %d fields\n", rowCounter, field);
        }

        rowCounter++;
    }
	rewind(file);
}

void print_fields_of_option_chain_quote(struct OptionChainQuote* quote) {
    std::cout << "quote_unixtime: " << quote->quote_unixtime << "\n";
    std::cout << "quote_readtime: " << quote->quote_readtime << "\n";
    std::cout << "quote_date: " << quote->quote_date << "\n";
    std::cout << "quote_time_hours: " << quote->quote_time_hours << "\n";
    std::cout << "underlying_last: " << quote->underlying_last << "\n";
    // Tu peux compléter ici si tu veux afficher tous les champs
}

void print_option_chain_quote(struct OptionChainQuote* quotes, int rowsToPrint) {
    for (int i = 0; i < rowsToPrint; i++) {
        std::cout << "Row " << i + 1 << ":\n";
        print_fields_of_option_chain_quote(&quotes[i]);
        std::cout << "\n";
    }
}

int main()
{
    std::string path = "C:\\Users\\paula\\source\\repos\\LearnCpp\\raw_data.csv";

    clock_t start = clock();
    FILE* file = fopen(path.c_str(), "r");
    int rowsCount = get_csv_rows_count(file);
    std::cout << "Rows count: " << rowsCount << "\n";

    int numberOfImports = 1000;
    struct OptionChainQuote* quotes = new struct OptionChainQuote[rowsCount];

    for (int i = 0; i <= numberOfImports; i++) {
        if (i % 100 == 0) {
            fill_all_option_chain_quotes(quotes, rowsCount - 1, file);
            std::cout << "Importing file #" << (i + 1) << "\n";
        }
    }

    clock_t end = clock();
    double elapsed_seconds = (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "Memory allocated for " << rowsCount << " OptionChainQuote structures.\n";

    // Afficher les 3 premières lignes
    //print_option_chain_quote(quotes, 746);

    printf("Execution time: %.6f seconds\n", elapsed_seconds);
	fclose(file);
	delete[] quotes;
    std::cin.get();
    return 0;
}
