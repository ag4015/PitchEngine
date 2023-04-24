
import trainEngine as te
import os
import time
import torch
import torch.nn as nn
import torch.optim as optim

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

start_time = time.time()

x_train, y_train = te.load_input_and_target_data(training_input_path, training_target_path)
x_train, y_train = te.preprocess_training_data(x_train, y_train)

# Define the neural network architecture
class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.fc1 = nn.Linear(1026, 1025)
        # self.fc2 = nn.Linear(1026, 1025)
        # self.fc1 = nn.Linear(1026, 512)
        # self.fc2 = nn.Linear(512, 256)
        # self.fc3 = nn.Linear(256, 128)
        # self.fc4 = nn.Linear(128, 1025)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        # x = self.fc2(x)
        # x = torch.relu(self.fc2(x))
        # x = torch.relu(self.fc3(x))
        # x = self.fc4(x)
        return x

# Create the neural network instance
net = Net()

# Define the loss function and optimizer
criterion = nn.MSELoss()
optimizer = optim.SGD(net.parameters(), lr=0.01)

# Convert the training data to PyTorch tensors
x_train = torch.from_numpy(x_train).float()
y_train = torch.from_numpy(y_train).float()

# Train the neural network
for epoch in range(1000):
    # Forward pass
    outputs = net(x_train)
    loss = criterion(outputs, y_train)

    # Backward pass and optimization
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
    
    # Print the training progress
    if (epoch+1) % 10 == 0:
        print('Epoch [{}/{}], Loss: {:.4f}'.format(epoch+1, 100, loss.item()))

# Save the trained model
torch.save(net.state_dict(), 'model.pth')

execution_time = time.time() - start_time
print("Execution time:", execution_time, "seconds")


