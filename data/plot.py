import numpy as np
import matplotlib.pyplot as plt
from glob import glob
import os
import math

path = "C:\\Users\\Vilde\\OneDrive - NTNU\\Høst 2022\\Prosjektoppgave\\Arbeid\\Data"
foldername = "Wed Oct 12 10.48.56 2022 Frosted glass, k-space image, transmission mode"
#foldername = "Wed Oct 12 10.50.32 2022 Frosted glass, real image, transmission mode"

files = glob(path+os.sep+foldername+os.sep+"*.npy")

fig_rows = 4
fig_columns = math.ceil(len(files)/fig_rows)

fig, axs = plt.subplots(fig_columns,fig_rows,figsize=(5*fig_columns,5*fig_rows))

for i in range(len(files)):
    file = files[i]
    filename = file.replace(path+os.sep+foldername+os.sep,"").replace(".npy","")
    position = filename[:filename.find("Wl")]
    wavelength = filename[filename.find("Wl")+2:]
    
    im: np.ndarray = np.load(file)
    im = im[250:420,170:343]
    
    if fig_rows == 1 or fig_columns == 1:
        ax = axs[i]
    else:
        ax = axs[i//fig_rows,i%fig_rows]
        
    ax.imshow(im)
    
    ax.set_title(position)
    
    ax.set_xticks([])
    ax.set_yticks([])
    
fig.suptitle(foldername)
fig.tight_layout()
plt.savefig(path+os.sep+foldername+os.sep+foldername+".png") 
plt.show()