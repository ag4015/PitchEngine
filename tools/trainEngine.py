
import numpy as np
import os
import re

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

def get_target_dict_from_input_dict(input_dict):
    shifted_dict = input_dict.copy()
    shifted_dict["data"]  = "target"
    shifted_dict["steps"] = 0
    # Multiply the frequency of the input with the pitch shift factor
    freq = float(input_dict["freq"]) * 2**(int(input_dict["steps"])/12)
    shifted_dict["freq"] = f"{freq:e}"
    return shifted_dict

def normalize_array(array, axisVal):
    # Compute the mean and standard deviation along the row axis
    mean = np.mean(array, axis=axisVal)
    std  = np.std(array, axis=axisVal)

    # import pdb; pdb.set_trace()
    if axisVal == 1:
        normalized_array = (array - mean.reshape(-1,1))/(std.reshape(-1,1))
    elif axisVal == 0:
        normalized_array = (array - mean.reshape(1,-1))/(std.reshape(1,-1))
    return normalized_array

def load_input_and_target_data(input_path, target_path):

    # get a list of all files and directories in the given path
    items = os.listdir(input_path)

    # filter out the non-directories from the list
    folders = [item for item in items if os.path.isdir(os.path.join(input_path, item))]

    # Assume that all files have the same buflen, numSamp and hopA
    input_dict = parse_name(folders[0])
    buflen     = int(input_dict["buflen"]) 
    numSamp    = int(input_dict["numSamp"])
    numFrames  = int(buflen / int(input_dict["hopA"]))
    numRows    = int(numFrames*(numSamp - buflen)/buflen)

    x_train = np.zeros((len(folders), numRows, buflen + 2))
    y_train = np.zeros((len(folders), numRows, buflen + 1))
    for i, input_folder in enumerate(folders):

        input_dict  = parse_name(input_folder)
        target_dict = get_target_dict_from_input_dict(input_dict)

        input_folder_path = input_path + input_folder + os.sep

        mag_file   = input_folder_path + "mag.csv"
        phase_file = input_folder_path + "phi_a.csv"

        mag   = np.genfromtxt(mag_file, delimiter=',')
        phase = np.genfromtxt(phase_file, delimiter=',')
        steps = np.ones(mag.shape[0]).reshape(-1,1)*float(input_dict["steps"])
        bode_data = np.concatenate((mag, phase), axis=1)
        x_train[i] = np.concatenate((bode_data, steps), axis=1)

        target_folder_path = target_path + generate_string_from_dict(target_dict) + os.sep

        mag_file   = target_folder_path + "mag.csv"
        phase_file = target_folder_path + "phi_a.csv"

        mag   = np.genfromtxt(mag_file, delimiter=',')
        phase = np.genfromtxt(phase_file, delimiter=',')
        y_train[i] = np.concatenate((mag,phase), axis=1)

    x_train = x_train.reshape(len(folders)*numRows, -1)
    y_train = y_train.reshape(len(folders)*numRows, -1)
    return x_train, y_train

def preprocess_training_data(x_train, y_train):
    # Normalize along the columns
    normalize_array(x_train, 0)
    normalize_array(y_train, 0)
    return x_train, y_train
