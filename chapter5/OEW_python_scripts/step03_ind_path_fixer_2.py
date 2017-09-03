#!/usr/bin/python
from __future__ import division
import numpy as np
import random
from datetime import datetime
import math
import os
import sys

#This program will open each individual animals path and attempt to fix any errors present in it.
#Oliver Edgar Williams for Julie Freeman, Chris Faulkes, RAT.systems 2017


#Location of individual reduced animal paths - MUST BE NO OTHER FILES IN THIS FOLDER!
paths_loc = "/step2"

#Location of sensor adjacency matrix folder.
s_adj_loc = "/sensor_adj.txt"
s_adj = open(s_adj_loc,'r')

#Location of nest location adjacency matrix folder.
n_adj_loc = "/nest_adj.txt"
n_adj = open(n_adj_loc,'r')

global e_count			#Count for number of errors.

#Location functions.

def first_unique_pair(input_list):
	count = 1								#Find number of steps after which location is well defined.
	for i in xrange(len(input_list) - 1):
		check = input_list[i:i+2]			#Take each successive pair
		if (check[0] != check[1]) and (s_adj_mat[check[0],check[1]] == 1):
			return count					#If we have a suitable pair, return it.
		else:
			count += 1						#If not, increase count and move to next pair.
			
def get_edge_list(file_name):
	edge_file = open(file_name,'r')			#Open edge list file.
	edge_list = {}							#Initialise dictionary for edge list.
	for line in edge_file:
		edge = line.split("\t")[0]			#Take the edge name.
		dum_nodes = line.split("\t")[1][1:-2]		#Take the string for the nodes.		(THIS SETUP REQUIRES LAST CHARACTER TO BE \n)
		comma = dum_nodes.split(",")
		nodes = {comma[0],comma[1]}	#Take the nodes from the string and place in a set.
		
		edge_list[edge] = nodes
	return edge_list
	
def pair_loc(edge_number_1,edge_number_2):
	edge_1 = edge_list[edge_number_1]				#Get the first edge from its number.
	edge_2 = edge_list[edge_number_2]				#Get the second edge.
	
	shared = edge_1.intersection(edge_2)		#Find the common location.
	location = edge_2.difference(shared)		#Find the location after the pair.
	
	return list(location)[0]

def next_loc(current_loc,edge_num):
	
	edge = edge_list[edge_num]				#Find the edge corresponding to the step taken.

	#print edge
	new_loc = edge.difference({str(current_loc)})	#Get the next location.
	#print new_loc
	#print 'current: '+str(current_loc)+' new: '+str(list(new_loc)[0])+' edge: '+str(edge_num)
	if len(new_loc) == 2:
		return 'ERROR'
	else:
		return list(new_loc)[0]

def loc_path(edge_path,initial_loc):
	locs = []										#Initialise the list of time stamped locations.
	new_loc = next_loc(initial_loc,edge_path[0][0])
	locs.append((new_loc,edge_path[0][1]))			#Find the location after the first step.
	
	for i in xrange(len(edge_path) - 1):
		new_loc = next_loc(locs[i][0],edge_path[i+1][0])	#Get the consecutive locations.
		locs.append((new_loc,edge_path[i+1][1]))		#Add the new location with time stamp.
		
	return list(locs)

def reverser(in_list):
	r_list = []
	elements = len(in_list)
	for i in xrange(elements):
		r_list.append(in_list[elements-1-i])
	return r_list


#Functions to do with adj.

#Get the matrix of minimum distances from node i to j.
def f_w_dist(adj):
	nodes = adj.shape[0]			#get the number of nodes in the graph.
	dist = np.zeros([nodes,nodes])	#Initialise the distance matrix.
	dist[np.where(dist == 0)] = float("inf")	#Set to infinities.
	dist[np.where(adj == 1)] = 1		#Set distance for links that exist.
	
	for k in xrange(nodes):
	    for i in xrange(nodes):
	        for j in xrange(nodes):
	           if dist[i,j] > dist[i,k] + dist[k,j]: 
	               dist[i,j] = dist[i,k] + dist[k,j]
	
	for i in xrange(nodes):
		dist[i,i] = 0				#Set the distance from a node to its self to zero.
	return dist

