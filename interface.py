import subprocess
from tkinter import *

def cacher():
    img_path = img_entry.get()
    txt_path = txt_entry.get()
    out_path = out_entry.get()

    # Lancer le programme en C avec les arguments appropriés
    subprocess.run(["./test.x", img_path, txt_path, out_path])

def extraire():
    img_path = img_entry.get()
    txt_path = txt_entry.get()
    # Lancer le programme en C avec les arguments appropriés
    subprocess.run(["./test.x", img_path, txt_path])

root = Tk()
root.title("La stéganographie")
root.configure(bg='#ECECEC')  # Setting light gray background color

# Setting a bigger window size
root.geometry("500x300")

option_frame = Frame(root, bg='#ECECEC')  # Setting light gray background color
option_frame.pack()

option = IntVar()
cacher_option = Radiobutton(option_frame, text="Cacher", variable=option, value=1, bg='#ECECEC', fg='#333333', font=('Arial', 12))
cacher_option.grid(row=0, column=0, padx=5, pady=5)
extraire_option = Radiobutton(option_frame, text="Extraire", variable=option, value=2, bg='#ECECEC', fg='#333333', font=('Arial', 12))
extraire_option.grid(row=0, column=1, padx=5, pady=5)

param_frame = Frame(root, bg='#ECECEC')  # Setting light gray background color
param_frame.pack()

img_label = Label(param_frame, text="Image:", bg='#ECECEC', fg='#333333', font=('Arial', 12, 'bold'))
img_label.grid(row=0, column=0, padx=5, pady=5)
img_entry = Entry(param_frame, font=('Arial', 12))
img_entry.grid(row=0, column=1, padx=5, pady=5)

txt_label = Label(param_frame, text="Texte:", bg='#ECECEC', fg='#333333', font=('Arial', 12))
txt_label.grid(row=1, column=0, padx=5, pady=5)
txt_entry = Entry(param_frame, font=('Arial', 12))
txt_entry.grid(row=1, column=1, padx=5, pady=5)

out_label = Label(param_frame, text="Sortie:", bg='#ECECEC', fg='#333333', font=('Arial', 12, 'bold'))
out_label.grid(row=2, column=0, padx=5, pady=5)
out_entry = Entry(param_frame, font=('Arial', 12))
out_entry.grid(row=2, column=1, padx=5, pady=5)

def executer():
    choix = option.get()
    if choix == 1:
        cacher()
    elif choix == 2:
        extraire()

executer_button = Button(root, text="Exécuter", command=executer, bg='#333333', fg='#ECECEC', font=('Arial', 12, 'bold'))
executer_button.pack(padx=10, pady=10)

# Hide the output entry for the "Extraire" option
def hide_output_entry():
    out_label.grid_remove()
    out_entry.grid_remove()

# Show the output entry for the "Cacher" option
def show_output_entry():
    out_label.grid()
    out_entry.grid()

# Binding the show/hide functions to the option selection
cacher_option.configure(command=show_output_entry)
extraire_option.configure(command=hide_output_entry)

# Initially hide the output entry
hide_output_entry()

# Launch the GUI
root.mainloop()
