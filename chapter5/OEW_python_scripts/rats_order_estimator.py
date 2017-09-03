#!/usr/bin/python
from datetime import datetime
import os
import sys
import numpy as np
import operator
from operator import itemgetter
from pathtools.markovchain import MarkovChain
import pathtools.markovtools as mt

#REQURES THE PACKAGE "pathtools" BY P.SINGER: https://github.com/psinger/PathTools
#"pathtools" REQUIRES THE PACKAGES "prettyplotlib" and "tables" which in turn requires H5PY and more

#This program takes the time stamped, cleaned, mole rat path data and attempts to reconstruct the location path (data only contains links taken).
#Oliver Edgar Williams for Julie Freeman, Chris Faulkes, RAT.systems 2017


paths_loc = "/step3"

out = open('rats_order_estimate_201706.txt','w')	#Set out file.


paths = {}			#Initialise dictionary for individual paths.


#Isolate each time stamped path and take the sensor list.
for file in os.listdir(paths_loc):
	in_f_name = os.path.join(paths_loc,file)		#Concatenate names.
	
	if in_f_name != os.path.join(paths_loc,".DS_Store"):
		f = open(in_f_name)			#Get data file.
		for line in f:
			data = line.split("\t")			#Break up line data.
	
			name = data[0]					#Get name.
			sensor = data[1]				#Get sensor.
	
			if paths.has_key(name):
				paths[name].append(sensor)			#If the tag has been seen before then add the sensor,time touple to the path.
			else:
				paths[name] = [sensor]					#Else add a new key first.

#Function to get the estimated order of each sensor path.				
def order_est(X,r):
	max_model = r
	
	paths = X

	likelihoods = {}
	parameters = {}
	observations = {}
	state_count_initial = {}

	evidences = {}

	#This is for the Bayesian case
	for i in range(0,max_model+1):
		markov = MarkovChain(k=i, use_prior=True, reset = True, modus="bayes")
		markov.prepare_data(paths)
		markov.fit(paths)

		evidence = markov.bayesian_evidence()
		evidences[i] = evidence
	
		del markov

	model_probas = mt.bayesian_model_selection(evidences=evidences, params=parameters, penalty=False)

	#print model_probas

	est_val = max(model_probas.iteritems(), key=operator.itemgetter(1))[0]
	return(est_val)
	
#Now get the estimated memory.

r = 10			#Set the maximum memory to try.

for x in paths:
	p_est = order_est([paths[x]], r)
	
	out.write(str(x) + '\t' + str(p_est) + '\n')
	
	print str(x) + '\t' + str(p_est)
