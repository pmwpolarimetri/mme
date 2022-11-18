import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import scipy
import scipy.ndimage
import sys
from glob import glob
import os
import math

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

def do_gaussian_2d_fit(file,dark_im,plot=True):
    im: np.ndarray = np.load(file)
    
    if np.array_equal(dark_im,np.ndarray(shape=(512,512))) == False:
        im = im/dark_im
    
    #Find position of maximum inside the expected area
    h,w=im.shape
    #circular_mask = create_circular_mask(h, w, center=(h/2,w/2),radius=0.2*h)
    circular_mask = create_circular_mask(h, w, center=(250,320),radius=0.2*h)
    center = np.copy(im)
    center = scipy.ndimage.gaussian_filter(center, 3) #Smooth to remove small spots
    center[~circular_mask] = 0
    max_idx = np.flip(np.unravel_index(np.argsort(center.ravel())[-1], center.shape))
    
    #Choose to fit only the peak and the nearest surroundings,
    #10 pixels outside where the peak drops to 10% of its maximum, assuming a circular beam
    peak_pixel_width = np.where(im[max_idx[1]:,max_idx[0]]-np.median(im)<0.1*(im[max_idx[1],max_idx[0]]-np.median(im)))[0][0]+10 #100
    x_min = max_idx[0]-peak_pixel_width
    x_max = max_idx[0]+peak_pixel_width
    y_min = max_idx[1]-peak_pixel_width
    y_max = max_idx[1]+peak_pixel_width
    im = im[y_min:y_max,x_min:x_max]
    h,w = im.shape
    
    # #Can choose to smooth the signal using a Gaussian filter
    # im = scipy.ndimage.gaussian_filter(im, 3)
    
    [x0,y0,sigma,A,offset],im_fit = gaussian_2d_fit(im)
    
    #Remove background signal
    im = im-offset
    im_fit = im_fit-offset
    
    if plot:
        
        fig, (ax1,ax2,ax3) = plt.subplots(1,3,figsize=(15,5))
        
        
        #Plot the sum of the beam along y, and the center plane along y, as a function of x-position,
        #and vice versa
        
        x = np.arange(h)
        y = np.arange(w)
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
        
        
        #3d-plot of the signal, to indicate the shape of the beam

        # fig = plt.figure()
        # ax = fig.add_subplot(111, projection='3d')

        # x = np.arange(0, im.shape[0], 1)
        # y = np.arange(0, im.shape[1], 1)
        # xx,yy = np.meshgrid(x,y)

        # ax.plot_surface(xx,yy,im)
        # plt.show()
    
    #Calculate the total intensity inside fitted gaussian beam
    #Can use the width 2*sigma: radius = sigma
    #Can use the radius to FWHM: radius = sigma*np.sqrt(2*np.log(2))
    #Can use the radius to account for 91% of the beam intensity: radius = 2*sigma
    circular_mask = create_circular_mask(h, w, center=(x0,y0),radius=sigma)
    center_spot = np.copy(im_fit)
    center_spot[~circular_mask] = 0
    #total_intensity = np.sum(center_spot)
    total_intensity = np.max(center_spot)
    
    return total_intensity, [x0,y0,sigma,A,offset], im_fit, im

def analyse(path,foldername,kspace=False):
    if kspace:
        files = glob(path+os.sep+foldername+os.sep+"*.npy")
        while (path+os.sep+foldername+os.sep+"Intensities.npy") in files:
            files.remove((path+os.sep+foldername+os.sep+"Intensities.npy"))
        while (path+os.sep+foldername+os.sep+"Peak_positions.npy") in files:
            files.remove((path+os.sep+foldername+os.sep+"Peak_positions.npy"))
        
        intensities = []
        peak_positions = []
        
        dark_im = np.ndarray(shape=(512,512))
        for file in files:
            if ("Dark measurement") in file:
                files.remove(file)
                dark_im: np.ndarray = np.load(file)

        for file in files:
            filename = file.replace(path+os.sep+foldername+os.sep,"").replace(".npy","")
            position = filename[:filename.find("Wl")]
            PSG_pos = float(position[position.find("PSG")+3:position.find("PSA")])
            PSA_pos = float(position[position.find("PSA")+3:])
            total_intensity, [x0,y0,sigma,A,offset], im_fit, im = do_gaussian_2d_fit(file,dark_im,plot=False)#,plot=True)
            intensities.append([PSG_pos,PSA_pos,round(total_intensity,2)])
            peak_positions.append([PSG_pos,PSA_pos,x0,y0]) 
        
        intensities = np.array(intensities)
        peak_positions = np.array(peak_positions)
        
        np.save(path+os.sep+foldername+os.sep+"Intensities.npy",intensities)
        np.save(path+os.sep+foldername+os.sep+"Peak_positions.npy",peak_positions)
        
        fig, (ax1,ax2) = plt.subplots(1,2,figsize=(10,5))
        ax1.plot(intensities[:,2])
        ax1.set_title("Intensity as function of measurement")
        ax2.plot(peak_positions[:,2],peak_positions[:,3])
        ax2.set_title("Peak position variation")
       
        # #Print the intensities and peak positions
        # for i in range(len(intensities)):
        #     intensity = intensities[i]
        #     peak_pos = peak_positions[i]
        #     print("PSG:",intensity[0],"\nPSA:",intensity[1],"\nIntensity:",intensity[2],"\nPeak position: x =", peak_pos[2], "y =", peak_pos[3], "\n")
        
        
