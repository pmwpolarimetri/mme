"""
Script for performing intensity fit calibration for the Mueller matrix microscope in D4-112
Written by Vilde Vraalstad, last modified December 2022


Use the parametrized intensity model generated in Maple and optimize the components based on the measured intensity.
The data are aquired using a rotation ratio PSA:PSG 5:1, which is commonly used and found to minimize the condition number.
"""

import numpy as np
import matplotlib.pyplot as plt
import lmfit
import math
import scipy
import scipy.ndimage

def rad(x):
    return x*np.pi/180


""" Measurements of dispersion of the retarder, both in redtardance and orientation """

def delta_R_meas(nm):
    eV = 1240/nm
    mean = 129.0005+2.29074*eV-0.43863*eV**2+0.11198*eV**3
    min = mean-0.01952-0.02866*eV-0.01293*eV**2-0.00182*eV**3
    max = mean+0.01952+0.02866*eV+0.01293*eV**2+0.00182*eV**3
    
    return rad(mean),rad(min),rad(max)

def optical_rotation_R(nm):
    eV = 1240/nm
    mean = 0.11584+0.12231*eV-0.0755*eV**2+0.01223*eV**3
    min = mean-0.00013-0.0121*eV-0.00458*eV**2-0.000507*eV**3
    max = mean+0.00013+0.0121*eV+0.00458*eV**2+0.000507*eV**3
    return [rad(mean),rad(min),rad(max)]

def orientation_dispersion(nm):
    eV = 1240/nm
    mean = 0.90001+0.02182*eV+0.01272*eV**2
    min = mean-0.00312-0.00221*eV-0.0000159*eV**2
    max = mean+0.00312+0.00221*eV+0.0000159*eV**2
    return [rad(mean),rad(min),rad(max)]

def Delta_theta_R_meas(nm):
    rot = optical_rotation_R(nm)
    disp = orientation_dispersion(nm)
    mean = rot[0]+disp[0]
    min = rot[1]+disp[1]
    max = rot[2]+disp[2]
    return mean, min, max

""" Model generated in Maple """

#Expressions for W and A generated in Maple. To convert from Maple code generation to numpy, find and replace:
#   - math->np
#   - __ -> _

