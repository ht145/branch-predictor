#include "combination.h"
#include "../predictor.h"
#include "../doctest.h"

bool Combination::get_prediction(const branch_record_c* br, const op_state_c* os) {
        if (!br->is_conditional) {
        return true;
    }
    
    auto path_history = alph.global_path_history;
    bool global_prediction = 0; 
    auto pc = alph.get_ten_bits_of(br->instruction_addr);
    auto local_branch_history = alph.local_history_table.get(pc);

    auto local_prediction = alph.local_predictor_table.get(local_branch_history);
    
    auto product = percep.get_perceptron_output(pc);
    
    if(product < 0){
      global_prediction = 0; 
      }
      
    else{
      global_prediction = 1; 
      }
    
    
    auto choice_prediction = alph.choice_prediction_table.get(path_history);

    // mux
    return choice_prediction.predict() ? local_prediction.predict() : global_prediction;
    //return local_prediction.predict();
}

void Combination::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) {
    if (!br->is_conditional) {
        return ;
    }
    auto global_path_history = alph.global_path_history;
    auto pathHistory = percep.pathHistory;
    
    percep.update_path_history(pathHistory, taken);
    
    auto pc = alph.get_ten_bits_of(br->instruction_addr);
    auto local_branch_history = alph.local_history_table.get(pc);

    // get prediction again, to update choice prediction table
    auto local_prediction = alph.local_predictor_table.get(local_branch_history).predict();
    

    alph.update_local_history_table(pc, taken);

    // update local & global predictor tables
    alph.local_predictor_table.get(local_branch_history).update(taken);


    //not sure if this is the correct way to return the predicted value from the above fxn
    auto global_prediction = get_prediction(br, os);
    Weights& weights = percep.perceptronTable.get(pc);
    int tau = taken ? 1 : -1; 

    auto y_out = percep.get_perceptron_output(pc);
    auto threshold = percep.threshold;
    
    // formula in the paper:
    // if (sign(y_out) != taken) or (abs(y_out) <= threshold)
    // "only train if prediction was wrong, and y_out doesn't exceed training threshold"
    if (global_prediction != taken || std::abs(y_out) <= threshold) {
        percep.update_weights(pc, tau, pathHistory, weights);
        percep.perceptronTable.insert(pc, weights); // TODO: I think its actually not needed bc weights is a ref?
    }

   
    if (local_prediction == global_prediction) {
        // do nothing
    } else if (local_prediction == taken) { // else, if local prediction is correct
        alph.choice_prediction_table.get(global_path_history).incr();
    } else if (global_prediction == taken) { // else, if global prediction is correct
        alph.choice_prediction_table.get(global_path_history).decr();
    }

    alph.update_global_path_history(taken);
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

