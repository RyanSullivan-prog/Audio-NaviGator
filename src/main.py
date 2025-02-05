# working with soundfile tutorial https://github.com/mgeier/python-audio/blob/d8b877bc91e1637aae628bc9d225531fac021de7/audio-files/audio-files-with-pysoundfile.ipynb
import soundfile as sf
from pathlib import Path


def input_audio_file():
    file_string = input('Please enter a path to a wav file: ')
    file_path = Path(file_string)
    # verify path exists and points to wav
    while not (file_path.exists() and file_string.endswith('.wav')):
        if not file_path.exists():
            print('Path does not exist.')
        if not file_string.endswith('.wav') and file_path.exists():
            print('Path does not point to wav file')
        file_string = input('Please enter a path to a wav file: ')
        file_path = Path(file_string)
    # verify path leads to wav file

    sig, samplerate = sf.read(file_path)
    # prints frames and channels for file, for babyelephantwalk60 should be 1323000 and empty/1
    print(sig.shape)
    # prints samplerate for file, for babyelephanwalk60 should be 22050
    print(samplerate)


if __name__ == "__main__":
    input_audio_file()
