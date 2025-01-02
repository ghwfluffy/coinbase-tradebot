#include <vector>
#include <numeric>
#include <cmath>

#include <tensorflow/cc/client/client_session.h>
#include <tensorflow/cc/ops/standard_ops.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/cc/framework/gradients.h>
#include <tensorflow/core/framework/tensor.h>

using namespace tensorflow;
using namespace tensorflow::ops;

// Define state and neural network
struct State {
    static constexpr const int target = 123;
    int a = 1, b = 1, c = 1;

    int current() const {
        int a2 = a;
        int b2 = b;
        int c2 = c;
        if (c > b)
        {
            if (a2 > 4)
                a2 = a2 % 4;
        }
        else
        {
            if (b2 > 10)
                b2 = b2 % 10;
        }
        if (c2 > 20)
            c2 = c2 % 20;
        return (a2 * b2) - c2;
    }
};

constexpr const int state_dims = 4;
constexpr const int logits_dims = 64;

auto Zeros(
    const Scope &scope,
    int size)
{
    return Fill(scope, Const(scope, {size}), 0.0f);
}

// Neural network definition
class PPOAgent {
public:
    Scope root;
    ClientSession session;

    // Model components
    Output inputs, policy_logits, value;
    Output action_placeholder, advantage_placeholder, target_value_placeholder;

    Output policy_loss, value_loss;

    std::vector<Operation> policy_optimization_steps;
    std::vector<Operation> value_optimization_steps;

public:
    Output Dense(
        const Scope &scope,
        const Output &input,
        // Variables are [input_dims, units]
        int input_dims,
        int units,
        std::vector<Variable> &variables,
        const std::string& layer_name = "dense")
    {
        // Create weights and biases
        auto weights = Variable(scope.WithOpName(layer_name + "/weights"),
                                {input_dims, units}, DT_FLOAT);
        auto biases = Variable(scope.WithOpName(layer_name + "/biases"),
                               {units}, DT_FLOAT);
        variables.push_back(weights);
        variables.push_back(biases);

        // Initialize weights and biases
        auto weights_init = Assign(scope.WithOpName(layer_name + "/weights_init"),
                                   weights, RandomNormal(scope, {input_dims, units}, DT_FLOAT));
        auto biases_init = Assign(scope.WithOpName(layer_name + "/biases_init"),
                                  biases, Zeros(scope, units));
        init_ops.push_back(weights_init.operation);
        init_ops.push_back(biases_init.operation);

        // Fully connected layer operation
        auto matmul = MatMul(scope.WithOpName(layer_name + "/matmul"), input, weights);
        auto add_bias = Add(scope.WithOpName(layer_name + "/add_bias"), matmul, biases);

        // Return the output of the dense layer
        return add_bias;
    }

