from intensity_fit_calibration import *

import sys
sys.path.insert(1,"C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/")
from sample_MM_generation import *

""" Perform calibration"""
datapath = "C:/Users/PolarimetriD4-112/dev/mme/data/"
file = datapath+"Fri Nov 11 14.20.08 2022 Calibration angles, 633nm, k-space, trans/Intensities.npy"
#file = datapath+"Sat Dec  3 14.07.26 2022 Calibration angles 633nm, k-space, trans, without reference detector/Intensities.npy"
#file = datapath+"Sat Dec  3 14.22.57 2022 Calibration angles 633nm, k-space, trans/Intensities.npy"

calibrationfilepath = "C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/"

wavelength = 633
function = Isim_simple

calibration(file,wavelength,calibrationfilepath,function)

""" Test if the dispersion model looks correct"""
test_model(file, calibrationfilepath+"calibration_values_500nm.npy", 633, 500)

""" Test MM generation """
calibration_file = calibrationfilepath+"calibration_values_633nm.npy"
optimal_angles_file = datapath+"Fri Nov 11 14.27.08 2022 Optimal angles, 633nm, k-space, trans"
#optimal_angles_file = datapath+"Sat Dec  3 14.16.22 2022 Optimal angles 633nm, k-space, trans, without reference detector"
#optimal_angles_file = datapath+"Sat Dec  3 14.29.34 2022 Optimal angles 633nm, k-space, trans"
get_MM_from_calibration(calibration_file,633,optimal_angles_file)