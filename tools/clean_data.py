
import os

# Get the paths of the folders to delete files from
current_path = os.getcwd() + os.sep
pitchengine_index = current_path.rfind('PitchEngine')
pitch_engine_path = current_path[:pitchengine_index+len('PitchEngine')]

data_path = pitch_engine_path + os.sep + "data" + os.sep

output_audio_path             = data_path + "output_audio"             + os.sep
training_input_path           = data_path + "training_input"           + os.sep
training_input_audio_path     = data_path + "training_input_audio"     + os.sep
training_target_path          = data_path + "training_target"          + os.sep
training_target_audio_path    = data_path + "training_target_audio"    + os.sep
training_predicted_path       = data_path + "training_predicted"       + os.sep
training_predicted_audio_path = data_path + "training_predicted_audio" + os.sep

# Define a function to delete files in a directory
def delete_files_in_dir(directory):
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        try:
            os.remove(file_path)
            print(f"Deleted {file_path}")
        except Exception as e:
            print(f"Error deleting {file_path}: {e}")

# Delete files in each directory
delete_files_in_dir(output_audio_path)
delete_files_in_dir(training_input_path)
delete_files_in_dir(training_input_audio_path)
delete_files_in_dir(training_target_path)
delete_files_in_dir(training_target_audio_path)
delete_files_in_dir(training_predicted_path)
delete_files_in_dir(training_predicted_audio_path)

print("Cleaned data")