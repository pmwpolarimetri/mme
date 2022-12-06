from mme_microscope_dataanalysis import *
    
path = "C:/Users/PolarimetriD4-112/dev/mme/data"


"""To rerun data analysis: run analyse-function with the chosen foldername"""
# foldername = "Wed Nov 30 16.23.40 2022 PSG rotation, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Wed Nov 30 16.26.23 2022 PSA rotation, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Wed Nov 30 16.29.55 2022 Calibration angles, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Wed Nov 30 16.36.54 2022 Optimal angles, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Wed Nov 30 17.54.10 2022 Constant intensity, 633nm, binning, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Wed Nov 30 17.09.17 2022 Constant intensity, 633nm, not binning, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)

# foldername = "Fri Nov 11 14.20.08 2022 Calibration angles, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.36.43 2022 Calibration angles, 500nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.27.08 2022 Optimal angles, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.48.06 2022 PSG movement, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.50.38 2022 PSA movement, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.53.16 2022 PSA movement, 410nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 14.55.43 2022 PSG movement, 410nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 15.02.15 2022 Constant intensity, 410nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)
# foldername = "Fri Nov 11 15.05.42 2022 Constant intensity, 633nm, k-space, trans"
# analyse(path,foldername,centerspot_intensity_fit=True,plot_data=False,plot_gaussian_fit=False)


"""To plot a specific image"""
# foldername = "Wed Nov 30 17.09.17 2022 Constant intensity, 633nm, not binning, k-space, trans"
# filename = "PSG-15.1PSA-51.7Wl633"
# file = path+os.sep+foldername+os.sep+filename+".npy"
# im: np.ndarray = np.load(file)
# plt.imshow(im)
# reference_file = path+os.sep+foldername+os.sep+filename+".txt"
# reference = np.loadtxt(reference_file)
# print(reference)


"""To plot all images in a folder"""
# foldername = "Wed Nov 30 17.09.17 2022 Constant intensity, 633nm, not binning, k-space, trans"
# files = glob(path+os.sep+foldername+os.sep+"*.npy")
# reference_files = glob(path+os.sep+foldername+os.sep+"*.txt")
# files = [x for x in files if "Peak_positions" not in x]
# files = [x for x in files if "Intensities" not in x]
# for file in files:
#     if ("Dark measurement") in file:
#         files.remove(file)
#         dark_im: np.ndarray = np.load(file)

# for i in range(len(files)):
#     file  = files[i]
#     im: np.ndarray = np.load(file)
#     im = im-dark_im
#     plt.figure()
#     plt.imshow()
#     reference_file = reference_files[i]
#     reference = np.loadtzt(reference_file)
#     print(np.mean(reference))

