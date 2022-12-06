"""
Program for data analysis of the acquired data from the Mueller matrix microscope in D4-112
Written by Vilde Vraalstad, last modified December 2022

The function analyse(path, foldername, bool centerspot_intensity_fit, bool plot_data, bool plot_gaussian_fit) reads all data in the folder,
performs dark-correction, and prints a warning if saturation of the detector is close or reached.
- If centerspot_intensity_fit is True, a gaussian fit of the center spot is performed, the resulting peak intensities are saved to file,
  and the sample Mueller matrix calculated from intensity fit calibration at 633nm is saved to file.
- If plot_data is True, the images are plotted and saved as a figure in the datafolder
- If plot_gaussian_fit is True, the gaussian fits of the center spots are plotted. It can be useful to double check this if the resulting intensities seem wrong.
"""

from sample_MM_generation import *

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from glob import glob
import os
import math
import sys

image_acquisition_app_path = "C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/"

def rad(x):
    return x*np.pi/180

def gaussian_2d(xy,x0,y0,sigma,A,offset):
    x,y=xy
    I = A*np.exp(-1/(2*sigma**2)*((x-x0)**2+(y-y0)**2))+offset
    return I

def gaussian_2d_fit(image):
    x = np.arange(0, image.shape[1], 1)
    y = np.arange(0, image.shape[0], 1)
    xx, yy = np.meshgrid(x, y)
    
    # Guess intial parameters
    max_idx = np.unravel_index(np.argmax(image, axis=None), image.shape)
    x0 = max_idx[1]
    y0 = max_idx[0]
    sigma = max(*image.shape) * 0.1
    A = np.max(image)
    offset = image[0,0]
    initial_guess = [x0, y0, sigma, A, offset]
    
    #Use the Levenberg-Marquadt algorithm to do the curve fit: method="lm"
    [x0,y0,sigma,A,offset], uncert_cov = curve_fit(gaussian_2d,(xx.ravel(),yy.ravel()),image.ravel(),method="lm",p0=initial_guess)
    
    im_fit = gaussian_2d((xx.ravel(),yy.ravel()),x0,y0,sigma,A,offset)
    im_fit = im_fit.reshape(image.shape)
    
    return [x0,y0,sigma,A,offset],im_fit

def create_circular_mask(h, w, center, radius):
    Y, X = np.ogrid[:h, :w]
    dist_from_center = np.sqrt((X - center[0])**2 + (Y-center[1])**2)
    mask = dist_from_center <= radius
    return mask

def PSA_movement_correction(PSA_angle):
    """ 
    Use the calibration file to find the spot position change (in number of pixels) form origin as a function of the PSA position.
    """
    
    PSA_calibration_file = image_acquisition_app_path+"spot_rotation_PSA_calibration.txt"
    try:
        PSA_angle = rad(PSA_angle)
        [angle_offset, xc, yc, R] = np.loadtxt(PSA_calibration_file)
        x_correction = xc+R*np.cos(angle_offset+PSA_angle)
        y_correction = yc+R*np.sin(angle_offset+PSA_angle)
        return x_correction,y_correction
    except:
        print("Could not find file spot_rotation_PSA_calibration.txt. Make sure it is placed in the folder mme/image_acquisition_app.")
        return 0,0

