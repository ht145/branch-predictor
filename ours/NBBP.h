#ifndef NBBP_H
#define NBBP_H

#include <array> 
#include <unordered_map>
#include <bitset> 
#include <vector>
#include "LRUMap.h"
#include "OurPredictor.h"
#include "SaturatingCounter.h"

// Based on: https://www.jstage.jst.go.jp/article/nolta/8/3/8_235/_pdf

typedef unsigned int TwelveBitPC;

// paper uses 4 bit saturating counters to represent probabilities in the CPT table
typedef SaturatingCounter<3> NBProbability;

const int GBH_LENGTH = 12;
typedef std::bitset<GBH_LENGTH> GBH; // or, global path history

// This data structure is described in the paper at Sec 2.2, pg.237 
//
// for each x_n bit in branch history, store the probabilities for when x_n = 1 and x_n = 0
// n = GBH_SIZE

// contains two entries: x_one and x_zero
typedef std::array<std::array<NBProbability,GBH_LENGTH>, 2> CPTEntry;

// contains two entries: y_one and y_zero
typedef std::array<CPTEntry,2> CPT;
typedef LRUMap<TwelveBitPC, CPT> CPTTable;

// p(y)
// contains two entries: y_one and y_zero
typedef std::array<NBProbability,2> PriorProbability;

typedef LRUMap<TwelveBitPC, PriorProbability> PPTable;

class NBBP: public OurPredictor {
public:
    GBH gbh;
    CPTTable cpt_table = CPTTable(256); 
    PPTable pp_table = PPTable(256);

    NBBP() = default;
    virtual ~NBBP() = default;
    virtual bool get_prediction(const branch_record_c* br, const op_state_c* os);
    virtual void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

    void update_prior_probability(const TwelveBitPC pc, bool taken);
    int get_cpt_probability(const TwelveBitPC pc, const bool y, const bool P_y);

    // TODO: move these common fns to base class

    TwelveBitPC get_twelve_bits_of(const unsigned int pc);
};

#endif // NBBP_H

