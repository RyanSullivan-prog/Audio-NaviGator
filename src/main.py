# working with soundfile tutorial https://github.com/mgeier/python-audio/blob/d8b877bc91e1637aae628bc9d225531fac021de7/audio-files/audio-files-with-pysoundfile.ipynb
import soundfile as sf
from pathlib import Path


def input_audio_file():
    file_path = Path(input('Please enter a path to a wav file: '))
    sig, samplerate = sf.read(file_path)
    # prints frames and channels for file, for babyelephantwalk60 should be 1323000 and empty/1
    print(sig.shape)
    # prints samplerate for file, for babyelephanwalk60 should be 22050
    print(samplerate)


if __name__ == "__main__":
    input_audio_file()
