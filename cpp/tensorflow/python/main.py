#!/usr/bin/env python3

import tensorflow as tf
import numpy as np
from tensorflow.keras import layers
import random

class PPOAgent:
    def __init__(self, input_dim, num_actions, learning_rate=0.001, gamma=0.99, clip_ratio=0.2):
        self.input_dim = input_dim
        self.num_actions = num_actions
        self.gamma = gamma
        self.clip_ratio = clip_ratio

        # Actor network
        self.actor = self.build_actor()
        self.actor_optimizer = tf.keras.optimizers.Adam(learning_rate)

        # Critic network
        self.critic = self.build_critic()
        self.critic_optimizer = tf.keras.optimizers.Adam(learning_rate)

    def build_actor(self):
        model = tf.keras.Sequential([
            layers.Input(shape=(self.input_dim,)),
            layers.Dense(64, activation='relu'),
            layers.Dense(64, activation='relu'),
            layers.Dense(self.num_actions, activation='softmax')
        ])
        return model

    def build_critic(self):
        model = tf.keras.Sequential([
            layers.Input(shape=(self.input_dim,)),
            layers.Dense(64, activation='relu'),
            layers.Dense(64, activation='relu'),
            layers.Dense(1)
        ])
        return model

    def get_action(self, state):
        state = np.expand_dims(state, axis=0)
        probabilities = self.actor(state)[0].numpy()
        action = np.random.choice(self.num_actions, p=probabilities)
        return action, probabilities[action]

    def compute_advantages(self, rewards, values, next_values, dones):
        advantages = []
        gae = 0
        for t in reversed(range(len(rewards))):
            delta = rewards[t] + self.gamma * next_values[t] * (1 - dones[t]) - values[t]
            gae = delta + self.gamma * gae * (1 - dones[t])
            advantages.insert(0, gae)

        # XXX: Normalize advantages
        #advantages = (advantages - np.mean(advantages)) / (np.std(advantages) + 1e-8)

        return advantages

    def train(self, states, actions, advantages, rewards, old_probs):
        # Update actor
        with tf.GradientTape() as tape:
            probs = self.actor(states, training=True)
            action_probs = tf.reduce_sum(probs * tf.one_hot(actions, self.num_actions), axis=1)
            ratios = action_probs / old_probs

            clipped_ratios = tf.clip_by_value(ratios, 1 - self.clip_ratio, 1 + self.clip_ratio)
            loss_clip = tf.minimum(ratios * advantages, clipped_ratios * advantages)
            actor_loss = -tf.reduce_mean(loss_clip)

        grads = tape.gradient(actor_loss, self.actor.trainable_variables)
        self.actor_optimizer.apply_gradients(zip(grads, self.actor.trainable_variables))

        # Update critic
        with tf.GradientTape() as tape:
            values = self.critic(states, training=True)
            critic_loss = tf.reduce_mean((rewards - tf.squeeze(values)) ** 2)

        #print(f"Actor Loss: {actor_loss.numpy()}, Critic Loss: {critic_loss.numpy()}")

        grads = tape.gradient(critic_loss, self.critic.trainable_variables)
        self.critic_optimizer.apply_gradients(zip(grads, self.critic.trainable_variables))

def generate_sample():
    correct_index = random.randint(0, 2)
    sample = [124.0 for _ in range(3)]
    #sample = [random.uniform(0, 200) for _ in range(3)]
    sample[correct_index] = 123.0
    return sample, correct_index

# Hyperparameters
input_dim = 3
num_actions = 3
epochs = 5000
batch_size = 256

gamma = 0.99
clip_ratio = 0.01
learning_rate = 0.001

# Initialize PPO agent
agent = PPOAgent(input_dim, num_actions, learning_rate, gamma, clip_ratio)

# Training loop
for epoch in range(epochs):
    states = []
    actions = []
    rewards = []
    dones = []
    old_probs = []
    values = []
    next_values = []

    correct = 0
    cur_batch_size = 32
    #cur_batch_size = random.randint(8, batch_size)
    for _ in range(cur_batch_size):
        state, correct_action = generate_sample()
        action, prob = agent.get_action(state)
        value = agent.critic(np.expand_dims(state, axis=0))[0][0].numpy()

        reward = 1.0 if action == correct_action else -1.0
        next_value = 0 if reward == 1.0 else agent.critic(np.expand_dims(state, axis=0))[0][0].numpy()

        if action == correct_action:
            correct += 1

        #if epoch % 100 == 0 or epoch == (epochs - 1):
        #    print(reward)

        states.append(state)
        actions.append(action)
        rewards.append(reward)
        dones.append(0 if reward == 1.0 else 1.0)
        old_probs.append(prob)
        values.append(value)
        next_values.append(next_value)

    advantages = agent.compute_advantages(rewards, values, next_values, dones)

    agent.train(
        states=np.array(states),
        actions=np.array(actions),
        advantages=np.array(advantages),
        rewards=np.array(rewards),
        old_probs=np.array(old_probs)
    )

    percent: float = correct / cur_batch_size * 100.0
    print(f"Epoch {epoch + 1}/{epochs} completed: {percent}")

