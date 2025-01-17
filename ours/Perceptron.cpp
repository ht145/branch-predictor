#include "Perceptron.h"
#include "../predictor.h"
#include "../doctest.h"


bool Perceptron::get_prediction(const branch_record_c* br, const op_state_c* os) {
    if (!br->is_conditional) {
        return true;
    }
    auto pc = br->instruction_addr; 
    auto ten_bit_pc = get_ten_bits_of(pc); 

    auto product = get_perceptron_output(ten_bit_pc);
      
    // if product < 0, not taken. else taken
    if (product < 0)
        return false;
    else
        return true; 
}

void Perceptron::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) {
    auto ten_bits_pc = get_ten_bits_of(br->instruction_addr); 

    update_path_history(pathHistory, taken);

    //not sure if this is the correct way to return the predicted value from the above fxn
    auto prediction = get_prediction(br, os);
    Weights& weights = perceptronTable.get(ten_bits_pc);
    int tau = taken ? 1 : -1; 

    auto y_out = get_perceptron_output(ten_bits_pc);

    // formula in the paper:
    // if (sign(y_out) != taken) or (abs(y_out) <= threshold)
    // "only train if prediction was wrong, and y_out doesn't exceed training threshold"
    if (prediction != taken || std::abs(y_out) <= threshold) {
        update_weights(ten_bits_pc, tau, pathHistory, weights);
        perceptronTable.insert(ten_bits_pc, weights); // TODO: I think its actually not needed bc weights is a ref?
    }
}

float Perceptron::get_perceptron_output(const unsigned int ten_bit_pc)
{
    auto weights = perceptronTable.get(ten_bit_pc);

    // weight[0] is a "bias" weight
    auto product = weights[0] + dot_product(weights, pathHistory);
    return product;
}

template<typename T1, std::size_t N1>
void Perceptron::update_path_history(std::array<T1, N1>& arr, bool taken) {
    // Shift all elements to the left by one position
    T1 element;
    for (std::size_t i = 0; i < arr.size() - 1; ++i) {
        arr[i] = arr[i + 1];
    }
    // Add the new element at the end
    element = taken ? 1 : -1; // some code i attempted
    arr[arr.size() - 1] = element;

    // x[0] always set as 1, as it is the bias input
    arr[0] = 1; 
}


template<typename T1, typename T2, std::size_t N1, std::size_t N2>
void Perceptron::update_weights(unsigned int pc, int t, std::array<T1, N1> pathHistory, std::array<T2, N2>& weights){
    for(std::size_t i = 0; i < weights.size(); i++ )
    {
        if(t == pathHistory[i])
        {
            weights[i] = weights[i] + 1;
        }
       else
       {
            weights[i] = weights[i] - 1;
       }
    }
}


TenBitPC Perceptron::get_ten_bits_9_0(const unsigned int pc) {
    return pc & 0b1111111111;
}