def analyse_and_plot(path,foldername,kspace=False):
    
    files = glob(path+os.sep+foldername+os.sep+"*.npy")
    while (path+os.sep+foldername+os.sep+"Intensities.npy") in files:
        files.remove((path+os.sep+foldername+os.sep+"Intensities.npy"))
    while (path+os.sep+foldername+os.sep+"Peak_positions.npy") in files:
        files.remove((path+os.sep+foldername+os.sep+"Peak_positions.npy"))

    intensities = []
    peak_positions = []
    
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
    
    dark_im = np.ndarray(shape=(512,512))
    for file in files:
        if ("dark measurement") in file:
            files.remove(file)
            dark_im: np.ndarray = np.load(file)
    saturation = False
    for i in range(len(files)):
        file = files[i]
        filename = file.replace(path+os.sep+foldername+os.sep,"").replace(".npy","")
        position = filename[:filename.find("Wl")]
        PSG_pos = float(position[position.find("PSG")+3:position.find("PSA")])
        PSA_pos = float(position[position.find("PSA")+3:])
        
        im: np.ndarray = np.load(file)
        if np.array_equal(dark_im,np.ndarray(shape=(512,512))) == False:
            im = im/dark_im
        im = im[250:420,170:343]
        if saturation == False:
            if np.max(im) > 4090:
                print("\nWARNING: The camera is saturated, and the exposure time must be reduced")
                saturation = True
            elif np.max(im) > 4000:
                print("\nVery close to reaching saturation, with largest intensities", np.flip(np.sort(im,axis=None))[:5])
                print("Should reduce the exposure time")
                
        if kspace:
            total_intensity, [x0,y0,sigma,A,offset], im_fit, im = do_gaussian_2d_fit(file,dark_im,plot=False)
            intensities.append([PSG_pos,PSA_pos,round(total_intensity,2)])
            peak_positions.append([PSG_pos,PSA_pos,x0,y0])
        
        if fig_rows == 1 or fig_columns == 1:
            ax = axs[i]
        else:
            ax = axs[i//fig_rows,i%fig_rows]
            
        ax.imshow(im)
        
        if kspace:
            circle = plt.Circle((x0,y0),sigma, facecolor='none', edgecolor="red", linewidth=1, alpha=0.8)
            ax.add_patch(circle)
        
        ax.set_title(position)
        
        ax.set_xticks([])
        ax.set_yticks([])
        
    fig.suptitle(foldername)
    fig.tight_layout()
    plt.savefig(path+os.sep+foldername+os.sep+"Plot.png") 
    plt.show()
    
    if kspace:
        
        intensities = np.array(intensities)
        peak_positions = np.array(peak_positions)
        np.save(path+os.sep+foldername+os.sep+"Intensities.npy",intensities)
        np.save(path+os.sep+foldername+os.sep+"Peak_positions.npy",peak_positions)
        
        fig, (ax1,ax2) = plt.subplots(1,2,figsize=(10,5))
        ax1.plot(intensities[:,2])
        ax1.set_title("Intensity as function of measurement")
        ax2.plot(peak_positions[:,2],peak_positions[:,3])
        ax2.set_title("Peak position variation")
        
        # #Print the intensities and peak positions
        # for i in range(len(intensities)):
        #     intensity = intensities[i]
        #     peak_pos = peak_positions[i]
        #     print("PSG:",intensity[0],"\nPSA:",intensity[1],"\nIntensity:",intensity[2],"\nPeak position: x =", peak_pos[2], "y =", peak_pos[3], "\n")
