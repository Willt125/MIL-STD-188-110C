#ifndef FILTERS_H
#define FILTERS_H

#include <cmath>
#include <cstdint>
#include <fftw3.h>
#include <numeric>
#include <vector>

class TapGenerators {
    public:
    std::vector<double> generateSRRCTaps(const size_t num_taps, const double sample_rate, const double symbol_rate, const double rolloff) const {
        std::vector<double> freq_response(num_taps, 0.0);
        std::vector<double> taps(num_taps);

        double fn = symbol_rate / 2.0;
        double f_step = sample_rate / num_taps;

        for (size_t i = 0; i < num_taps / 2; i++) {
            double f = i * f_step;

            if (f <= fn * (1 - rolloff)) {
                freq_response[i] = 1.0;
            } else if (f <= fn * (1 + rolloff)) {
                freq_response[i] = 0.5 * (1 - std::sin(M_PI * (f - fn * (1 - rolloff)) / (2 * rolloff * fn)));
            } else {
                freq_response[i] = 0.0;
            }
        }

        for (size_t i = num_taps / 2; i < num_taps; i++) {
            freq_response[i] = freq_response[num_taps - i - 1];
        }

        fftw_complex* freq_domain = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_taps);
        for (size_t i = 0; i < num_taps; i++) {
            freq_domain[i][0] = freq_response[i];
            freq_domain[i][1] = 0.0;
        }

        std::vector<double> time_domain_taps(num_taps, 0.0);

        fftw_plan plan = fftw_plan_dft_c2r_1d(num_taps, freq_domain, time_domain_taps.data(), FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_free(freq_domain);

        double norm_factor = std::sqrt(std::accumulate(time_domain_taps.begin(), time_domain_taps.end(), 0.0, [](double sum, double val) { return sum + val * val; }));
        for (auto& tap : time_domain_taps) {
            tap /= norm_factor;
        }

        return time_domain_taps;
    }

    std::vector<double> generateLowpassTaps(const size_t num_taps, const double cutoff_freq, const double sample_rate) const {
        std::vector<double> freq_response(num_taps, 0.0);
        std::vector<double> taps(num_taps);

        double fn = cutoff_freq / 2.0;
        double f_step = sample_rate / num_taps;

        // Define frequency response
        for (size_t i = 0; i < num_taps / 2; i++) {
            double f = i * f_step;
            if (f <= fn) {
                freq_response[i] = 1.0;  // Passband
            } else {
                freq_response[i] = 0.0;  // Stopband
            }
        }

        // Mirror the second half of the response for symmetry
        for (size_t i = num_taps / 2; i < num_taps; i++) {
            freq_response[i] = freq_response[num_taps - i - 1];
        }

        // Perform inverse FFT to get time-domain taps
        fftw_complex* freq_domain = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_taps);
        for (size_t i = 0; i < num_taps; i++) {
            freq_domain[i][0] = freq_response[i];
            freq_domain[i][1] = 0.0;
        }

        std::vector<double> time_domain_taps(num_taps, 0.0);
        fftw_plan plan = fftw_plan_dft_c2r_1d(num_taps, freq_domain, time_domain_taps.data(), FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_free(freq_domain);

        // Normalize filter taps
        double norm_factor = std::sqrt(std::accumulate(time_domain_taps.begin(), time_domain_taps.end(), 0.0, [](double sum, double val) { return sum + val * val; }));
        for (auto& tap : time_domain_taps) {
            tap /= norm_factor;
        }

        return time_domain_taps;
    }
};

class Filter {
    public:
    Filter(const std::vector<double>& _filter_taps) : filter_taps(_filter_taps), buffer(_filter_taps.size(), 0.0), buffer_index(0) {}

    double filterSample(const double sample) {
        buffer[buffer_index] = std::complex<double>(sample,0.0);
        double filtered_val = 0.0;
        for (size_t j = 0; j < filter_taps.size(); j++) {
            size_t signal_index = (buffer_index + j) % filter_taps.size();
            filtered_val += filter_taps[j] * buffer[signal_index].real();
        }

        buffer_index = (buffer_index + 1) % filter_taps.size();
        return filtered_val;
    }

    std::complex<double> filterSample(const std::complex<double>& sample) {
        buffer[buffer_index] = sample;
        std::complex<double> filtered_val = std::complex<double>(0.0, 0.0);
        for (size_t j = 0; j < filter_taps.size(); j++) {
            size_t signal_index = (buffer_index + j) % filter_taps.size();
            filtered_val += filter_taps[j] * buffer[signal_index];
        }

        buffer_index = (buffer_index + 1) % filter_taps.size();
        return filtered_val;
    }

    std::vector<double> applyFilter(const std::vector<double>& signal) {
        std::vector<double> filtered_signal(signal.size(), 0.0);

        // Convolve the signal with the filter taps
        for (size_t i = 0; i < signal.size(); ++i) {
            filtered_signal[i] = filterSample(signal[i]);
        }

        return filtered_signal;
    }

    std::vector<std::complex<double>> applyFilter(const std::vector<std::complex<double>>& signal) {
        std::vector<std::complex<double>> filtered_signal(signal.size(), std::complex<double>(0.0, 0.0));

        // Convolve the signal with the filter taps
        for (size_t i = 0; i < signal.size(); ++i) {
            filtered_signal[i] = filterSample(signal[i]);
        }

        return filtered_signal;
    }

    private:
    std::vector<double> filter_taps;
    std::vector<std::complex<double>> buffer;
    size_t buffer_index;
};

class SRRCFilter : public Filter {
    public:
    SRRCFilter(const size_t num_taps, const double sample_rate, const double symbol_rate, const double rolloff) : Filter(TapGenerators().generateSRRCTaps(num_taps, sample_rate, symbol_rate, rolloff)) {}
};

class LowPassFilter : public Filter {
    public:
    LowPassFilter(const size_t num_taps, const double cutoff_freq, const double sample_rate) : Filter(TapGenerators().generateLowpassTaps(num_taps, cutoff_freq, sample_rate)) {}
};

#endif /* FILTERS_H */