# In[1]:

# Load required modules ===============================================================
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.lines as mlines
import matplotlib as mpl
import sys
reload(sys)
sys.setdefaultencoding('utf8')

data_distribution = None
model_distribution = None
data_weekly = None
model_weekly = None
S = None
I = None
R = None

# In[2]:

# Load data/Read in CSV files ===============================================================
def read(_data_distribution, _model_distribution, _data_weekly, _model_weekly, _S, _I, _R):
	global data_distribution, model_distribution, data_weekly, model_weekly, S, I, R

	data_distribution = pd.read_csv(_data_distribution)
	model_distribution = pd.read_csv(_model_distribution)
	data_weekly = pd.read_csv(_data_weekly)
	model_weekly = pd.read_csv(_model_weekly)
	S = pd.read_csv(_S)
	I = pd.read_csv(_I)
	R = pd.read_csv(_R)

	# Take out the relevant age groups
	model_distribution = model_distribution[0:5]
	data_distribution = data_distribution[0:5]


# In[3]:

# Plot the graphs ===============================================================
def plot(_output): 
	global data_distribution, model_distribution, data_weekly, model_weekly, S, I, R

	mpl.style.use('classic') # Use classic MPL layout
	fig = plt.figure() # add plot figure
	fig.add_subplot(233)
	# Plot all columns for R
	for i in range(1, len(R.columns)):
	    plt.plot(R['Period'], R.iloc[:, i], color = '#787878')
	plt.xlabel('Period')
	plt.title('R')
	fig.add_subplot(232)
	# Plot all columns for I
	for i in range(1, len(I.columns)):
	    plt.plot(I['Period'], I.iloc[:, i], color = '#787878')
	plt.xlabel('Period')
	plt.title('I')
	fig.add_subplot(231)
	# Plot all columns for S
	for i in range(1, len(S.columns)):
	    plt.plot(S['Period'], S.iloc[:, i], color = '#787878')
	plt.xlabel('Period')
	plt.title('S')
	fig.add_subplot(234)
	# Plot all columns for weekly data and overlay model
	for i in range(1, len(model_weekly.columns)):
	    plt.plot(model_weekly['Time'], model_weekly.iloc[:, i], color = '#787878')
	for i in range(1, len(data_weekly.columns)):
	    plt.plot(data_weekly['Time'], data_weekly.iloc[:, i], color = 'black',
	             linewidth = '2', marker = 'o', markersize = '5')
	plt.xlabel('Time')
	plt.title('Model')
	ax = fig.add_subplot(235)
	x_axis = np.arange(1, 6) # create y-axis coordinates for scatter plot (dataset) points
	# Plot all columns for age distribution data
	for i in range(0, len(model_distribution)):
	    for j in range(1, len(model_distribution.columns)):
	        plt.scatter(x_axis[i], model_distribution.iloc[:, j][i], marker = '_', color = '#C74EFF', s = 200) # plot a scatter plot with dataset values
	        plt.scatter(x_axis[i], data_distribution['%'][i], marker = '_', color = '#000EFF', s = 200) # plot a scatter plot with dataset values
	ax.set_xticks(x_axis)
	ax.set_xticklabels(model_distribution['Age'],  fontsize = 6)
	plt.yticks(fontsize = 6)
	plt.xlabel('Age Group')
	plt.ylabel('Age Distribution')
	plt.title('Distribution')
	plt.tight_layout()
	plt.savefig(_output)
