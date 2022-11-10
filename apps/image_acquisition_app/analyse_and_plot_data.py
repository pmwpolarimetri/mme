from analyse_and_plot_function import *
    
path = "C:\\Users\\PolarimetriD4-112\\dev\\mme\\data"

foldername = 'Thu Nov 10 20.05.45 2022 Air calibration angles 633nm, bright background, k-space, trans'
#analyse_and_plot(path,foldername,kspace=True)
analyse(path,foldername,kspace=True)

#file = path+"/Thu Nov 10 19.59.54 2022 Air calibration angles 633nm, bright background, k-space, trans/PSG0.0PSA0.0Wl633.npy"
#im: np.ndarray = np.load(file)
#plt.imshow(im)