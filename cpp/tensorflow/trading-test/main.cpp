#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

#include <tensorflow/cc/client/client_session.h>
#include <tensorflow/cc/ops/standard_ops.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/common_runtime/gradients.h>

using namespace tensorflow;
using namespace tensorflow::ops;

constexpr const int input_dim = 8; // The input tensor has 8 features - XXX

// Struct to hold current market data
struct MarketData {
    float bid;       // USD highest bid
    float ask;       // USD lowest ask
    float timestamp; // Current time
};

// Struct to hold our current state
struct CurrentState {
    float timestamp = 0;
    float wallet = 0;
    float holding = 0;
    float holding_time = 0;
    float profit = 0;
    float profit_time = 1;

    float score(const MarketData &market) const {
        float holding_value = holding * market.bid;
        if (holding_time > 0) {
            constexpr float SCALE_FACTOR = 1.1;
            holding_value -= SCALE_FACTOR * std::log(holding_time);
        }

#if 0
        float money = profit + holding_value - wallet;
        return money / (1 + (profit_time / 60 / 60));
#else
        return wallet + holding_value - (profit_time / 60);
#endif
    }
};

void normalize(CurrentState &state) {
    if (state.wallet < 0) state.wallet = 0;
    if (state.holding < 0) state.holding = 0;
    if (state.holding <= 0 || state.holding_time < 0) state.holding_time = 0;
    if (state.profit_time < 0) state.profit_time = 0;
}

void idle(CurrentState &state, const MarketData &market) {

    float elapsed = market.timestamp - state.timestamp;

    if (state.holding > 0)
        state.holding_time += elapsed / state.holding;
    else
        state.holding_time = 0;

#if 0
    if (state.profit > 0)
        state.profit_time += elapsed / state.profit;
#else
    state.profit_time += elapsed;
#endif
}

void sell(CurrentState &state, const MarketData &market, float percent) {
    float to_sell = state.holding * percent;
    float revenue = to_sell * market.bid - 1;
    if (to_sell > 0 && to_sell <= state.holding) {
        printf("[SELL] %0.08f BTC ($%0.02f USD)\n", to_sell, revenue);
        state.wallet += revenue;
        state.profit += revenue;
        state.holding -= to_sell;
        normalize(state);
    }
    else {
        idle(state, market);
    }
}

void buy(CurrentState &state, const MarketData &market, float percent) {
    float to_buy = state.wallet * percent;
    if (to_buy > 0.01 && to_buy <= state.wallet) {
        printf("[BUY] $%0.2f\n", to_buy);
        float purchased = to_buy / market.ask;
        state.holding += purchased;
        state.wallet -= to_buy;
        normalize(state);
    }
    else {
        idle(state, market);
    }
}

// Generate market data for training
std::vector<MarketData> getMarketData() {
    std::vector<MarketData> values;
    float timestamp = 0;
    for (int cycle = 0; cycle < 10000; ++cycle) {
        for (float median = 1; median < 10; ++median)
            values.push_back({median, median + 1, timestamp++});
        for (float median = 9; median >= 1; --median)
            values.push_back({median, median + 1, timestamp++});
    }
    return values;
}

Tensor PreprocessData(const CurrentState &state, const MarketData &market) {
    Tensor input_tensor(DT_FLOAT, TensorShape({1, input_dim}));
    auto input_flat = input_tensor.flat<float>();
    input_flat(0) = market.bid;
    input_flat(1) = market.ask;
    input_flat(2) = state.wallet;
    input_flat(3) = state.holding;
    input_flat(4) = state.holding_time;
    input_flat(5) = state.profit;
    input_flat(6) = state.profit_time;
    input_flat(7) = state.score(market);
    return input_tensor;
}

Output Dense(
    const Scope &scope,
    const Input &input,
    int input_dim,
    int num_units,
    std::vector<Operation> &init_ops,
    const std::string &layer_name)
{
    // Create unique names for variables
    std::string weights_name = layer_name + "_weights";
    std::string biases_name = layer_name + "_biases";

    // Create weights and biases variables
    auto weights = Variable(
        scope.WithOpName(weights_name),
        TensorShape({input_dim, num_units}),
        DT_FLOAT);
    auto biases = Variable(
        scope.WithOpName(biases_name),
        TensorShape({num_units}),
        DT_FLOAT);

    // Initialize weights and biases only once
    auto weights_init = Assign(scope.WithOpName(weights_name + "_init"), weights,
                               RandomNormal(scope, {input_dim, num_units}, DT_FLOAT));
    auto biases_init = Assign(scope.WithOpName(biases_name + "_init"), biases,
                              Const(scope, 0.0f, TensorShape({num_units})));

    // Add initialization ops to the list
    init_ops.push_back(weights_init.operation);
    init_ops.push_back(biases_init.operation);

    // Dense layer: output = matmul(input, weights) + biases
    auto matmul = MatMul(scope.WithOpName(layer_name + "_matmul"), input, weights);
    return Add(scope.WithOpName(layer_name + "_dense_output"), matmul, biases);
}