def Isim_plain(wavelength,t, psi_A, psi_W, alpha, beta, delta_PA, delta_PW, theta_A0, theta_W0, theta_A, theta_W,delta_RA,delta_RW, DeltaPsi_A, DeltaPsi_W):
    return -t * ((np.cos(2 * DeltaPsi_W) * np.cos(delta_RA) - 1) * ((-np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) + 1) * np.cos(2 * psi_W) * np.cos(2 * theta_W + 2 * theta_W0 ) ** 2 - np.cos(2 * alpha) * np.sin(2 * DeltaPsi_W) * np.cos(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * alpha) * np.sin(2 * DeltaPsi_W) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.cos(2 * psi_W) * (-1 + (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) + 1) * np.cos(2 * alpha) ** 2)) * np.cos(2 * psi_A) * np.cos(2 * theta_A + 2 * theta_A0 ) ** 2 + (-np.cos(2 * alpha) * np.cos(2 * psi_W) * np.sin(2 * DeltaPsi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * np.cos(2 * theta_W + 2 * theta_W0 ) ** 2 + (-(np.cos(2 * psi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * alpha) * np.sin(2 * DeltaPsi_W)) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RA) - 1) * np.cos(2 * psi_A) * np.sin(2 * theta_A + 2 * theta_A0 ) - np.sin(2 * alpha) * np.cos(2 * psi_W) * np.sin(2 * DeltaPsi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * np.sin(2 * theta_W + 2 * theta_W0 ) - ((np.sin(delta_RA) * np.sin(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W) + np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.cos(2 * alpha) ** 2 - np.sin(delta_RA) * np.sin(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W)) * np.cos(2 * psi_W) * np.cos(2 * psi_A) - np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.cos(2 * theta_W + 2 * theta_W0 ) + np.cos(2 * alpha) * ((-np.sin(2 * theta_W + 2 * theta_W0 ) * np.sin(2 * DeltaPsi_W) + np.sin(2 * alpha) * np.cos(2 * psi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) + 1)) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RA) - 1) * np.cos(2 * psi_A) * np.sin(2 * theta_A + 2 * theta_A0 ) - np.sin(2 * alpha) * np.cos(2 * psi_A) * np.cos(2 * psi_W) * (np.sin(delta_RA) * np.sin(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W) + np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * DeltaPsi_W) * (np.cos(delta_RW) * np.cos(2 * psi_W) * np.cos(2 * DeltaPsi_W) + np.cos(2 * psi_A)))) * np.cos(2 * theta_A + 2 * theta_A0 ) + (np.sin(2 * alpha) * np.sin(2 * DeltaPsi_W) * np.sin(2 * theta_A + 2 * theta_A0 ) + (-1 + (np.cos(2 * DeltaPsi_W) * np.cos(delta_RA) + 1) * np.cos(2 * alpha) ** 2) * np.cos(2 * psi_A)) * np.cos(2 * psi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * np.cos(2 * theta_W + 2 * theta_W0 ) ** 2 + (-(np.sin(2 * DeltaPsi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * alpha) * np.cos(2 * psi_A) * (np.sin(delta_RA) * np.sin(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W) + np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W))) * np.cos(2 * psi_W) * np.sin(2 * theta_A + 2 * theta_A0 ) + np.sin(2 * alpha) * np.cos(2 * psi_A) * np.cos(2 * psi_W) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RW) - 1) * (np.cos(2 * DeltaPsi_W) * np.cos(delta_RA) + 1) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * DeltaPsi_W) * (np.cos(delta_RA) * np.cos(2 * psi_A) * np.cos(2 * DeltaPsi_W) + np.cos(2 * psi_W))) * np.cos(2 * alpha) * np.cos(2 * theta_W + 2 * theta_W0 ) + ((np.cos(2 * psi_W) * ((np.sin(delta_RA) * np.sin(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W) + np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.cos(2 * alpha) ** 2 - np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.cos(2 * psi_A) - np.sin(2 * DeltaPsi_W) * np.sin(2 * DeltaPsi_W)) * np.sin(2 * theta_W + 2 * theta_W0 ) + np.sin(2 * alpha) * np.sin(2 * DeltaPsi_W) * (np.cos(2 * psi_A) + np.cos(2 * psi_W))) * np.sin(2 * theta_A + 2 * theta_A0 ) + np.sin(2 * alpha) * np.sin(2 * DeltaPsi_W) * (np.cos(2 * psi_A) + np.cos(2 * psi_W)) * np.sin(2 * theta_W + 2 * theta_W0 ) - 1 - (1 + (np.cos(delta_RA) * np.cos(delta_RW) * np.cos(2 * DeltaPsi_W) * np.cos(2 * DeltaPsi_W) - 1) * np.cos(2 * alpha) ** 2) * np.cos(2 * psi_W) * np.cos(2 * psi_A)) / 4

def Isim(wavelength,t, psi_A, psi_W, alpha, beta, delta_PA, delta_PW, theta_A0, theta_W0, theta_A, theta_W,delta_RA,delta_RW, DeltaPsi_A, DeltaPsi_W):
    return Isim_plain(wavelength,t, psi_A, psi_W, alpha, beta, delta_PA, delta_PW, theta_A0, theta_W0, theta_A, theta_W,delta_RA,delta_RW, DeltaPsi_A, DeltaPsi_W)/Isim_plain(wavelength,t, psi_A, psi_W, alpha, beta, delta_PA, delta_PW, theta_A0, theta_W0, 0, 0,delta_RA,delta_RW, DeltaPsi_A, DeltaPsi_W)

def Isim_simple(wavelength,t, alpha, beta, theta_A0, theta_W0, theta_A, theta_W, delta_RA,delta_RW):
    return Isim(wavelength,t, 0, 0, alpha, beta, 0,0, theta_A0, theta_W0, theta_A, theta_W, delta_RA,delta_RW, 0,0)

def make_model(function,Imeas,param_list,theta_first_polarizer, theta_A,theta_W,wavelength,t=1,scale_result=[1,0]):
    model = lmfit.Model(function,independent_vars=["theta_A","theta_W","beta","wavelength","t"])
    
    params = model.make_params()
    for param in param_list:
        params.add(param[0],value=param[1],min=param[2],max=param[3])
    
    result = model.fit(Imeas,params,t=t,beta=theta_first_polarizer,theta_A=theta_A,theta_W=theta_W,wavelength=wavelength)

    x = np.arange(len(Imeas))
    
    Isim = result.best_fit        

    plt.plot(x, Imeas,'b-',label="Measured")
    plt.plot(x, Isim*scale_result[0]+scale_result[1], 'r-',label="Fitted")
    plt.legend()
    plt.show()
    
    print(result.fit_report())
    
    fitted_params = result.params
    
    return fitted_params
    
def calibration(file,wavelength,path_to_save,function=Isim_simple):
    """
    Perform a calibration using the calibration data in file (measurements with rotation increments PSG:PSA 1:5) at the specified wavelength,
    fitting the measured intensitites to function Isim_simple or Isim.
    """
    
    intensities = np.load(file)
    intensities = intensities[np.argsort(intensities[:,1])]
    
    Imeas = intensities[:,2]
    Imeas = Imeas/Imeas[0]
    
    # Can choose to normalize with respect to each peak instead of only the first peak.
    # Must then find the peak posisions, and use the printed values to normalize
    
    # Imeas_copy = Imeas#scipy.ndimage.gaussian_filter(Imeas,5)
    # peaks, _ = scipy.signal.find_peaks(Imeas_copy, height=0.6)
    # plt.figure()
    # plt.plot(Imeas_copy)
    # plt.plot(peaks,Imeas_copy[peaks],"x")
    # print(peaks)
    # print(np.argmax(Imeas))
    # plt.figure()
    
#    interval = [0,20,55,75,100,145,170,190,240,260,280,325,361]
#    for i in range(len(interval)-1):
#        Imeas[interval[i]:interval[i+1]] = Imeas[interval[i]:interval[i+1]]/np.max(Imeas[interval[i]:interval[i+1]])
    
    
    theta_W = rad(intensities[:,0])
    theta_A = rad(intensities[:,1])
    
    delta_R, delta_R_min, delta_R_max = delta_R_meas(wavelength)
    theta_disp, theta_disp_min, theta_disp_max = Delta_theta_R_meas(wavelength)
    theta_first_polarizer = 0
    
    if function == Isim_simple:
    
        #Add parameters: name,value,min,max
        param_list = []
        param_list.append(["alpha",0,0,np.pi/2])
        param_list.append(["theta_A0",theta_disp,-rad(10)+theta_disp_min,rad(10)+theta_disp_max])
        param_list.append(["theta_W0",theta_disp,-rad(10)+theta_disp_min,rad(10)+theta_disp_max])
        param_list.append(["delta_RA",delta_R,delta_R_min,delta_R_max])
        param_list.append(["delta_RW",delta_R,delta_R_min,delta_R_max])
        scale_result=[1,0]
            
        fitted_params = make_model(function,Imeas,param_list,theta_first_polarizer=theta_first_polarizer,theta_A=theta_A,theta_W=theta_W,wavelength=wavelength,scale_result=scale_result)
        calibration_values = fitted_params.valuesdict()
        calibration_values["t"] = 1
        calibration_values["scale_result"] = scale_result
        calibration_values["beta"] = theta_first_polarizer
        calibration_values["psi_A"] = 0
        calibration_values["psi_W"] = 0
        calibration_values["delta_PA"] = 0
        calibration_values["delta_PW"] = 0
        calibration_values["DeltaPsi_A"] = 0
        calibration_values["DeltaPsi_W"] = 0
        
    elif function == Isim:
        #Add parameters: name,value,min,max
        param_list = []
        #param_list.append(["t",0.2,0.01,1])
        param_list.append(["psi_A",0,0,np.pi/2])
        param_list.append(["psi_W",0,0,np.pi/2])
        param_list.append(["alpha",0,0,np.pi/2])
        param_list.append(["delta_RA",delta_R,delta_R_min,delta_R_max])
        param_list.append(["delta_RW",delta_R,delta_R_min,delta_R_max])
        #param_list.append(["theta_disp",theta_disp, theta_disp_min, theta_disp_max])
        param_list.append(["delta_PA",0,-np.pi/2,np.pi/2])
        param_list.append(["delta_PW",0,-np.pi/2,np.pi/2])
        param_list.append(["theta_A0",theta_disp,-rad(10)+theta_disp_min,rad(10)+theta_disp_max])
        param_list.append(["theta_W0",theta_disp,-rad(10)+theta_disp_min,rad(10)+theta_disp_max])
        param_list.append(["DeltaPsi_A",0,-np.pi/4,np.pi/4])
        param_list.append(["DeltaPsi_W",0,-np.pi/4,np.pi/4])
        scale_result=[1,0]
            
        fitted_params = make_model(function,Imeas,param_list,theta_first_polarizer=theta_first_polarizer,theta_A=theta_A,theta_W=theta_W,wavelength=wavelength,scale_result=scale_result)
        calibration_values = fitted_params.valuesdict()
        calibration_values["t"] = 1
        calibration_values["scale_result"] = scale_result
        calibration_values["wavelength"] = wavelength
        calibration_values["beta"] = theta_first_polarizer
    
    np.save(path_to_save+"/calibration_values_"+str(wavelength)+"nm.npy",calibration_values)
    
    
def test_model(datafile, calibrationfile, data_wavelength, calibration_wavelength):
    """
    Test calibration parameters of one wavelength with data from other wavelengths, to see if the included dispersion functions look correct.
    
    datafile: path of file to test, with data at data_wavelength
    calibrationfile: path to file with calibration values at calibration_wavelength
    
    """
    
    calibration_values = np.load(calibrationfile,allow_pickle=True).item()
    intensities = np.load(datafile)
    intensities = intensities[np.argsort(intensities[:,1])]
    Imeas = intensities[:,2]
    Imeas = Imeas/Imeas[0]
    
    theta_W = rad(intensities[:,0])
    theta_A = rad(intensities[:,1])
    
    t = calibration_values["t"]
    psi_W = calibration_values["psi_W"]
    psi_A = calibration_values["psi_A"]
    beta = calibration_values["beta"]
    alpha = calibration_values["alpha"]
    delta_RA = calibration_values["delta_RA"]+delta_R_meas(data_wavelength)[0]-delta_R_meas(calibration_wavelength)[0]
    delta_RW = calibration_values["delta_RW"]+delta_R_meas(data_wavelength)[0]-delta_R_meas(calibration_wavelength)[0]
    DeltaPsi_W = calibration_values["DeltaPsi_W"]
    DeltaPsi_A = calibration_values["DeltaPsi_A"]
    theta_W0 = calibration_values["theta_W0"]+Delta_theta_R_meas(data_wavelength)[0]-Delta_theta_R_meas(calibration_wavelength)[0]
    theta_A0 = calibration_values["theta_A0"]+Delta_theta_R_meas(data_wavelength)[0]-Delta_theta_R_meas(calibration_wavelength)[0]
        
    delta_PW = calibration_values["delta_PW"]
    delta_PA = calibration_values["delta_PA"]

    scale_result = calibration_values["scale_result"]
    
    Isim_res = Isim(data_wavelength,t, psi_A, psi_W, alpha, beta, delta_PA, delta_PW, theta_A0, theta_W0, theta_A, theta_W,delta_RA,delta_RW, DeltaPsi_A, DeltaPsi_W)
    
    plt.plot(np.arange(len(Isim_res)), Imeas[:len(Isim_res)],'b-',label="Measured")
    plt.plot(np.arange(len(Isim_res)), Isim_res*scale_result[0]+scale_result[1], 'r-',label="Fitted")
    plt.legend()
    plt.show()