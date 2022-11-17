from analyse_and_plot_function import *
    
#path = "C:\\Users\\PolarimetriD4-112\\dev\\mme\\data"
path = "C:/Users/Vilde/OneDrive - NTNU/Høst 2022/Prosjektoppgave/Arbeid/Data"

#file = path+"/Thu Nov 10 19.59.54 2022 Air calibration angles 633nm, bright background, k-space, trans/PSG0.0PSA0.0Wl633.npy"
#im: np.ndarray = np.load(file)
#plt.imshow(im)


#analyse_and_plot(path,foldername,kspace=True)

foldername = "Fri Nov 11 14.20.08 2022 Calibration angles, 633nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.36.43 2022 Calibration angles, 500nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.27.08 2022 Optimal angles, 633nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.48.06 2022 PSG movement, 633nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.50.38 2022 PSA movement, 633nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.53.16 2022 PSA movement, 410nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 14.55.43 2022 PSG movement, 410nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 15.02.15 2022 Constant intensity, 410nm, k-space, trans"
analyse(path,foldername,kspace=True)
foldername = "Fri Nov 11 15.05.42 2022 Constant intensity, 633nm, k-space, trans"
analyse(path,foldername,kspace=True)