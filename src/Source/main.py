from pedalboard import Pedalboard, Reverb, Phaser, Gain, Distortion, Compressor, LowpassFilter
from pedalboard.io import AudioFile
import sys
from pathlib import Path

# Make a Pedalboard object, containing multiple audio plugins:
board = Pedalboard([Reverb(room_size=float(sys.argv[2]), damping=float(sys.argv[3]), wet_level=float(sys.argv[4]), dry_level=float(sys.argv[5]), width=float(sys.argv[6]), freeze_mode=float(sys.argv[7])), Gain(float(sys.argv[8])), Distortion(float(sys.argv[11])), Compressor(threshold_db=float(sys.argv[12]), ratio=float(sys.argv[13]), attack_ms=float(sys.argv[14]), release_ms=float(sys.argv[15])), Phaser(float(sys.argv[16]), float(sys.argv[17]), float(sys.argv[18]), float(sys.argv[19]), float(sys.argv[20])), LowpassFilter(float(sys.argv[21]))])

myPath = Path(sys.argv[1])
# Open an audio file for reading, just like a regular file:
with AudioFile(sys.argv[1]) as f:
  myStem = myPath.stem
  # Open an audio file to write to:
  count = 0
  myStart = float(sys.argv[9])
  myStart = int(myStart)
  myStop = float(sys.argv[10])
  myStop = int(myStop)

  with AudioFile('output_' + myStem + '.wav', 'w', f.samplerate, f.num_channels) as o:

    # Read one second of audio at a time, until the file is empty:
    while f.tell() < f.frames:
      chunk = f.read(f.samplerate)

      # Run the audio through our pedalboard:
      if(myStart != 0 and myStop != 0):
        if(count >= myStart and count <= myStop):
          effected = board(chunk, f.samplerate, reset=False)
        else:
          effected = chunk
      else:
        effected = board(chunk, f.samplerate, reset=False)

      # Write the output to our output file:
      o.write(effected)
      count = count+1