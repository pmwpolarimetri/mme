from analyse_and_plot_function import *

pathinput = str(sys.argv[3])
foldername = pathinput.split('/')[-1]
path = pathinput.replace('/'+foldername,'')

kspace = False
if "k-space" in foldername:
    kspace=True

analyse_and_plot(path,foldername,kspace=kspace)