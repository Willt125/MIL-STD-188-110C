#include <complex>
#include <cmath>
#include <vector>
#include <iostream>

#include "filters.h"

class PhaseDetector {
public:
    PhaseDetector() {}
    PhaseDetector(const std::vector<std::complex<double>>& _symbolMap) : symbolMap(_symbolMap) {}

    uint8_t getSymbol(const std::complex<double>& input) {
        double phase = std::atan2(input.imag(), input.real());
        return symbolFromPhase(phase);
    }

private:
    std::vector<std::complex<double>> symbolMap;

    uint8_t symbolFromPhase(const double phase) {
        // Calculate the closest symbol based on phase difference
        double min_distance = 2 * M_PI; // Maximum possible phase difference
        uint8_t closest_symbol = 0;

        for (uint8_t i = 0; i < symbolMap.size(); ++i) {
            double symbol_phase = std::atan2(symbolMap[i].imag(), symbolMap[i].real());
            double distance = std::abs(symbol_phase - phase);

            if (distance < min_distance) {
                min_distance = distance;
                closest_symbol = i;
            }
        }

        return closest_symbol;
    }
};

class CostasLoop {
public:
    CostasLoop(const double _carrier_freq, const double _sample_rate, const std::vector<std::complex<double>>& _symbolMap, const double _vco_gain, const double _alpha, const double _beta) 
        : carrier_freq(_carrier_freq), sample_rate(_sample_rate), vco_gain(_vco_gain), alpha(_alpha), beta(_beta), freq_error(0.0), k_factor(-1 / (_sample_rate * _vco_gain)),
          prev_in_iir(0), prev_out_iir(0), prev_in_vco(0), feedback(1.0, 0.0),
          error_total(0), out_iir_total(0), in_vco_total(0),
          srrc_filter(SRRCFilter(48, _sample_rate, 2400, 0.35)) {}

    std::vector<std::complex<double>> process(const std::vector<double>& input_signal) {
        std::vector<std::complex<double>> output_signal(input_signal.size());
        double current_phase = 0.0;
        
        error_total = 0;
        out_iir_total = 0;
        in_vco_total = 0;

        for (size_t i = 0; i < input_signal.size(); ++i) {
            // Multiply input by feedback signal
            std::complex<double> multiplied = input_signal[i] * feedback;

            // Filter signal
            std::complex<double> filtered = srrc_filter.filterSample(multiplied);

            // Output best-guess corrected I/Q components
            output_signal[i] = filtered;

            // Generate limited components
            std::complex<double> limited = limiter(filtered);
            
            // IIR Filter
            double error_real = (limited.real() > 0 ? 1.0 : -1.0) * limited.imag();
            double error_imag = (limited.imag() > 0 ? 1.0 : -1.0) * limited.real();
            double phase_error = error_real - error_imag;
            phase_error = 0.5 * (std::abs(phase_error + 1) - std::abs(phase_error - 1));

            freq_error += beta * phase_error;
            double phase_adjust = alpha * phase_error + freq_error;

            current_phase += 2 * M_PI * carrier_freq / sample_rate + k_factor * phase_adjust;
            if (current_phase > M_PI) current_phase -= 2 * M_PI;
            else if (current_phase < -M_PI) current_phase += 2 * M_PI;

            // Generate feedback signal for next iteration
            double feedback_real = std::cos(current_phase);
            double feedback_imag = -std::sin(current_phase);
            feedback = std::complex<double>(feedback_real, feedback_imag);
        }

        return output_signal;
    }

private:
    double carrier_freq;
    double sample_rate;
    double k_factor;
    double prev_in_iir;
    double prev_out_iir;
    double prev_in_vco;
    std::complex<double> feedback;
    double error_total;
    double out_iir_total;
    double in_vco_total;
    SRRCFilter srrc_filter;
    double vco_gain;
    double alpha;
    double beta;
    double freq_error;

    std::complex<double> limiter(const std::complex<double>& sample) const {
        double limited_I = std::clamp(sample.real(), -1.0, 1.0);
        double limited_Q = std::clamp(sample.imag(), -1.0, 1.0);
        return std::complex<double>(limited_I, limited_Q);
    }
};