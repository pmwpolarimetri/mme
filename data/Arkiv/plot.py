import numpy as np
import matplotlib.pyplot as plt
from glob import glob
import os
import math

def plot(path,foldername,fig_rows=4):
    
    files = sorted(glob(path+os.sep+foldername+os.sep+"*.npy"),key=lambda x: (float(x[x.index("PSG",len(path)+len(foldername)+2)+3:x.index("PSA",len(path)+len(foldername)+2)]),float(x[x.index("PSA",len(path)+len(foldername)+2)+3:x.index("Wl",len(path)+len(foldername)+2)])))
    
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
    plt.savefig(path+os.sep+foldername+os.sep+"test.png") 
    plt.show()
    
path = "C:\\Users\\PolarimetriD4-112\\dev\\mme\\data"

foldername = 'Wed Oct 19 14.57.04 2022 Air, spot movement PSA,  interference filter wl531nm, k-space image, transmission mode'
plot(path,foldername,fig_rows=6)