#Get the matrix that only contains links in a shortest path from i to j.	
def shortest_path_mat(node_1, node_2):
	nodes = n_adj_mat.shape[0]			#get the number of nodes in the graph.
	reduced_adj = n_adj_mat.copy()		#Initialise the reduced adjacency matrix to the intial one.
	distance = loc_dists[node_1,node_2]		#Take the distance between the two nodes.
	for i in xrange(nodes):
		if (loc_dists[i,node_2] >= distance) and (i != int(node_1)):
			for j in xrange(nodes):
				reduced_adj[i,j] = 0		#For each node, if it is farther from the target than the source, ignore all its links.
				reduced_adj[j,i] = 0
				
	reduced_adj[node_1,node_1] = 0			#Disallow moving from source node to its self.
	
	return reduced_adj
	
#Given the current node and the target node, randomly pick the next step in the path to the target. 
def next_step(source,target):
	
	path_mat = shortest_path_mat(n_key[source], n_key[target])			#Get the shortest path matrix.
	nodes = n_adj_mat.shape[0]												#get the number of nodes in the graph.
	
	possible_steps = [path_mat[n_key[source],j] for j in xrange(nodes)]		#Get the list of possible links.
	step_locs = [i for i, e in enumerate(possible_steps) if e != 0]		#Get the locations of the possible next nodes.

	step_key = step_locs[random.randrange(len(step_locs))]					#Find the next step.
	
	step = inv_key[step_key]
	
	#Get the link that has been traversed between locations.
	
	edge_taken = (source,step)
	try:
		link = inv_edge[edge_taken]
	except:
		link = inv_edge[(step,source)]
	
	return (step,link)

#Check if the current step is allowed.
def is_allowed(source,target,adj_mat):
	if adj_mat[source[0],target[0]] == 1:
		return(True)
	else:
		return(False)
	

#Get the corrected path when needed.
def error_correct(pair, target, last_time):
	#Function takes two nodes, then finds their distance d.
	d = int(loc_dists[n_key[pair[0]],n_key[target]])
	
	#Insert d-1 new loactions at evenly spaced times along shortest paths.
	insert = []
	dum_s = (pair[0],-1)			#Set second entry to -1 as flag.
	
	t_step = (pair[1][1] - last_time)//(d+1)

	dum_t = pair[1][1]
	
	for i in xrange(d):
		next = next_step(dum_s[0], target)		#Get pair: (new location, edge taken).
						
		dum_s = (next[0],-1)
		
		new_t = dum_t - ((d-i)*t_step)
		
		next_pair = (next[1],new_t)
	
		#insert.insert(0,next_pair)
		insert.append(next_pair)
		
	return(insert)

#Given a source location and a target edge to follow, find/pick the target location to enter via the edge.
def get_target(c_loc,t_edge):
	#First get the rooms adjacent to the target edge.
	rooms = edge_list_tup[t_edge]
	
	#Then find the subset of adjacent rooms which have minimal distance from the current location.
	dist_1 = loc_dists[n_key[rooms[0]],n_key[c_loc]]
	dist_2 = loc_dists[n_key[rooms[1]],n_key[c_loc]]
	
	
	#Pick one of these rooms.
	
	if dist_1 == dist_2:
		pick = int(math.floor(0.5 - random.random())+1)			#choose which of the two rooms if equal dist.
		target = rooms[pick]
	elif dist_1 > dist_2:
		target = rooms[1]
	else:
		target = rooms[0]
		
	return(target)
	
#Take sub-path and send it to be corrected.
def correct_path(path,start):
	
	global e_count
	clean_path = []
	
	move_from = start
	dum_time = path[0][1]
	for x in path:
		pair = (move_from,x)					#Get current location/edge pair.
	
		test_move = next_loc(pair[0], pair[1][0])	#Attempt to follow the edge.
		
		if test_move == 'ERROR':
			target = get_target(move_from,x[0])						#Get the new location to aim for.
			path_to_target = error_correct(pair,target,dum_time)	#Get a shortest path to that location.
			insert_path = path_to_target + [x]						#Make the edge list to insert.
			clean_path += insert_path								#Extend the cleaned path.
			move_from = next_loc(target, x[0])						#Update the current location.
			e_count += 1											#Increment the error count.
		else:
			move_from = test_move
			clean_path.append(x)
			
		dum_time = x[1]
	
	return(clean_path)


