#ifndef BLACK_PRICER_H
#define BLACK_PRICER_H

/*
   total_vol = σ * sqrt(T)
   is_call  = 1 pour un call, 0 pour un put
*/
double black_scholes_price(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    int    is_call);

double black_scholes_delta(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    int    is_call);

double black_scholes_gamma(double forward,
    double strike,
    double total_vol,
    double discount_factor);

/*
   maturity en années (T), sigma = volatilité annuelle σ
*/
double black_scholes_vega(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    double maturity);

double black_scholes_theta(double forward,
    double strike,
    double sigma,
    double discount_factor,
    double maturity,
    int    is_call);

#endif /* BLACK_PRICER_H */