    PPOAgent()
        : root(Scope::NewRootScope()), session(root) {
        // Define placeholders
        inputs = Placeholder(root.WithOpName("inputs"), DT_FLOAT, Placeholder::Shape({-1, state_dims}));
        action_placeholder = Placeholder(root.WithOpName("actions"), DT_INT32, Placeholder::Shape({-1, 1}));
        advantage_placeholder = Placeholder(root.WithOpName("advantages"), DT_FLOAT, Placeholder::Shape({-1, 1}));
        target_value_placeholder = Placeholder(root.WithOpName("target_values"), DT_FLOAT, Placeholder::Shape({-1, 1}));

        // Policy network
        std::vector<Variable> hiddenVariables;
        auto hidden1 = Relu(root.WithOpName("hidden1"), Dense(root, inputs, state_dims, logits_dims, hiddenVariables)); // Input: state_dims, Output: logits_dims
        auto hidden2 = Relu(root.WithOpName("hidden2"), Dense(root, hidden1, logits_dims, logits_dims, hiddenVariables)); // Input: logits_dims, Output: logits_dims
        std::vector<Variable> policyVariables;
        policy_logits = Dense(root.WithOpName("policy_logits"), hidden2, logits_dims, 3, policyVariables); // Input: logits_dims, Output: 3

        // Value network
        std::vector<Variable> valueVariables;
        auto value_hidden1 = Relu(root.WithOpName("value_hidden1"), Dense(root, inputs, state_dims, logits_dims, valueVariables)); // Input: state_dims, Output: logits_dims
        value = Dense(root.WithOpName("value"), value_hidden1, logits_dims, 1, valueVariables); // Input: logits_dims, Output: 1

        // PPO loss
        auto neglogp = Neg(root, Log(root, Softmax(root, policy_logits)));
        auto reshaped_advantage = Reshape(root, advantage_placeholder, Const(root, {-1, 1}));
        auto policy_loss_expr = Mul(root, reshaped_advantage, neglogp);
        //auto policy_loss_expr = Mul(root, advantage_placeholder, neglogp);
        policy_loss = Mean(root, policy_loss_expr, {0});

        auto value_loss_expr = Square(root, Sub(root, target_value_placeholder, value));
        value_loss = Mean(root, value_loss_expr, {0});

        for (const auto& var : policyVariables) {
            // Get gradient of 'loss' wrt 'var'
            std::vector<Output> grad_outputs;
            TF_CHECK_OK(AddSymbolicGradients(root, {policy_loss}, {var}, &grad_outputs));
            auto gradient = grad_outputs[0];

            policy_optimization_steps.push_back(
                ApplyGradientDescent(root, var, Const(root, 0.001f), gradient).operation);
        }

        for (const auto& var : valueVariables) {
            // Get gradient of 'loss' wrt 'var'
            std::vector<Output> grad_outputs;
            TF_CHECK_OK(AddSymbolicGradients(root, {value_loss}, {var}, &grad_outputs));
            auto gradient = grad_outputs[0];

            value_optimization_steps.push_back(
                ApplyGradientDescent(root, var, Const(root, 0.001f), gradient).operation);
        }


        InitializeGraph();
    }

    void OptimizePolicy()
    {
        TF_CHECK_OK(session.Run({}, {}, policy_optimization_steps, nullptr));
    }

    void OptimizeValue()
    {
        TF_CHECK_OK(session.Run({}, {}, value_optimization_steps, nullptr));
    }

    // Train function
    void Train(const Tensor& state_input, const Tensor& actions, const Tensor& advantages, const Tensor& target_values) {
        std::vector<Tensor> outputs;
        std::vector<Operation> optimizers;
        for (auto &o : value_optimization_steps)
            optimizers.push_back(o);
        for (auto &o : policy_optimization_steps)
            optimizers.push_back(o);
        TF_CHECK_OK(session.Run({{inputs, state_input},
                                 {action_placeholder, actions},
                                 {advantage_placeholder, advantages},
                                 {target_value_placeholder, target_values}},
                                {},
                                optimizers, &outputs));
    }


    // Compute softmax probabilities
    std::vector<float> ComputeSoftmax(const std::vector<float>& logits) {
        std::vector<float> probabilities(logits.size());
        float max_logit = *std::max_element(logits.begin(), logits.end());

        // Compute exp(logit - max_logit) for numerical stability
        float sum_exp = 0.0f;
        for (size_t i = 0; i < logits.size(); ++i) {
            probabilities[i] = std::exp(logits[i] - max_logit);
            sum_exp += probabilities[i];
        }

        // Normalize to get probabilities
        for (size_t i = 0; i < probabilities.size(); ++i) {
            probabilities[i] /= sum_exp;
        }

        return probabilities;
    }

    // Act function
    int Act(const Tensor& state_input) {

        std::vector<Tensor> outputs;

        // Run the model to get policy logits
        TF_CHECK_OK(session.Run({{inputs, state_input}}, {policy_logits}, &outputs));
        Tensor logits = outputs[0];  // Shape: [1, num_actions]

        // Extract logits into a vector
        auto logits_flat = logits.flat<float>();
        std::vector<float> logits_vec(logits_flat.data(), logits_flat.data() + logits_flat.size());

        // Compute softmax probabilities
        std::vector<float> probabilities = ComputeSoftmax(logits_vec);

        // Sample an action using the computed probabilities
        std::random_device rd;  // Seed for randomness
        std::mt19937 gen(rd()); // Mersenne Twister RNG
        std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

        // Return the sampled action index
        return dist(gen);
    }
    std::vector<Operation> init_ops;

