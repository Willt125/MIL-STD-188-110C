import numpy as np

samplerate = 384000
tone0, tone1, tone2, tone3, tone4, tone5, tone6, tone7 = [], [], [], [], [], [], [], []
f_c = 1800
for i in range(160):
    tone0.append(np.cos(2*np.pi*f_c*i/samplerate))
    tone1.append(np.cos(2*np.pi*f_c*i/samplerate + 45))
    tone2.append(np.cos(2*np.pi*f_c*i/samplerate + 90))
    tone3.append(np.cos(2*np.pi*f_c*i/samplerate + 135))
    tone4.append(np.cos(2*np.pi*f_c*i/samplerate + 180))
    tone5.append(np.cos(2*np.pi*f_c*i/samplerate + 225))
    tone6.append(np.cos(2*np.pi*f_c*i/samplerate + 270))
    tone7.append(np.cos(2*np.pi*f_c*i/samplerate + 315))


def convertToTones(message: np.ndarray) -> np.ndarray:
    outsound = np.empty(len(message) * 160, dtype=np.float32)
    _len = 160
    for i in range(len(message)):
        match message[i]:
            case 0:
                outsound[i*_len:i*_len+_len] = tone0
            case 1:
                outsound[i*_len:i*_len+_len] = tone1
            case 2:
                outsound[i*_len:i*_len+_len] = tone2
            case 3:
                outsound[i*_len:i*_len+_len] = tone3
            case 4:
                outsound[i*_len:i*_len+_len] = tone4
            case 5:
                outsound[i*_len:i*_len+_len] = tone5
            case 6:
                outsound[i*_len:i*_len+_len] = tone6
            case 7:
                outsound[i*_len:i*_len+_len] = tone7
    return outsound
