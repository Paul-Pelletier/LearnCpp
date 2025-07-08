/* csv_loader.h ----------------------------------------------------------- */
/*   Lecture du fichier raw_data.csv avec vincentlaucsb-csv-parser         */
/* ----------------------------------------------------------------------- */
#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <string>
#include <vector>
#include "option_data.h"    /* définit OptionChainQuote                   */

bool load_quotes_csv(const std::string& path,
    std::vector<OptionChainQuote>& out_quotes);
