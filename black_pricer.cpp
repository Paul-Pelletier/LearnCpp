#include "black_pricer.h"
#include <math.h>

#define PI 3.14159265358979323846
#define EPSILON (0.01)  //

static double norm_cdf(double x) {
    return 0.5 * erfc(-x / sqrt(2.0));
}

static double norm_pdf(double x) {
    return exp(-0.5 * x * x) / sqrt(2.0 * PI);
}

double black_scholes_price(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    int    is_call)
{
    if (total_vol <= 0.0)
        return 0.0;

    double d1 = (log(forward / strike) + 0.5 * total_vol * total_vol) / total_vol;
    double d2 = d1 - total_vol;

    if (is_call) {
        return discount_factor *
            (forward * norm_cdf(d1) - strike * norm_cdf(d2));
    }
    else {
        return discount_factor *
            (strike * norm_cdf(-d2) - forward * norm_cdf(-d1));
    }
}

double black_scholes_delta(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    int    is_call)
{
    if (total_vol <= 0.0)
        return 0.0;

    double d1 = (log(forward / strike) + 0.5 * total_vol * total_vol) / total_vol;
    double base = norm_cdf(d1);

    return discount_factor *
        (is_call ? base : (base - 1.0));
}

double black_scholes_gamma(double forward,
    double strike,
    double total_vol,
    double discount_factor)
{
    if (total_vol <= 0.0 || forward <= 0.0)
        return 0.0;

    double d1 = (log(forward / strike) + 0.5 * total_vol * total_vol) / total_vol;
    return discount_factor * norm_pdf(d1) / (forward * total_vol);
}

double black_scholes_vega(double forward,
    double strike,
    double total_vol,
    double discount_factor,
    double maturity)
{
    if (total_vol <= 0.0 || maturity <= 0.0)
        return 0.0;

    double d1 = (log(forward / strike) + 0.5 * total_vol * total_vol) / total_vol;
    // Vega par variation de σ, /100 pour points vol
    return forward * norm_pdf(d1) * sqrt(maturity) * discount_factor / 100.0;
}

double black_scholes_theta(double forward,
    double strike,
    double sigma,
    double discount_factor,
    double maturity,
    int    is_call)
{
    if (sigma <= 0.0 || maturity <= EPSILON)
        return 0.0;

    // maturités ±1 jour
    double mp = maturity + EPSILON;
    double mm = maturity - EPSILON;
    if (mm <= 0.0) mm = maturity;

    // volatilité totale pour chaque T
    double vol_p = sigma * sqrt(mp);
    double vol_m = sigma * sqrt(mm);

    double p_plus = black_scholes_price(forward, strike, vol_p, discount_factor, is_call);
    double p_minus = black_scholes_price(forward, strike, vol_m, discount_factor, is_call);

    // θ ≈ (C(T+Δ) - C(T−Δ)) / (2Δ) → prix/jour
    return -(p_plus - p_minus) / (2.0 * EPSILON)/365;
}
