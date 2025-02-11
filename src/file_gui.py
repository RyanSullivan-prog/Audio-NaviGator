# created using code from https://www.geeksforgeeks.org/file-explorer-in-python-using-tkinter/

from tkinter import *
from tkinter import filedialog


def browse_files():
    file_name = filedialog.askopenfilename(initialdir='/', title='select a file',
                                           filetypes=(('Wav files', '*.wav'), ('All files', '*.*')))
    if not file_name.endswith('.wav'):
        label_file_explorer.configure(text='Please select a .wav file')
    else:
        label_file_explorer.configure(text='File opened: ' + file_name)


window = Tk()

window.title('Wav file input')

window.geometry('700x500')

window.config(background='white')

label_file_explorer = Label(window, text='File input using Tkinter', width=100, height=4, fg='blue')

button_explore = Button(window, text='Browse Files', command=browse_files)

button_exit = Button(window, text='Exit', command=exit)

label_file_explorer.grid(column=1, row=1)

button_explore.grid(column=1, row=2)

button_exit.grid(column=1, row=3)

window.mainloop()
