#!/usr/bin/python

#This program will open the mole rat data file "xxxxxxx.csv" then re-format it and output a 'clean' version.
#It will then break up the data into a time series for each animal.
#Oliver Edgar Williams for Julie Freeman, Chris Faulkes, RAT.systems 2017


f = open("/nmr_data.csv",'r')		#Open the data file.
check = open("nmr_index.csv",'r')			#Open tag_id index file.

out = open("clean_nmr_data_dec16.txt",'w')		#Open all paths write file.

ind_path = {}		#Create dict for the individual paths

allowed_tags = []		#Set for allowed tags.

#Get allowed tags.
for line in check:
	check_split = line.split(',')
	check_id = check_split[1][1:-1]
	
	if check_id != "tag_id":
		allowed_tags.append(check_id)

allowed_tags = set(allowed_tags)

#clean and extract paths.

for line in f:
	c_split = line.split(',')			#Break the line up by commas.
	
	tag_id = c_split[0][1:-1]			#Take the tag id and remove the speech marks.
	r_id = c_split[1]					#Take the r id value.
	time = c_split[2][1:-1]				#Take the time of the sensor trip.
	
	if tag_id in allowed_tags:
	
		out.write(tag_id + '\t' + r_id + '\t' + time + '\n')		#Wite to all paths file.
	
		d_tag = [tag_id,r_id,time]			#Store all date in line as single list.
	
		if ind_path.has_key(tag_id):
			ind_path[tag_id].append(d_tag)			#If the id has been seen, extend the path.
		else:
			ind_path[tag_id] = [d_tag]				#Else, start that path.

#Print individual paths.		
for ids in ind_path.keys():
	ind_f_name = "ind_rat_path_" + ids + ".txt"			#Generate new name for individual file output.
	
	ind_out = open(ind_f_name,'w')			#Open individual output file.
	
	for x in ind_path[ids]:
		ind_out.write(x[0] + '\t' + x[1] + '\t' + x[2] + '\n')		#Output each path component.
		


















	