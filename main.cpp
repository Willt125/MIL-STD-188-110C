#include <bitset>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <sndfile.h>
#include <vector>

#include "ModemController.h"
#include "PSKModulator.h"

BitStream generateBernoulliData(size_t length, double p = 0.5) {
    BitStream random_data;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(p);

    for (size_t i = 0; i < length * 8; ++i) {
        random_data.putBit(dist(gen));  // Generates 0 or 1 with probability p
    }
    return random_data;
}

int main() {
    // Convert sample data to a BitStream object
    BitStream input_data = generateBernoulliData(28800);

    // Configuration for modem
    size_t baud_rate = 75;
    bool is_voice = false;              // False indicates data mode
    bool is_frequency_hopping = false;  // Fixed frequency operation
    size_t interleave_setting = 2;      // Short interleave

    // Create ModemController instance
    ModemController modem(baud_rate, is_voice, is_frequency_hopping, interleave_setting);
    
    const char* file_name = "modulated_signal_75bps_longinterleave.wav";

    // Perform transmit operation to generate modulated signal
    std::vector<int16_t> modulated_signal = modem.transmit(input_data);

    // Output modulated signal to a WAV file using libsndfile
    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.samplerate = 48000;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* sndfile = sf_open(file_name, SFM_WRITE, &sfinfo);
    if (sndfile == nullptr) {
        std::cerr << "Unable to open WAV file for writing modulated signal: " << sf_strerror(sndfile) << "\n";
        return 1;
    }

    sf_write_short(sndfile, modulated_signal.data(), modulated_signal.size());
    sf_close(sndfile);
    std::cout << "Modulated signal written to " << file_name << '\n';

    // Success message
    std::cout << "Modem test completed successfully.\n";

    return 0;
}