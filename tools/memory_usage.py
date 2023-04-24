import os
import sys

# Get the paths of the folders to delete files from
current_path = os.getcwd() + os.sep
pitchengine_index = current_path.rfind('PitchEngine')
if pitchengine_index == -1:
  pitch_engine_path = current_path + "PitchEngine" + os.sep
else:
  pitch_engine_path = current_path[:pitchengine_index+len('PitchEngine')]


data_path = pitch_engine_path + os.sep + "data" + os.sep

output_audio_path             = data_path + "output_audio"             + os.sep
training_input_path           = data_path + "training_input"           + os.sep
training_input_audio_path     = data_path + "training_input_audio"     + os.sep
training_target_path          = data_path + "training_target"          + os.sep
training_target_audio_path    = data_path + "training_target_audio"    + os.sep
training_predicted_path       = data_path + "training_predicted"       + os.sep
training_predicted_audio_path = data_path + "training_predicted_audio" + os.sep


# Define a function to calculate the total memory usage of all files in a directory
def get_directory_memory_usage(directory):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            try:
                file_size = os.path.getsize(file_path)
                total_size += file_size
            except Exception as e:
                print(f"Error getting size of {file_path}: {e}")
    return total_size

# Calculate the total memory usage for each directory
output_audio_memory_usage             = get_directory_memory_usage(output_audio_path)
training_input_memory_usage           = get_directory_memory_usage(training_input_path)
training_input_audio_memory_usage     = get_directory_memory_usage(training_input_audio_path)
training_target_memory_usage          = get_directory_memory_usage(training_target_path)
training_target_audio_memory_usage    = get_directory_memory_usage(training_target_audio_path)
training_predicted_memory_usage       = get_directory_memory_usage(training_predicted_path)
training_predicted_audio_memory_usage = get_directory_memory_usage(training_predicted_audio_path)

total_memory_usage = (
    output_audio_memory_usage + 
    training_input_memory_usage +
    training_input_audio_memory_usage +
    training_target_memory_usage +
    training_target_audio_memory_usage +
    training_predicted_memory_usage +
    training_predicted_audio_memory_usage
)

# Define a function to convert bytes to gigabytes
def bytes_to_gb(bytes):
    return round(bytes / (1024 * 1024 * 1024), 2)

# Print the total memory usage for each directory in GB
print(f"Memory usage for {output_audio_path}:             {bytes_to_gb(output_audio_memory_usage)} GB")
print(f"Memory usage for {training_input_path}:           {bytes_to_gb(training_input_memory_usage)} GB")
print(f"Memory usage for {training_input_audio_path}:     {bytes_to_gb(training_input_audio_memory_usage)} GB")
print(f"Memory usage for {training_target_path}:          {bytes_to_gb(training_target_memory_usage)} GB")
print(f"Memory usage for {training_target_audio_path}:    {bytes_to_gb(training_target_audio_memory_usage)} GB")
print(f"Memory usage for {training_predicted_path}:       {bytes_to_gb(training_predicted_memory_usage)} GB")
print(f"Memory usage for {training_predicted_audio_path}: {bytes_to_gb(training_predicted_audio_memory_usage)} GB")

# Print the total memory usage for all directories in GB
print(f"Total memory usage for all directories: {bytes_to_gb(total_memory_usage)} GB")
