#!/usr/bin/env python3

import torch
import torch.nn as nn
import torch.optim as optim
import random

# Define the dataset generation function
def generate_data(samples=1000):
    data = []
    labels = []
    for _ in range(samples):
        inputs = [random.randint(0, 200) for _ in range(3)]
        target_index = random.randint(0, 2)
        inputs[target_index] = 123  # Ensure one input is 123
        data.append(inputs)
        labels.append(target_index)
    return torch.tensor(data, dtype=torch.float32), torch.tensor(labels, dtype=torch.long)

# Neural network definition
class SimpleNN(nn.Module):
    def __init__(self):
        super(SimpleNN, self).__init__()
        self.fc = nn.Sequential(
            nn.Linear(3, 16),
            nn.ReLU(),
            nn.Linear(16, 3)
        )

    def forward(self, x):
        return self.fc(x)

# Hyperparameters and setup
num_epochs = 10
learning_rate = 0.01
batch_size = 32

# Generate training data
train_data, train_labels = generate_data(10000)

# DataLoader for batching
dataset = torch.utils.data.TensorDataset(train_data, train_labels)
data_loader = torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)

# Initialize model, loss function, and optimizer
model = SimpleNN()
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=learning_rate)

# Training loop
for epoch in range(num_epochs):
    for inputs, labels in data_loader:
        outputs = model(inputs)
        loss = criterion(outputs, labels)

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

    print(f"Epoch [{epoch + 1}/{num_epochs}], Loss: {loss.item():.4f}")

# Evaluation function
def evaluate():
    inputs = [random.randint(0, 200) for _ in range(2)] + [123]
    random.shuffle(inputs)
    input_tensor = torch.tensor(inputs, dtype=torch.float32).unsqueeze(0)
    output = model(input_tensor)
    predicted_index = torch.argmax(output, dim=1).item()
    print(f"Inputs: {inputs}")
    print(f"Predicted index: {predicted_index}")

# Evaluate the trained model
evaluate()