def do_gaussian_2d_fit(file,reference_file,dark_im,PSA_pos=0,plot=True):
    """
    Performs a gaussian fit of the center spot, close to the position (255,335) of the image, and makes dark and offset corrections to the image.
    Returns the peak intensity, fitted parameters and fitted image.
    If plot = True, the gaussian fit is plotted.
    
    Only used if k-space image.
    """
    
    im: np.ndarray = np.load(file)
    
    if np.array_equal(dark_im,np.ndarray(shape=(512,512))) == False:
        im = im-dark_im
        
    x_correction,y_correction = PSA_movement_correction(PSA_pos)
    
    #Find position of maximum inside the expected area
    h,w=im.shape
    circular_mask = create_circular_mask(w,h, center=(255/512*w+x_correction,335/512*h+y_correction),radius=0.03*h)
    center = np.copy(im)
    #center = scipy.ndimage.gaussian_filter(center, 3) #Smooth to remove small spots
    center[~circular_mask] = 0
    max_idx = np.flip(np.unravel_index(np.argsort(center.ravel())[-1], center.shape))  
    
    #Choose to fit only the center spot
    im = im[int(335/512*h+y_correction-20):int(335/512*h+y_correction+20),int(255/512*w+x_correction-20):int(255/512*w+x_correction+20)]
    
    h,w = im.shape
    
    # # Can choose to smooth the signal using a Gaussian filter
    # im = scipy.ndimage.gaussian_filter(im, 3)
    
    [x0,y0,sigma,A,offset],im_fit = gaussian_2d_fit(im)
    
    
    #Remove background signal
    im = im-offset
    im_fit = im_fit-offset
    
    if plot:
        
        fig, (ax1,ax2,ax3) = plt.subplots(1,3,figsize=(15,5))
        
        #Plot the sum of the beam along y, and the center plane along y, as a function of x-position,
        #and vice versa
        
        x = np.arange(w)
        y = np.arange(h)
        max_idx = np.flip(np.unravel_index(np.argsort(im.ravel())[-1], im.shape))

        ax1.plot(x,im[:,max_idx[0]],label="Measured",linewidth=3)
        ax1.plot(x,im.sum(0),label="Measured",linewidth=3)
        ax1.plot(x,im_fit[:,max_idx[0]],label="Fit")
        ax1.plot(x,im_fit.sum(0),label="Fit")
        ax1.legend()

        ax2.plot(y,im[max_idx[1],:],label="Measured",linewidth=3)
        ax2.plot(y,im.sum(1),label="Measured",linewidth=3)
        ax2.plot(y,im_fit[max_idx[1],:],label="Fit")
        ax2.plot(y,im_fit.sum(1),label="Fit")
        ax2.legend()
        
        #Plot the Gaussian fit as a circle on the image, to indicate position and width of the peak
        ax3.imshow(im)
        circle = plt.Circle((x0,y0),sigma, facecolor='none', edgecolor="red", linewidth=1, alpha=0.8)
        ax3.add_patch(circle)

        plt.show()
        
    
    # Find the intensity of the center spor
        
    if file == reference_file:
        reference = 1
    else:
        reference = reference = np.sum(np.loadtxt(reference_file))
    
    # Calculate the total intensity inside fitted gaussian beam
    # - Can use the width 2*sigma: radius = sigma
    # - Can use the radius to FWHM: radius = sigma*np.sqrt(2*np.log(2))
    # - Can use the radius to account for 91% of the beam intensity: radius = 2*sigma
    circular_mask = create_circular_mask(h, w, center=(x0,y0),radius=np.abs(sigma))
    center_spot_fit = np.copy(im_fit)
    center_spot_fit[~circular_mask] = 0
    center_spot = np.copy(im)
    center_spot[~circular_mask] = 0
    
    #print("Deviation between fitted and measured center spot max: {:.2f}%".format(100*(np.max(center_spot_fit)-np.max(center_spot))/np.max(center_spot_fit)))
    
    # # Choose the desired way of selecting the peak intensity
    total_intensity = np.max(center_spot_fit)/reference #Gave best results for initial tests of calibration
    # total_intensity = np.max(center_spot_fit)
    # total_intensity = np.sum(center_spot_fit)/reference
    # total_intensity = np.sum(center_spot_fit)
    # total_intensity = np.max(center_spot)/reference 
    # total_intensity = np.max(center_spot)
    # total_intensity = np.sum(center_spot)/reference
    # total_intensity = np.sum(center_spot)
    
    return total_intensity, [x0,y0,sigma,A,offset], im_fit, im

    
