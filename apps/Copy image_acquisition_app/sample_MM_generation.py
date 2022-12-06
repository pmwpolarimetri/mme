"""
Script for generating sample Mueller matrices from measurements
Written by Vilde Vraalstad, last modified December 2022

Generates A- and W-matrices based on a calibration file, and finds the sample Mueller matrix from measurements.
The MM is saved to file at the same location as the datafile
"""

import numpy as np
import matplotlib.pyplot as plt
import lmfit
import math

def rad(x):
    return x*np.pi/180

#Expressions for W and A generated in Maple. To convert from Maple code generation to numpy, find and replace:
#   - math->np
#   - __ -> _

def get_W(t,psi,beta,delta,DeltaPsi_R,theta0,theta1,theta2,theta3,theta4):

    W1 = t / 2 * np.mat([-np.sin(2 * theta0 + 2 * theta1) * np.sin(2 * DeltaPsi_R) * np.sin(2 * beta) * np.cos(2 * psi) - np.cos(2 * theta0 + 2 * theta1) * np.sin(2 * DeltaPsi_R) * np.cos(2 * beta) * np.cos(2 * psi) + 1,-np.sin(2 * theta0 + 2 * theta2) * np.sin(2 * DeltaPsi_R) * np.sin(2 * beta) * np.cos(2 * psi) - np.cos(2 * theta0 + 2 * theta2) * np.sin(2 * DeltaPsi_R) * np.cos(2 * beta) * np.cos(2 * psi) + 1,-np.cos(2 * theta0 + 2 * theta3) * np.sin(2 * DeltaPsi_R) * np.cos(2 * beta) * np.cos(2 * psi) - np.sin(2 * theta0 + 2 * theta3) * np.sin(2 * DeltaPsi_R) * np.sin(2 * beta) * np.cos(2 * psi) + 1,-np.cos(2 * theta0 + 2 * theta4) * np.sin(2 * DeltaPsi_R) * np.cos(2 * beta) * np.cos(2 * psi) - np.sin(2 * theta0 + 2 * theta4) * np.sin(2 * DeltaPsi_R) * np.sin(2 * beta) * np.cos(2 * psi) + 1])

    W2 = t / 2 * np.mat([-np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) ** 2 + (-np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta1) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta1) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * beta),-np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) ** 2 + (-np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta2) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta2) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * beta),-np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) ** 2 + (-np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta3) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta3) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * beta),-np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) ** 2 + (-np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta4) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta4) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * beta)])
    
    W3 = t / 2 * np.mat([np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) ** 2 - np.sin(2 * theta0 + 2 * theta1) * np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) + np.sin(2 * beta) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta1),np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) ** 2 - np.sin(2 * theta0 + 2 * theta2) * np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) + np.sin(2 * beta) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta2),np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) ** 2 - np.sin(2 * theta0 + 2 * theta3) * np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) + np.sin(2 * beta) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta3),np.sin(2 * beta) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) ** 2 - np.sin(2 * theta0 + 2 * theta4) * np.cos(2 * psi) * np.cos(2 * beta) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) + np.sin(2 * beta) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta4)])

    W4 = t / 2 * np.mat([np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.sin(2 * theta0 + 2 * theta1) * np.cos(2 * beta) - np.cos(2 * theta0 + 2 * theta1) * np.sin(2 * beta)),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.sin(2 * theta0 + 2 * theta2) * np.cos(2 * beta) - np.cos(2 * theta0 + 2 * theta2) * np.sin(2 * beta)),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.cos(2 * beta) * np.sin(2 * theta0 + 2 * theta3) - np.sin(2 * beta) * np.cos(2 * theta0 + 2 * theta3)),np.cos(2 * DeltaPsi_R) * np.sin(delta) * np.cos(2 * psi) * (np.sin(2 * theta0 + 2 * theta4) * np.cos(2 * beta) - np.cos(2 * theta0 + 2 * theta4) * np.sin(2 * beta))])
    
    W = np.c_[W1,W2,W3,W4].reshape(4,4)
    W_inv = np.linalg.inv(W)
    W_cond = np.linalg.cond(W)
    
    return W,W_inv,W_cond

