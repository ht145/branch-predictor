/* Author: Mark Faust
 *
 * C version of predictor file
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <cstring>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class

#include "ours/alpha.h"
#include "ours/NBBP.h"
#include "ours/Perceptron.h"
#include "ours/combination.h"

enum OurPredictorType {
    P_Alpha,
    P_Perceptron,
    P_NBBP,
    P_Combination
    // add more enums if we add predictors
};

class PREDICTOR
{
public:
    PREDICTOR(OurPredictorType p) : predictor_type(p) {
        switch (p)
        {
        case OurPredictorType::P_Alpha:
            predictor = new Alpha();
            break;

        case OurPredictorType::P_Perceptron:
            predictor = new Perceptron();
            break;

        case OurPredictorType::P_NBBP:
            predictor = new NBBP();
            break;
            
        case OurPredictorType::P_Combination:
            predictor = new Combination();
            break;
        
        
        default:
            throw std::runtime_error("failed to initialize predictor; unknown predictor type");
            break;
        }
    };

    OurPredictor* predictor;
    OurPredictorType predictor_type;

    bool get_prediction(const branch_record_c* br, const op_state_c* os);

    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

};

#endif // PREDICTOR_H_SEEN

