#include "NBBP.h"
#include "../predictor.h"
#include "../doctest.h"


bool NBBP::get_prediction(const branch_record_c* br, const op_state_c* os) {
    if (!br->is_conditional) {
        return true;
    }
    auto pc_bits = get_twelve_bits_of(br->instruction_addr); 

    auto priorProbability = pp_table.get(pc_bits);

    auto Pr_taken = get_cpt_probability(pc_bits, true, priorProbability[1].msb());
    auto Pr_not_taken = get_cpt_probability(pc_bits, false, priorProbability[0].msb());
    
    //printf("pr taken: %d, pr not taken: %d \n", Pr_taken, Pr_not_taken);

    // we just say that if Pr(y=1) is greater than Pr(y=0)
    // means that we "predict taken"?
    auto predict = Pr_taken > Pr_not_taken;
    return predict;
}

void NBBP::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) {
    if (!br->is_conditional) {
        return;
    }
    auto twelve_bit_pc = get_twelve_bits_of(br->instruction_addr); 

    update_prior_probability(twelve_bit_pc, taken);

    if (taken != get_prediction(br, os)) {
        // Update CPT
        for (auto i = 0; i < GBH_LENGTH; i++)
        {
            CPT &cpt = cpt_table.get(twelve_bit_pc);
            auto bit_at_i = gbh[i];

            cpt[taken][bit_at_i][i].incr();
            cpt[taken][!bit_at_i][i].decr();
        }
    }

    gbh = (gbh <<= 1) |= taken;
}

void NBBP::update_prior_probability(const TwelveBitPC pc, bool taken)
{
    PriorProbability& pp = pp_table.get(pc);
    pp[(int) taken].incr();
    pp[(int) !taken].decr();
}

// to avoid mul, we instead sum up the msbs in our counters
// (i.e., count how many they are)
int NBBP::get_cpt_probability(const TwelveBitPC pc, const bool y, const bool P_y) {
    // init result counter with P_y
    auto result = (int)P_y;

    auto cpt = cpt_table.get(pc);
    auto cpt_entry = cpt[(int)y];

    // calc P(x_i | y) and add to result
    for (auto i = 0; i < GBH_LENGTH; i++) {
        // is the bit at x_i 1 or 0?
        auto bit_at_i = gbh[i];

        // index into xs
        auto xs = cpt_entry[bit_at_i];

        // get pr at bit, then get the msb
        result += (int) xs[i].msb();
    }

    return result;
}

// TODO: this actually gets 8 bits
TwelveBitPC NBBP::get_twelve_bits_of(const unsigned int pc) {
    return pc & 0b11111111;
}

TEST_SUITE("NBBP predictor") {
    TEST_CASE("NBBP: NBProbability stays balanced?") {
        PriorProbability pp;
        // when init'd the probabilities (out of possible 16) should be balanced
        CHECK(pp[0].counter == 4);
        CHECK(pp[1].counter == 4);
        CHECK(pp[0].counter + pp[1].counter == 8);

        bool taken = true;

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        // after a bunch of takens, it should update to weigh towards taken
        CHECK(pp[0].counter == 1);
        CHECK(pp[1].counter == 7);
        // and the sum should stay balanced
        CHECK(pp[0].counter + pp[1].counter == 8);

        taken = false;

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        pp[(int) taken].incr();
        pp[(int) !taken].decr();

        // and again...
        CHECK(pp[0].counter == 4);
        CHECK(pp[1].counter == 4);
        CHECK(pp[0].counter + pp[1].counter == 8);
    }
}
