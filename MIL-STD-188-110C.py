#!/usr/bin/env python3

import numpy as np
import scipy.io.wavfile as wavfile
from scipy.signal import butter, lfilter
from PreambleGenerate import PreambleGenerate
from InputDataToBinaryStream import InputDataToBinaryStream
from FECEncodeBits import FECEncodeBits
from InterleaveData import InterleaveData
from MGDDecode import MGD_Decode
from channelMap import channelMap
from ScrambleBits import ScrambleBits
from ConvertToTones import convertToTones

def butter_lowpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    return b, a

def butter_lowpass_filter(data, cutoff, fs, order=5):
    b, a = butter_lowpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y

def butter_highpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    return b, a

def butter_highpass_filter(data, cutoff, fs, order=5):
    b, a = butter_highpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y

samplerate = 384000

def main(input_string, baud, interleave_size, data_type, frequency_mode, eom_cycles):
    '''The main wrapper function.
    `input_string` is any ASCII string. No limit on length, but untested.
    `baud` is the selected baud. You have 75-4800 Baud, by doubles.
    `interleave_length` is self-explanatory. You have zero, short, and long.
    `data_type` is "Data" or "Voice". Voice is untested, nor do I know what input data it would expect. Haven't read that part of the spec.
    `frequency_mode` is "Fixed" or "Hopping". Hopping isn't implemented.
    If you don't know what to pick, go 75L.
    Returns the waveform in a numpy array (samplerate 384000, mono, float), ready to write to a sound file.'''
        
    if input_string == "":
        print("No input. Not tested.")
    
    match baud:
        case 75 | "75":
            Bd = 75 # Set baud to 75 Bd
        case 150 | "150":
            Bd = 150 # Set baud to 150 Bd
        case 300 | "300":
            Bd = 300 # Set baud to 300 Bd
        case 600 | "600":
            Bd = 600 # Set baud to 600 Bd
        case 1200 | "1200":
            Bd = 1200 # Set baud to 1200 Bd
        case 2400 | "2400":
            Bd = 2400 # Set baud to 2400 Bd
        case 4800 | "4800":
            Bd = 4800 # Set baud to 4800 Bd
        
    assert Bd in (75, 150, 300, 600, 1200, 2400, 4800), "Invalid baud setting detected. Please choose a baud setting between 75 and 4800."
    
    match interleave_size:
        case "Z" | "z":
            interleave_setting =  "Z" # Set zero interleave length
        case "S" | "s":
            interleave_setting =  "S" # Set short interleave length
        case "L" | "l":
            interleave_setting =  "L" # Set long interleave length
        
    assert interleave_setting in ("Z", "S", "L"), "Invalid interleave length setting detected. Please choose one of Z (Zero), S (Short), or L (Long)."
    
    match data_type:
        case "Data":
            pass
        case "Voice":
            print("Voice input is not tested. Proceed with caution.")
    
    assert data_type in ("Data", "Voice"), "Invalid data setting detected. Please select one of Data or Voice."
    
    match frequency_mode:
        case "Fixed":
            pass
        case "Hopping":
            raise RuntimeError("Frequency hopping mode is not supported at this time.")
            
    assert frequency_mode in ("Fixed", "Hopping"), "Invalid frequency mode setting detected. Please select one of Fixed or Hopping."
        
    match (interleave_setting, Bd):
        case ("Z", _):
            interleave_length = 0
        case ("S", 75):
            interleave_length = 90
        case ("S", 150) | ("S", 300) | ("S", 600):
            interleave_length = 720
        case ("S", 1200):
            interleave_length = 1440
        case ("S", 2400):
            interleave_length = 2880
        case ("S", 4800):
            interleave_length = 0
        case ("L", 75):
            interleave_length = 720
        case ("L", 150) | ("L", 300) | ("L", 600):
            interleave_length = 5760
        case ("L", 1200):
            interleave_length = 11520
        case ("L", 2400):
            interleave_length = 23040
        case ("L", 4800):
            interleave_length = 0
        case (_, _):
            raise RuntimeError("An unexpected error has occurred!")

    # And now we start chugging!
    preamble_array = PreambleGenerate(Bd, interleave_setting, data_type)
    input_data = InputDataToBinaryStream(input_string, interleave_length, eom_cycles)
    fec_out = FECEncodeBits(input_data, Bd, frequency_mode)
    interleave_out = InterleaveData(fec_out, Bd, interleave_setting, frequency_mode)
    mgd_out = MGD_Decode(interleave_out, Bd, frequency_mode)
    map_out = channelMap(mgd_out, Bd, interleave_setting, frequency_mode, data_type)
    scramble_out = ScrambleBits(map_out)
    encoder_out = np.append(preamble_array, scramble_out)
    output_baseband = convertToTones(encoder_out)

    # And finally, we correct the bandwidth (3kHz, centered on 1800 Hz)
    output_baseband = butter_lowpass_filter(output_baseband, 3300, samplerate, order=1)
    output_baseband = butter_highpass_filter(output_baseband, 300, samplerate, order=1)

    return output_baseband