void DefineModel(Scope &root, Output &input, Output &action_probs, std::vector<Operation> &init_ops) {
    input = Placeholder(root.WithOpName("input"), DT_FLOAT, Placeholder::Shape({-1, input_dim}));
    auto hidden = Relu(root.WithOpName("hidden"), Dense(root, input, input_dim, 16, init_ops, "hidden"));
    auto logits = Dense(root.WithOpName("logits"), hidden, 16, 3, init_ops, "logits");
    action_probs = Softmax(root.WithOpName("action_probs"), logits);
}

void TrainModel(ClientSession &session, Scope &root, Output &input, std::vector<Operation> &init_ops, Output &action_probs) {
    auto market_data = getMarketData();

    CurrentState state;
    state.wallet = 5000;

    // 1) Create a scalar variable for reward
    auto reward_var = Variable(root.WithOpName("reward_var"), { }, DT_FLOAT);


    // 2) Initialize it to 0 (or some float)
    auto reward_init_value = Const(root, 0.0f, { });
    auto init_reward_var = Assign(root, reward_var, reward_init_value);
    init_ops.push_back(init_reward_var.operation);

    // 3) Create a placeholder for incoming reward deltas
    auto reward_delta_ph = Placeholder(root.WithOpName("reward_delta"), DT_FLOAT);

    // 4) Define an op to update the variable by adding the new delta
    auto update_reward_op = AssignAdd(root.WithOpName("update_reward"), reward_var, reward_delta_ph);

    // Finalize graph construction
    TF_CHECK_OK(session.Run({}, {}, init_ops, nullptr));

    for (int epoch = 0; epoch < 1; ++epoch) {
        if (state.wallet <= 0)
            state.wallet = 5000;

        for (const auto &data : market_data) {
            Tensor input_tensor = PreprocessData(state, data);

            // Simulate an action
            std::vector<Tensor> outputs;
            ClientSession::FeedType feed = {{input, input_tensor}};
            Status status = session.Run(feed, {action_probs}, &outputs);

            if (!status.ok()) {
                std::cerr << "Error during action_probs computation: " << status.ToString() << std::endl;
                return;
            }

            auto probs = outputs[0].flat<float>();
            if (outputs[0].dims() != 2 || outputs[0].dim_size(1) != 3) {
                std::cerr << "Unexpected shape for action_probs: " << outputs[0].shape().DebugString() << std::endl;
                return;
            }

            int action = std::distance(probs.data(), std::max_element(probs.data(), probs.data() + 3)) - 1;

#if 1
            printf("%0.08f = %0.08f | %0.08f | %0.08f\n",
                state.score(data),
                probs.data()[0],
                probs.data()[1],
                probs.data()[2]);
#endif

            // Apply action to state and compute reward
            if (action == 1) {
                sell(state, data, 0.001);
            } else if (action == -1) {
                buy(state, data, 0.001);
            } else {
                idle(state, data);
            }

            // 6) During training, feed a new float delta and run the update
            float reward = state.score(data);
            feed = {{reward_delta_ph, Tensor(reward)}};
            status = session.Run(feed, {update_reward_op}, &outputs);

            if (!status.ok()) {
                std::cerr << "Error during training step: " << status.ToString() << std::endl;
                return;
            }
        }
    }
}

float MakeActionDecision(ClientSession &session, const Tensor &input_tensor, const Output &input, const Output &action_probs) {
    ClientSession::FeedType feed = {{input, input_tensor}};
    std::vector<Tensor> outputs;
    Status status = session.Run(feed, {action_probs}, &outputs);

    if (!status.ok()) {
        std::cerr << "Error during action decision: " << status.ToString() << std::endl;
        return -1; // Handle error gracefully
    }

    if (outputs[0].dims() != 2 || outputs[0].dim_size(1) != 3) {
        std::cerr << "Unexpected shape for action_probs during inference: " << outputs[0].shape().DebugString() << std::endl;
        return -1;
    }

    auto probs = outputs[0].flat<float>();
    return std::distance(probs.data(), std::max_element(probs.data(), probs.data() + 3)) - 1;
}

void RunTradingBot() {
    Scope root = Scope::NewRootScope();
    ClientSession session(root);

    Output input, action_probs;
    std::vector<Operation> init_ops;

    DefineModel(root, input, action_probs, init_ops);

    TrainModel(session, root, input, init_ops, action_probs);
    printf("DONE TRAINING\n");

    CurrentState state;
    state.wallet = 5000;

    int i = 0;
    for (const MarketData &data : getMarketData()) {
        Tensor live_input = PreprocessData(state, data);
        float action = MakeActionDecision(session, live_input, input, action_probs);

        if (action == -1) sell(state, data, 1.0);
        else if (action == 1) buy(state, data, 1.0);

        if (i++ % 100 == 0)
            printf("Score: $%0.2f\n", state.score(data));
    }
}

int main() {
    RunTradingBot();
    return 0;
}

