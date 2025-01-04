#!/usr/bin/env python3

import tensorflow as tf
import numpy as np


print("Num GPUs Available: ", len(tf.config.list_physical_devices('GPU')))

# Generate training data
def generate_data(num_samples=10000):
    inputs = []
    labels = []
    for _ in range(num_samples):
        x = np.random.randint(1, 1000, size=3)
        index_123 = np.random.randint(0, 3)
        x[index_123] = 123
        inputs.append(x)
        labels.append(index_123)
    return np.array(inputs, dtype=np.float32), np.array(labels, dtype=np.int32)

# Create training and validation datasets
train_x, train_y = generate_data(8000)
val_x, val_y = generate_data(2000)

# Build the neural network model
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(3,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])

# Compile the model
model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
    loss=tf.keras.losses.SparseCategoricalCrossentropy(),
    metrics=['accuracy']
)

# Train the model
model.fit(train_x, train_y, validation_data=(val_x, val_y), epochs=10, batch_size=32)

# Test the model on new data
def test_model(model):
    test_input = np.random.randint(1, 1000, size=3)
    index_123 = np.random.randint(0, 3)
    test_input[index_123] = 123
    prediction = model.predict(test_input.reshape(1, -1), verbose=0)
    predicted_index = np.argmax(prediction)

    print("Test Input:", test_input)
    print("Actual Index of 123:", index_123)
    print("Predicted Index of 123:", predicted_index)

# Test the trained model
test_model(model)