def get_A(t,psi,alpha,delta,DeltaPsi_R,theta0,theta1,theta2,theta3,theta4):
    A1 = t / 2 * np.mat([-np.sin(2 * theta0 + 2 * theta1) * np.sin(2 * DeltaPsi_R) * np.sin(2 * alpha) * np.cos(2 * psi) - np.cos(2 * theta0 + 2 * theta1) * np.sin(2 * DeltaPsi_R) * np.cos(2 * alpha) * np.cos(2 * psi) + 1,-np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) ** 2 + (-np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta1) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta1) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * alpha),np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) ** 2 - np.sin(2 * theta0 + 2 * theta1) * np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta1) + np.sin(2 * alpha) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta1),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.cos(2 * theta0 + 2 * theta1) * np.sin(2 * alpha) - np.sin(2 * theta0 + 2 * theta1) * np.cos(2 * alpha))])

    A2 = t / 2 * np.mat([-np.sin(2 * theta0 + 2 * theta2) * np.sin(2 * DeltaPsi_R) * np.sin(2 * alpha) * np.cos(2 * psi) - np.cos(2 * theta0 + 2 * theta2) * np.sin(2 * DeltaPsi_R) * np.cos(2 * alpha) * np.cos(2 * psi) + 1,-np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) ** 2 + (-np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta2) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta2) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * alpha),np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) ** 2 - np.sin(2 * theta0 + 2 * theta2) * np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta2) + np.sin(2 * alpha) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta2),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.cos(2 * theta0 + 2 * theta2) * np.sin(2 * alpha) - np.sin(2 * theta0 + 2 * theta2) * np.cos(2 * alpha))])

    A3 = t / 2 * np.mat([-np.cos(2 * theta0 + 2 * theta3) * np.sin(2 * DeltaPsi_R) * np.cos(2 * alpha) * np.cos(2 * psi) - np.sin(2 * theta0 + 2 * theta3) * np.sin(2 * DeltaPsi_R) * np.sin(2 * alpha) * np.cos(2 * psi) + 1,-np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) ** 2 + (-np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta3) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta3) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * alpha),np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) ** 2 - np.sin(2 * theta0 + 2 * theta3) * np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta3) + np.sin(2 * alpha) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta3),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.sin(2 * alpha) * np.cos(2 * theta0 + 2 * theta3) - np.cos(2 * alpha) * np.sin(2 * theta0 + 2 * theta3))])

    A4 = t / 2 * np.mat([-np.cos(2 * theta0 + 2 * theta4) * np.sin(2 * DeltaPsi_R) * np.cos(2 * alpha) * np.cos(2 * psi) - np.sin(2 * theta0 + 2 * theta4) * np.sin(2 * DeltaPsi_R) * np.sin(2 * alpha) * np.cos(2 * psi) + 1,-np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) ** 2 + (-np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.sin(2 * theta0 + 2 * theta4) - np.sin(2 * DeltaPsi_R)) * np.cos(2 * theta0 + 2 * theta4) + np.cos(delta) * np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.cos(2 * alpha),np.sin(2 * alpha) * np.cos(2 * psi) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) ** 2 - np.sin(2 * theta0 + 2 * theta4) * np.cos(2 * psi) * np.cos(2 * alpha) * (np.cos(2 * DeltaPsi_R) * np.cos(delta) - 1) * np.cos(2 * theta0 + 2 * theta4) + np.sin(2 * alpha) * np.cos(2 * psi) - np.sin(2 * DeltaPsi_R) * np.sin(2 * theta0 + 2 * theta4),np.cos(2 * psi) * np.cos(2 * DeltaPsi_R) * np.sin(delta) * (np.cos(2 * theta0 + 2 * theta4) * np.sin(2 * alpha) - np.sin(2 * theta0 + 2 * theta4) * np.cos(2 * alpha))])

    A = np.c_[A1,A2,A3,A4].reshape(4,4)
    A_inv = np.linalg.inv(A)
    A_cond = np.linalg.cond(A)
    
    return A,A_inv,A_cond


