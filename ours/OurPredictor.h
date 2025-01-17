#pragma once
#include "../tread.h"

// abstract class for our predictors
class OurPredictor {
public:
    virtual ~OurPredictor() {};
    virtual bool get_prediction(const branch_record_c* br, const op_state_c* os) = 0;
    virtual void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) = 0;
};

