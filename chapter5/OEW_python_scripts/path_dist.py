#!/usr/bin/python
import numpy as np
from datetime import datetime
import os
import sys

#This script takes each individual animal path and outputs the total distance traveled after each new signal.
#Oliver Edgar Williams for Julie Freeman, Chris Faulkes, RAT.systems 2017


#Function to convert the strings representing date and time to a datetime object for sorting.
def strng_to_dt(time_string):
	try:
		new_time = datetime.strptime(time_string,'%Y-%m-%d %H:%M:%S.%f')
	except:
		new_time = datetime.strptime(time_string,'%Y-%m-%d %H:%M:%S')

	return new_time
	
	
#Function to produce distance path from sensor path.
def get_dist_path(path,d_array):
	
	p_len = len(path)
	
	d_path = [path[0]+[0]]			#Initial point in path, zero distance traveled.
	
	#print d_path
	
	d_total = 0							#Initialise total distance.
	source = path[0][1]
	
	for x in range(1,p_len):
		target = path[x][1]
		
		dist_travelled = d_array[source,target]
		d_total = dist_travelled
		
		new_entry = path[x]+[d_total]
		d_path.append(new_entry)
		
		source = target
		
	return(d_path)
	
	
	

#Main.

dist_array = np.zeros((7,7),dtype=np.int)	#Initialise array for distances.

#Get distance values.
dist_f = open("nmr_nest_distances_edit.csv",'r')

for line in dist_f:
	dum_line = line.split(",")
	
	source = int(dum_line[1])
	target = int(dum_line[2])
	dist = int(dum_line[3])
	
	dist_array[source,target] = dist
	dist_array[target,source] = dist
	

#Open each path individually.

paths_loc = "/fixed_reduced_ind_rat_path"

out_loc = "/dist_single_fixed_reduced_ind_rat_path"
	
for file in os.listdir(paths_loc):
	in_f_name = os.path.join(paths_loc,file)		#Concatenate names.
		
	if in_f_name != os.path.join(paths_loc,".DS_Store"):
		
		path = []
		
		f = open(in_f_name)			#Get data file.
		for line in f:
			data = line.split("\t")			#Break up line data.
		
			name = data[0]					#Get name.
			sensor = int(data[1])			#Get sensor (must be int).
			time = data[2][:-1]				#Get time data (ignore end of line character).
			formated_time = strng_to_dt(time)		#Make proper format.
			
			path.append([name,sensor,formated_time])
			
		#Go through each path line by line and get distance travelled.
	
		d_path = get_dist_path(path,dist_array)
	
	
		#Print out Id-sensor-time-distance
	
		out_f_name = "dist_single_fixed_reduced_ind_rat_path_" + name + ".txt"
		out_name = os.path.join(out_loc,out_f_name)
		out = open(out_name,'w')
	
		for x in d_path:
			out_string = str(x[0]) + '\t' + str(x[1]) + '\t' + str(x[2]) + '\t' + str(x[3]) + '\n'
			out.write(out_string)














	