    // Function to initialize all variables
    void InitializeGraph() {
        TF_CHECK_OK(session.Run({}, {}, init_ops, nullptr));
    }
};

void ComputeAdvantagesAndTrain(PPOAgent& agent,
                               const std::vector<State>& trajectory,
                               const std::vector<float>& rewards,
                               const std::vector<int>& actions,
                               float gamma = 0.99,       // Discount factor
                               float lambda = 0.95) {    // GAE parameter
    size_t T = trajectory.size();

    // Placeholder for state inputs
    Tensor state_input(DT_FLOAT, {static_cast<int>(T), state_dims});
    auto state_input_flat = state_input.flat<float>();
    for (size_t t = 0; t < T; ++t) {
        state_input_flat(t * state_dims + 0) = trajectory[t].a;
        state_input_flat(t * state_dims + 1) = trajectory[t].b;
        state_input_flat(t * state_dims + 2) = trajectory[t].c;
        state_input_flat(t * state_dims + 3) = trajectory[t].current();
    }

    // Placeholder for actions
    Tensor action_input(DT_INT32, {static_cast<int>(T), 1});
    auto action_input_flat = action_input.flat<int>();
    for (size_t t = 0; t < T; ++t) {
        action_input_flat(t) = actions[t];
    }

    // Predict values for states
    std::vector<Tensor> value_outputs;
    TF_CHECK_OK(agent.session.Run({{agent.inputs, state_input}}, {agent.value}, &value_outputs));
    Tensor value_predictions = value_outputs[0];  // Shape: [T, 1]
    auto value_flat = value_predictions.flat<float>();

    // Compute advantages and target values
    std::vector<float> advantages(T);
    std::vector<float> target_values(T);

    float last_value = 0;  // Bootstrap value for terminal state
    float last_advantage = 0;

    for (int t = T - 1; t >= 0; --t) {
        float reward = rewards[t];
        float value = value_flat(t);
        float next_value = (t == T - 1) ? last_value : value_flat(t + 1);

        // Temporal Difference (TD) residual
        float delta = reward + gamma * next_value - value;

        // GAE Advantage
        last_advantage = delta + gamma * lambda * last_advantage;
        advantages[t] = last_advantage;

        // Target value for value network
        target_values[t] = reward + gamma * ((t == T - 1) ? last_value : target_values[t + 1]);
    }

    // Convert advantages and target values to tensors
    Tensor advantages_tensor(DT_FLOAT, {static_cast<int>(T)});
    auto advantages_flat = advantages_tensor.flat<float>();
    std::copy(advantages.begin(), advantages.end(), advantages_flat.data());

    Tensor target_values_tensor(DT_FLOAT, {static_cast<int>(T)});
    auto target_values_flat = target_values_tensor.flat<float>();
    std::copy(target_values.begin(), target_values.end(), target_values_flat.data());

    // Train the agent
    agent.Train(state_input, action_input, advantages_tensor, target_values_tensor);
}


// Simulate environment and PPO training loop
int main()
{
    PPOAgent agent;

    for (int episode = 0; episode < 1000; ++episode) {
        if (episode % 20 == 0)
            printf("Episode %d\n", episode);
        State state;
        std::vector<State> trajectory;
        std::vector<float> rewards;
        std::vector<int> actions;

        // Run episode
        for (int t = 0; t < 64; ++t) {
            Tensor input(DT_FLOAT, {1, state_dims});
            auto flat = input.flat<float>();
            flat(0) = state.a;
            flat(1) = state.b;
            flat(2) = state.c;
            flat(3) = state.current();

            int action = agent.Act(input);
            actions.push_back(action);
            switch (action) {
                case 0: state.a++; break;
                case 1: state.b++; break;
                case 2: state.c++; break;
            }

            if (episode == 0 || episode == 100 || episode == 999) {
                printf("Value: %d/%d\n", state.current(), State::target);
            }

            int reward = -std::abs(state.current() - State::target);
            trajectory.push_back(state);
            rewards.push_back(reward);
        }

        ComputeAdvantagesAndTrain(agent, trajectory, rewards, actions);
    }
        State state;

    return 0;
}
