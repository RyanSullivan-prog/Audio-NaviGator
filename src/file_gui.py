# created using code from https://www.geeksforgeeks.org/file-explorer-in-python-using-tkinter/

from tkinter import *
from tkinter import filedialog
import soundfile as sf


def browse_files():
    file_name = filedialog.askopenfilename(initialdir='./', title='select a file',
                                           filetypes=(('Wav files', '*.wav'), ('All files', '*.*')))
    if not file_name.endswith('.wav'):
        label_file_explorer.configure(text='Please select a .wav file')
    else:
        label_file_explorer.configure(text='File opened: ' + file_name)
        sig, samplerate = sf.read(file_name)
        if len(sig.shape) == 1:
            label_sound_data.configure(
                text='Audio has ' + str(sig.shape[0]) + ' frames, 1 channel, and a sample rate of ' + str(samplerate) + ' Hz')
        else:
            label_sound_data.configure(
                text='Audio has ' + str(sig.shape[0]) + ' frames, ' + sig.shape[1] + ' channels, and a sample rate of ' + str(samplerate) + ' Hz')


window = Tk()

window.title('Wav file input')

window.geometry('700x500')

window.config(background='white')

label_file_explorer = Label(window, text='File input', width=100, height=4, fg='blue')

button_explore = Button(window, text='Browse Files', command=browse_files)

button_exit = Button(window, text='Exit', command=exit)

label_sound_data = Label(window, text='', width=100, height=4, fg='blue')

label_file_explorer.grid(column=1, row=1)

button_explore.grid(column=1, row=2)

button_exit.grid(column=1, row=3)

label_sound_data.grid(column=1, row=4)

window.mainloop()
