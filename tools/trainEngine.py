
import numpy as np
import pandas as pd
import os
import re
import platform
import matplotlib.pyplot as plt

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
    shifted_dict["data"] = "target"
    # Multiply the frequency of the input with the pitch shift factor
    freq = float(shifted_dict["freq"]) * 2**(int(shifted_dict["steps"])/12)
    shifted_dict["freq"] = f"{freq:e}"
    return shifted_dict

sep = "/"
if platform.system() == "Windows":
    sep = "\\"

current_path = os.getcwd() + sep
data_path = current_path + "data" + sep
debug_data_path = "debugData" + sep
training_audio_path = "training_audio" + sep

# get a list of all files and directories in the given path
items = os.listdir(debug_data_path)

# filter out the non-directories from the list
folders = [item for item in items if os.path.isdir(os.path.join(debug_data_path, item))]

folder_name = folders[0]

input_dict = parse_name(folder_name)

folder_path = debug_data_path + folder_name + sep

mag_file = folder_path + "mag.csv"
phase_file = folder_path + "phi_a.csv"

# Turn csv to numpy array and remove last nan column
mag = np.genfromtxt(mag_file, delimiter=',')
phase = np.genfromtxt(phase_file, delimiter=',')
bode_train_data = np.concatenate((mag,phase), axis=1)

shifted_dict = get_shifted_dict_from_input_dict(input_dict)
shifted_audio_data_name = debug_data_path + generate_string_from_dict(shifted_dict)



