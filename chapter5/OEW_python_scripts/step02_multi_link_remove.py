#!/usr/bin/python
from __future__ import division
import numpy as np
import random
from datetime import datetime
from datetime import timedelta
import os
#This program will take a time stamped animal path, then run through looking for repeated RID tags.
#If the tags are all within some time frame (user) then a number of these links will be removed.
#The number removed will be even if the number of links is even, and odd otherwise.
#Oliver Edgar Williams for Julie Freeman, Chris Faulkes, RAT.systems 2017

#Open the file and then extract the path.
def get_path(file_name):
		
	tag_id = file_name[-15:-4]
		
	print tag_id
		
	ind_path_f = open(file_name,'r')
		
	ind_path = []			#Initialise the list to store the individual path.
				
	for line in ind_path_f:
		dum_line = line.split("\t")
		dum_loc = dum_line[1]
		dum_time_string = dum_line[2][:-1] 
			
		time_reform = datetime.strptime(dum_time_string,'%Y-%m-%d %H:%M:%S.%f')
		
		ind_path.append((tag_id,dum_loc,time_reform))
		
	return(ind_path)
	

#Look for repeated links then remove them if needed.
def remove_links(path,threshold):

	new_path = []
	
	dum_point = path[0]				#Take the first point in the path.
	dum_path = []					#For storing the consecutive points.
	
	
	for x in range(1,len(path)):
		point = path[x]				#Take the current point in the path.
		
		t_delta = point[2] - dum_point[2]		#Get the time difference.
		#print t_delta
		
		if len(dum_path) == 0:
			dum_path = [dum_point]		#If the dummy path has no points, add one to it.
		
		if (t_delta <= threshold)and(point[1] == dum_point[1]):
			#If the sensor values are the same and the time gap is below the threshold add the new point to a list.
			dum_path.append(point)
		else:
			#If not then reduce the dummy list, place it into the new path, clean the list and move on.
			dum_path = reduce_list(dum_path)
			new_path += dum_path 
			dum_path = []
		
		dum_point = point
		
	return(new_path)
	
		
#Take a sample path, then remove elements while maintaining parity.
def reduce_list(path):
	new_path = []
	
	path_len = len(path)			#Get the number of elements in the path.
	
	parity = path_len % 2			#Get the parity.
	
	if parity == 0:
		#Even case.
		new_path = [path[0],path[-1]]
	else:
		#Odd case.
		new_path = [path[0]]
		
	return(new_path)

#Main.

threshold = timedelta(seconds = 3)		#Number of seconds for threshold on multiple links.

#Location of individual animal paths - MUST BE NO OTHER FILES IN THIS FOLDER!!!
paths_loc = "/Users/joz/phd/_NMR/data-analysis/analysis/monthly_snapshots/dec16/step1"

#Open the individual path 
#Scan over input files.
for file in os.listdir(paths_loc):
	in_f_name = os.path.join(paths_loc,file)		#Concatenate names.
	
	if in_f_name != os.path.join(paths_loc,".DS_Store"):
		path = get_path(in_f_name)				#If the file is part of the paths, then get the path.
		rem_path = remove_links(path,threshold)		#Run the link removal process.
		
		#Now output to file.
		
		tag_id = in_f_name[-15:-4]
			
		out_file_name = "reduced_ind_rat_path_" + str(tag_id) + ".txt"
		out_file = open(out_file_name,'w')
			 
		for x in rem_path:
			out_file.write(str(tag_id) + "\t" + str(x[1]) + "\t" + str(x[2]) + "\n")
	