#Error correction routine. 
def ecr(file_name):
	#First, open the file and then extract the path.
		
	tag_id = file_name[-15:-4]
	
	print tag_id
	
	ind_path_f = open(file_name,'r')
	
	ind_path = []			#Initialise the list to store the individual path.
	
	#d_mat = f_w_dist(adj_mat)		#Get matrix of distances.
	
	for line in ind_path_f:
		dum_line = line.split("\t")
		dum_loc = dum_line[1]
		dum_time_string = dum_line[2][:-1] 
		
		try:
			time_reform = datetime.strptime(dum_time_string,'%Y-%m-%d %H:%M:%S.%f')
		except:
			time_reform = datetime.strptime(dum_time_string,'%Y-%m-%d %H:%M:%S')

		next_loc = (dum_loc,time_reform)			#Place the next location-time pair into a touple
		
		ind_path.append(next_loc)
	
	#Now look at nest locations.
	
	sensor_list = [x[0] for x in ind_path]				#Extract the sensor list.
		
	first_loc_num = first_unique_pair(sensor_list)			#Find the second sensor in the first unique pair.
	
	pair_end_stamped = ind_path[first_loc_num]			#Take the time stamped entry for the sensor.

	pair = sensor_list[first_loc_num-1:first_loc_num+1]		#Find the first unique pair.

	first_loc = pair_loc(pair[0], pair[1])				#Get the first known location.

	first_loc_stamped = (first_loc,pair_end_stamped[1])
	
	#Now look at the first part of the path.
	
	up_to = ind_path[:first_loc_num+1]			#Take the path leading to the unique pair and reverse it in time.
	if len(up_to) != 1:
		r_up_to = reverser(up_to)
	else:
		r_up_to = up_to
	
	#Run through and correct the early time path, then re-reverse it.
	c_s_path = correct_path(r_up_to,first_loc)
	c_s_path = reverser(c_s_path)
	
	#Get the late time path.
	e_path = ind_path[first_loc_num+1:]
	c_e_path = correct_path(e_path,first_loc)
	
	
	clean_path = c_s_path + c_e_path			#concatenate clean paths (CHECK BOUNDARY VALUES!)
		
	#Now output to file.
	
	out_file_name = "fixed_reduced_ind_rat_path_" + str(tag_id) + ".txt"
	out_file = open(out_file_name,'w')
	 
	for x in clean_path:
		out_file.write(str(tag_id) + "\t" + str(x[0]) + "\t" + str(x[1]) + "\n")
	

#Main.


#Get the sensor adjacency matrix and turn it into a dict.
s_adj_mat = np.loadtxt(s_adj,dtype = 'int')

#Get the room adjacency matrix and turn it into a dict.
n_adj_mat = np.loadtxt(n_adj,dtype = 'int')

n_key = {'TB' : 0 , 'FD' : 1 , 'UC' : 2 , 'NE' : 3 , 'TA' : 4 , 'LC' : 5}		#Key for location matrix.

inv_key = {x:y for y,x in n_key.items()}

loc_dists = f_w_dist(n_adj_mat)

edge_list = get_edge_list('rats_edge_list.txt')			#Get the edge list.

#inv_edge = {(x[0],x[1]):y for y,x in edge_list.items()}			#Get the inverted edge list.
edge_list_tup = {x:tuple(y) for x,y in edge_list.items()}			#Get edge list with tuples not sets.

inv_edge = {}
for x in edge_list.keys():
	dum_pair = tuple(edge_list_tup[x])
	inv_edge[dum_pair] = x
	
#Scan over input files.
for file in os.listdir(paths_loc):
	in_f_name = os.path.join(paths_loc,file)		#Concatenate names.
	
	if in_f_name != os.path.join(paths_loc,".DS_Store"):
		e_count = 0
		ecr(in_f_name)									#If the file is part of the paths, then run the ecr process.
		print "Error count:\t"+str(e_count)

 