def get_MM_from_calibration(calibrationfile,calibrationwavelength,datapath):

    calibration_values = np.load(calibrationfile,allow_pickle=True).item()
    
    t = calibration_values["t"]
    psi_W = calibration_values["psi_W"]
    psi_A = calibration_values["psi_A"]
    beta = calibration_values["beta"]
    alpha = calibration_values["alpha"]
    delta_RA = calibration_values["delta_RA"]
    delta_RW = calibration_values["delta_RW"]
    DeltaPsi_W = calibration_values["DeltaPsi_W"]
    DeltaPsi_A = calibration_values["DeltaPsi_A"]
    theta_W0 = calibration_values["theta_W0"]
    theta_A0 = calibration_values["theta_A0"]
    theta_disp = 0#calibration_values["theta_disp"]
    delta_PW = calibration_values["delta_PW"]
    delta_PA = calibration_values["delta_PA"]
    
    scale_result = calibration_values["scale_result"]
    
    
    W,W_inv,W_cond = get_W(t,psi_W,beta,delta_RW,DeltaPsi_W,theta_W0+theta_disp,rad(-51.7),rad(-15.1),rad(15.1),rad(51.7))
    print("\n\nW:\n",W,"\n\nW_inv:\n",W_inv,"\n\n W condition number:",W_cond)
    
    A,A_inv,A_cond = get_A(t,psi_A,alpha,delta_RA,DeltaPsi_A,theta_A0+theta_disp,rad(-51.7),rad(-15.1),rad(15.1),rad(51.7))
    print("\n\nA:\n",A,"\n\nA_inv:\n",A_inv,"\n\n A condition number:",A_cond)


    """ Sample Mueller matrix """
    
    intensities = np.load(datapath+"/Intensities.npy")
    intensities = intensities[np.lexsort((intensities[:,1],intensities[:,0]))]
    
    B = intensities[:,2].reshape(4,4)
    
    Msample = A_inv@B@W_inv
    Msample = Msample/Msample[0,0]
    
    print("\n\nMeasured MM for air, using the calibration for " + str(calibrationwavelength) + "nm:")
    print(Msample)
    
    np.savetxt(datapath+"/Measured MM for " + str(calibrationwavelength) + "nm calibration.txt",Msample)


""" Testing """

#The generated ideal matrices look correct, and give the ideal condition numner sqrt(3) = 1.73

# S0 = np.array([[1],[0],[0],[0]])
# Mair = np.identity(4)

# def rotation(theta):
#     return np.mat([
#         [1,0, 0,0],
#         [0,np.cos(2*theta), np.sin(2*theta),0],
#         [0,-np.sin(2*theta), np.cos(2*theta),0],
#         [0,0, 0,1]
#         ])

# def general_element(A,theta,psi,delta):
#     unrotated = np.mat([
#         [1,np.cos(2*psi), 0,0],
#         [np.cos(2*psi),1,0,0],
#         [0,0, np.sin(2*psi)*np.cos(delta),np.sin(2*psi)*np.sin(delta)],
#         [0,0,-np.sin(2*psi)*np.sin(delta),np.sin(2*psi)*np.cos(delta)]
#         ])
#     return A*rotation(-theta)@unrotated@rotation(theta)

# def Mpol(t,psi,theta,delta):
#     return general_element(t/2,theta,psi,delta)

# def Mret(t,delta,theta,DeltaPsi):
#     return general_element(t,theta,np.pi/4+DeltaPsi,delta)

# #Ideal matrices
# print("W_ideal:")
# W,W_inv,W_cond = get_W(1,0,0,rad(132),0,0,rad(-15.1964),rad(15.1964),rad(-51.7076),rad(51.7076))
# print(W,"\n\n W_ideal_inv:\n",W_inv,"\n\n Condition number for W_ideal:",W_cond,"\n\n")

# print("A_ideal:")
# A,A_inv,A_cond = get_A(1,0,0,rad(132),0,0,rad(-15.1964),rad(15.1964),rad(-51.7076),rad(51.7076))
# print(A,"\n\n A_ideal_inv:\n",A_inv,"\n\n Condition number for A_ideal:",A_cond,"\n\n")

# print("Simulated B-matrix for air:")
# B0 = A@Mair@W
# print(B0,"\n\n")

# print("Simulated B-matrix for polarizer with angle 0:")
# B1 = A@Mpol(1,0,0,0)@W
# print(B1,"\n\n")