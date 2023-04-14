
import numpy as np
import pandas as pd
import os
import re
import platform
import time
#import matplotlib.pyplot as plt

def parse_name(name):
    # Replace underscores between two numbers with a decimal point
    name = re.sub(r"(?<=\d)_(?=\d)", ".", name)

    # Split the string by underscore character
    split_name = name.split("_")

    # Create an empty dictionary to store the key-value pairs
    result_dict = {}

    # Iterate over the split string, two elements at a time
    for i in range(0, len(split_name), 2):
        # Assign the key and value for each pair
        key = split_name[i]
        value = split_name[i+1]
        
        # Add the key-value pair to the dictionary
        result_dict[key] = value
    return result_dict

def generate_string_from_dict(d):
    # Create an empty list to store the key-value pairs
    items = []

    # Iterate over the dictionary items
    for key, value in d.items():
        # Add the key-value pair to the items list
        items.append(f"{key}_{value}")

    # Join the items list into a string separated by underscores
    result_string = "_".join(items)

    # Replace all dots with underscores
    result_string = result_string.replace(".", "_")

    return result_string

def get_shifted_dict_from_input_dict(input_dict):
    shifted_dict = input_dict.copy()
    shifted_dict["data"]  = "target"
    shifted_dict["steps"] = 0
    # Multiply the frequency of the input with the pitch shift factor
    freq = float(input_dict["freq"]) * 2**(int(input_dict["steps"])/12)
    shifted_dict["freq"] = f"{freq:e}"
    return shifted_dict

sep = "/"
if platform.system() == "Windows":
    sep = "\\"

current_path = os.getcwd() + sep
data_path = current_path + "data" + sep

output_audio_path             = data_path + "output_audio" + sep
training_input_path           = data_path + "training_input" + sep
training_input_audio_path     = data_path + "training_input_audio" + sep
training_target_path          = data_path + "training_target" + sep
training_target_audio_path    = data_path + "training_target_audio" + sep
training_predicted_path       = data_path + "training_predicted" + sep
training_predicted_audio_path = data_path + "training_predicted_audio" + sep

# get a list of all files and directories in the given path
items = os.listdir(training_input_path)

# filter out the non-directories from the list
folders = [item for item in items if os.path.isdir(os.path.join(training_input_path, item))]

start_time = time.time()

training_input_data = np.zeros((2,476,1026))
training_target_data = np.zeros((2,476,1025))
for i, input_folder in enumerate(folders):

    input_dict = parse_name(input_folder)

    input_folder_path = training_input_path + input_folder + sep

    mag_file   = input_folder_path + "mag.csv"
    phase_file = input_folder_path + "phi_a.csv"

    mag  = np.genfromtxt(mag_file, delimiter=',')
    phase = np.genfromtxt(phase_file, delimiter=',')
    steps = np.ones(mag.shape[0]).reshape(-1,1)*float(input_dict["steps"])
    bode_data = np.concatenate((mag, phase), axis=1)
    training_input_data[i] = np.concatenate((bode_data, steps), axis=1)

    shifted_dict = get_shifted_dict_from_input_dict(input_dict)
    target_folder_path = training_target_path + generate_string_from_dict(shifted_dict) + sep

    mag_file = target_folder_path + "mag.csv"
    phase_file = target_folder_path + "phi_a.csv"

    mag = np.genfromtxt(mag_file, delimiter=',')
    phase = np.genfromtxt(phase_file, delimiter=',')
    training_target_data[i] = np.concatenate((mag,phase), axis=1)

training_input_data = np.array(training_input_data)
training_target_data = np.array(training_target_data)

execution_time = time.time() - start_time
print("Execution time:", execution_time, "seconds")

pass



