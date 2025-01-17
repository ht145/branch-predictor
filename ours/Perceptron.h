#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <array> 
#include <unordered_map>
#include <bitset> 
#include "LRUMap.h"
#include "OurPredictor.h"


typedef unsigned int TenBitPC;

// 62 hist length was the best they found in the paper

const int HIST_LENGTH = 10;

// store in arrays in the first place so we dont have to convert back and forth when computing dot product
typedef std::array<int, HIST_LENGTH> GlobalPathHistory; 
typedef std::array<int, HIST_LENGTH> Weights;

typedef LRUMap<TenBitPC, Weights> PerceptronTable;

template<typename T1, std::size_t N1>
float dot_product(std::array<T1, N1> ws, std::array<T1, N1> xs){
    float result = 0;
    for (std::size_t i = 1; i < ws.size(); i++){
        // Avoid multiplication since there's only two states in xs anyways, 1 and 0
        // if taken, add. if not taken, do nothing
        if(ws[i] != xs[i]){
            result += -1; 
            }
        else{
            result += 1; 
        } 
    }
    return result;
}

class Perceptron : public OurPredictor {
public:

    GlobalPathHistory pathHistory {};
    PerceptronTable perceptronTable = PerceptronTable(512);

    // Best threshold is always exactly this computed value -- https://www.cs.utexas.edu/~lin/papers/hpca01.pdf
    int threshold = (int) ((32));

    virtual ~Perceptron() = default;
    virtual bool get_prediction(const branch_record_c* br, const op_state_c* os);
    virtual void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

    template<typename T1, typename T2, std::size_t N1, std::size_t N2 >
    void update_weights(unsigned int pc, int t, std::array<T1, N1> pathHistory, std::array<T2, N2>& weights);
    template<typename T3, std::size_t N3>
    void update_path_history(std::array<T3, N3>& arr, bool element);
    
    // In the paper, this is also called "compute y", and the output is called "y_out"
    float get_perceptron_output(const unsigned int pc);
    
    // pick whether we want [11:2] mode or [9:0] mode.
    // [11:2] is the spec, but [9:0] "works too"
    TenBitPC get_ten_bits_of(const unsigned int pc) {
        //return get_ten_bits_11_2(pc);
        return get_ten_bits_9_0(pc);
    };

    TenBitPC get_ten_bits_11_2(const unsigned int pc);
    TenBitPC get_ten_bits_9_0(const unsigned int pc);

};

#endif //PERCEPTRON_H




