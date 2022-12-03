"""
Script called by image_acquisition_app.cpp in mme, for data analysis of the acquired data from the Mueller matrix microscope in D4-112
Written by Vilde Vraalstad, last modified December 2022

Runs the analysis in mme_microscope_dataanalysis.py on the acquired data from a measurement series
- Performs dark-correction, and prints a warning if saturation of the detector is close or reached.
- All acquired images are plotted.
- If the filename contains "k-space", a gaussian fit of the center spot is performed, the resulting peak intensities are saved to file,
  and the sample Mueller matrix calculated from intensity fit calibration at 633nm is saved to file.
"""

from mme_microscope_dataanalysis import *

pathinput = str(sys.argv[3])
foldername = pathinput.split('/')[-1]
path = pathinput.replace('/'+foldername,'')

kspace = False
if "k-space" in foldername:
    kspace=True

analyse(path,foldername,kspace_intensity_fit=kspace,plot_data=True)