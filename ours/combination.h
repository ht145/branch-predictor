#ifndef COMB_H
#define COMB_H

#include <unordered_map>
#include <bitset>
#include "LRUMap.h"
#include "OurPredictor.h"
#include "alpha.h"
#include "Perceptron.h"

//typedef std::array<int8_t, 12> GlobalPathHistory; 

class Combination : public OurPredictor {
   
public:
    Alpha alph; 
    Perceptron percep; 
    GlobalPathHistory pathHistory;
    

    virtual ~Combination() = default;
    virtual bool get_prediction(const branch_record_c* br, const op_state_c* os);
    virtual void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

   
};

#endif //COMB_H
