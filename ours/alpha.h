#ifndef ALPHA_H
#define ALPHA_H

#include <unordered_map>
#include <bitset>
#include "LRUMap.h"
#include "OurPredictor.h"
#include "SaturatingCounter.h"

typedef unsigned int TenBitPC; // ten bits of PC. see helper fn get_ten_bits_of()
                               // TODO: newtype it so we got type checking?
typedef std::bitset<10> BranchHistory;

// This is the local predictor table, indexed by PC = location of branch instructions
// and storing the direction of the selected branch's last 10 executions
typedef LRUMap<BranchHistory, SaturatingCounter<3>> LocalPredictorTable;
typedef LRUMap<TenBitPC, BranchHistory> LocalHistoryTable;

typedef std::bitset<12> PathHistory;
typedef LRUMap<PathHistory, SaturatingCounter<2>> GlobalPredictorTable;
typedef LRUMap<PathHistory, SaturatingCounter<2>> ChoicePredictionTable;

class Alpha : public OurPredictor {
    // TODO: fields are public'd for testing.
    //       I hate how the private class testing is buggy with doctest... ugh
public:
    // LocalHistoryTable local_history_table = LocalHistoryTable(1024);
    LocalPredictorTable local_predictor_table = LocalPredictorTable(1024);

    LocalHistoryTable local_history_table = LocalHistoryTable(512);
    // LocalPredictorTable local_predictor_table = LocalPredictorTable(512);

    GlobalPredictorTable global_predictor_table = GlobalPredictorTable(4096);
    ChoicePredictionTable choice_prediction_table = ChoicePredictionTable(4096);

    PathHistory global_path_history {};

    virtual ~Alpha() = default;
    virtual bool get_prediction(const branch_record_c* br, const op_state_c* os);
    virtual void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

    void update_local_history_table(unsigned int pc, const bool taken);
    void update_global_path_history(const bool taken);

    // pick whether we want [11:2] mode or [9:0] mode.
    // [11:2] is the spec, but [9:0] "works too"
    TenBitPC get_ten_bits_of(const unsigned int pc) {
        //return get_ten_bits_11_2(pc);
        return get_ten_bits_9_0(pc);
    };

    TenBitPC get_ten_bits_11_2(const unsigned int pc);
    TenBitPC get_ten_bits_9_0(const unsigned int pc);
};

#endif //ALPHA_H
