# Mil-Std-188-110
A recreation of the MIL-188-110 encoding scheme for HF digital data/voice transfer

The encoding steps are as follows:
PreambleGenerate() + Input string -> EncodeInputData() -> FECEncodeBits() -> InterleaveBits() -> MGD_Decode() -> channelMap() -> Scramble() -> append -> ConvertToTones()

All you have to do is:
open a shell in the file directory
```py
import MIL-STD-188-110A
instring = "Hello, World!"
outsound = main(instring, 75, "L", "Data", "fixed", 1)
wavfile.write("/dir/here/filename.wav", 384000, outsound)
```
You then manually run wavfile.write() (from scipy), so that you can choose your own save location.

The current code is capable of fixed-frequency text encoding, and it produces audio that sounds right, though I don't have a way of testing a decode.
The code SHOULD run, but could probably be implemented better.

-------------------------------------------------------------------------------------

The C++ code, on the other hand, is not complete, will not run (yet), and could almost DEFINITELY be done a lot better.

-------------------------------------------------------------------------------------

The standard used is available here: https://www.sigidwiki.com/images/c/c8/MIL-STD-188_110C_CHG_NOTICE-1.pdf