def analyse(path,foldername,centerspot_intensity_fit=False,plot_data=False,plot_gaussian_fit=False):
    
    # Read all .npy-images in the folder (except the saved intensities and peak positions), and all reference .txt-files
    files = glob(path+os.sep+foldername+os.sep+"*.npy")
    files = [x for x in files if "Intensities" not in x]
    files = [x for x in files if "Peak_positions" not in x]
    reference_files = glob(path+os.sep+foldername+os.sep+"*.txt")
    reference_files = [x for x in reference_files if "MM" not in x] 
    
    dark_im = np.ndarray(shape=(512,512))
    for file in files:
        if ("Dark measurement") in file:
            files.remove(file)
            dark_im: np.ndarray = np.load(file)
    
    # If the reference_files-list is not empty, it should be of equal size as the files-list
    if reference_files:
        assert (len(reference_files) == len(files)) 

    if plot_data:
        if len(files) < 5:
            fig_rows = len(files)
        elif len(files) % 6 == 0:
            fig_rows = 6
        elif len(files) % 5 == 0:
            fig_rows = 5
        else:
            fig_rows = 4
        fig_columns = math.ceil(len(files)/fig_rows)
        fig, axs = plt.subplots(fig_columns,fig_rows,figsize=(5*fig_columns,5*fig_rows))
        
    intensities = []
    peak_positions = []
    saturation = False
    for i in range(len(files)):
        file = files[i]
        if reference_files:
            reference_file = reference_files[i]
        else:
            reference_file = files[i]
            
        # Read information of Wavelength and PSG- and PSA-positions from filename
        filename = file.replace(path+os.sep+foldername+os.sep,"").replace(".npy","")
        position = filename[:filename.find("Wl")]
        PSG_pos = float(position[position.find("PSG")+3:position.find("PSA")])
        PSA_pos = float(position[position.find("PSA")+3:])
        
        # Saturation check
        im: np.ndarray = np.load(file)
        if np.array_equal(dark_im,np.ndarray(shape=(512,512))) == False:
            im = im-dark_im
        im = im[250:420,170:343]
        if saturation == False:
            if np.max(im) > 4090:
                print("\nWARNING: The camera is saturated, and the exposure time must be reduced")
                saturation = True
            elif np.max(im) > 4000:
                print("\nVery close to reaching saturation, with largest intensities", np.flip(np.sort(im,axis=None))[:5])
                print("Should reduce the exposure time")
                
        # Perform gaussian intensity fit
        if centerspot_intensity_fit:
            total_intensity, [x0,y0,sigma,A,offset], im_fit, im = do_gaussian_2d_fit(file,reference_file,dark_im,PSA_pos=PSA_pos,plot=plot_gaussian_fit)
            intensities.append([PSG_pos,PSA_pos,round(total_intensity,2)])
            peak_positions.append([PSG_pos,PSA_pos,x0,y0])
        
        if plot_data:
            if fig_rows == 1 or fig_columns == 1:
                ax = axs[i]
            else:
                ax = axs[i//fig_rows,i%fig_rows]
                
            ax.imshow(im)
            
            # Plot the position and width of the gaussian fit, to check that it looks correct
            if centerspot_intensity_fit:
                circle = plt.Circle((x0,y0),sigma, facecolor='none', edgecolor="red", linewidth=1, alpha=0.8)
                ax.add_patch(circle)
            
            ax.set_title(position)
            
            ax.set_xticks([])
            ax.set_yticks([])
            
    if plot_data:
        fig.suptitle(foldername)
        fig.tight_layout()
        plt.savefig(path+os.sep+foldername+os.sep+"Plot.png") 
        plt.show()  
    
    if centerspot_intensity_fit:
        
        intensities = np.array(intensities)
        peak_positions = np.array(peak_positions)
        np.save(path+os.sep+foldername+os.sep+"Intensities.npy",intensities)
        #np.save(path+os.sep+foldername+os.sep+"Peak_positions.npy",peak_positions)
        
        #Plot the intensities and peak positions
        fig, (ax1,ax2) = plt.subplots(1,2,figsize=(10,5))
        ax1.plot(intensities[:,2])
        ax1.set_title("Intensity as function of measurement")#, variation: {:.2f}%".format(100*(np.max(intensities[:,2])-np.min(intensities[:,2]))/np.mean(intensities[:,2])))
        ax2.plot(peak_positions[:,2],peak_positions[:,3])
        ax2.set_title("Peak position variation")
        
        if len(intensities) == 16:
            #Measure sample Mueller matrix
            get_MM_from_calibration(image_acquisition_app_path+"calibration_values_633nm.npy",633,path+os.sep+foldername)
        