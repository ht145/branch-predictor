//
// Created by atsuzaki on 2/22/24.
//

#include "../predictor.h"
#include "../doctest.h"

#include "alpha.h"

bool Alpha::get_prediction(const branch_record_c* br, const op_state_c* os) {
    // unconditional is always taken, so just return true
    if (!br->is_conditional) {
        return true;
    }

    auto pc = get_ten_bits_of(br->instruction_addr);
    auto local_branch_history = local_history_table.get(pc);

    auto local_prediction = local_predictor_table.get(local_branch_history);
    auto global_prediction = global_predictor_table.get(global_path_history);
    auto choice_prediction = choice_prediction_table.get(global_path_history);

    // mux
    return choice_prediction.predict() ? local_prediction.predict() : global_prediction.predict();
}

void Alpha::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) {
    auto pc = get_ten_bits_of(br->instruction_addr);
    auto local_branch_history = local_history_table.get(pc);

    // get prediction again, to update choice prediction table
    auto local_prediction = local_predictor_table.get(local_branch_history).predict();
    auto global_prediction = global_predictor_table.get(global_path_history).predict();

    update_local_history_table(pc, taken);

    // update local & global predictor tables
    local_predictor_table.get(local_branch_history).update(taken);
    global_predictor_table.get(global_path_history).update(taken);

    // update choice prediction based on the following scheme[1].
    // given P1c and P2c denotes "P1 is correct" & "P2 is correct":
    //
    // P1c P2c P1c-P2c
    //  0   0     0    (no change)
    //  0   1    -1    (decrement counter)
    //  1   0     1    (increment counter)
    //  1   1     0    (no change)
    //
    // here, P1 = local, and P2 = global
    //
    // [1] McFarling, Scott. Combining branch predictors.
    //     Vol. 49. Technical Report TN-36, Digital Western Research Laboratory, 1993.

    // if both are correct/incorrect, meaning the predictios are equal
    if (local_prediction == global_prediction) {
        // do nothing
    } else if (local_prediction == taken) { // else, if local prediction is correct
        choice_prediction_table.get(global_path_history).incr();
    } else if (global_prediction == taken) { // else, if global prediction is correct
        choice_prediction_table.get(global_path_history).decr();
    }

    update_global_path_history(taken);
}

void Alpha::update_local_history_table(unsigned int pc, const bool taken) {
    auto local_branch_history = local_history_table.get(pc);
    auto new_history = (local_branch_history <<= 1) |= taken;
    local_history_table.insert(pc, new_history);
}

void Alpha::update_global_path_history(const bool taken) {
    (global_path_history <<= 1) |= taken;
}

// get [11:2] of PC, as per spec
TenBitPC Alpha::get_ten_bits_11_2(const unsigned int pc) {
    return (pc >> 2) & 0b1111111111;
}

// get [9:0] of PC, actually performs slightly better for the traces we have
TenBitPC Alpha::get_ten_bits_9_0(const unsigned int pc) {
    return pc & 0b1111111111;
}

TEST_SUITE("Alpha predictor") {
    TEST_CASE("Alpha: sanity test on shifts") {
        BranchHistory b {0b1000000000};
        CHECK((b <<= 1).test(9) == 0);
        CHECK((b <<= 1).none() == true);
        CHECK(((b <<= 1) |= 1).test(0) == 1);
    }

    TEST_CASE("Alpha: get_ten_bits_11_2() works") {
        Alpha a = Alpha {};
        CHECK(a.get_ten_bits_11_2(0b0000111111111100) == 0b1111111111);
    }

    TEST_CASE("Alpha: get_ten_bits_9_0() works") {
        Alpha a = Alpha {};
        CHECK(a.get_ten_bits_9_0(0b0000111111111100) == 0b1111111100);
    }

    // commented due to issues running on the linux.cecs.pdx.edu server

    /*
    TEST_CASE("Alpha: global path history is updating as expected") {
        Alpha a = Alpha {};

        // initialized as zeroes...
        CHECK(a.global_path_history == 0);

        // interleaving takens and not takens
        a.update_global_path_history(true);
        a.update_global_path_history(false);
        a.update_global_path_history(true);
        a.update_global_path_history(false);

        CHECK(a.global_path_history == 0b000000001010);

        // not taken eight times
        for (size_t i = 0; i < 8; i++)
        {
            a.update_global_path_history(false);
        }
        CHECK(a.global_path_history == 0b101000000000);

        // flood it with takens!
        for (size_t i = 0; i < 8; i++)
        {
            a.update_global_path_history(true);
        }
        CHECK(a.global_path_history == 0b000011111111);
    }

    TEST_CASE("Alpha: local history table is updating as expected") {
        Alpha a = Alpha {};
        auto pc = 1; // dummy pc

        // first access, should init as 0
        CHECK(a.local_history_table.get(pc) == 0);

        // taken three times...
        a.update_local_history_table(pc, true);
        a.update_local_history_table(pc, true);
        a.update_local_history_table(pc, true);

        CHECK(a.local_history_table.get(pc) == 0b0000000111);

        // flood it with nt
        for (size_t i = 0; i < 7; i++)
        {
            a.update_local_history_table(pc, false);
        }
        CHECK(a.local_history_table.get(pc) == 0b1110000000);
    }
    */
}
