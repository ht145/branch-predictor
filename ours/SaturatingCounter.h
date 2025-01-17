//
// Created by atsuzaki on 2/24/24.
//

#ifndef SATURATINGCOUNTER_H
#define SATURATINGCOUNTER_H

#include <bitset>
#include "../doctest.h"

template <uint32_t N>
class SaturatingCounter {
private:
    bool is_saturated() const {
        // assert(counter <= max_val);
        return counter >= max_val;
    }

public:
    // TODO: should be const but cpp isnt happy with me
    // TODO: un-public these once done with testing
    uint32_t num_of_bits;
    uint32_t max_val;

    uint32_t counter;

    SaturatingCounter()
        : num_of_bits(N),
          max_val( (1 << N) - 1), // 2^num_of_bits
          counter(1 << (N - 1)) // initializing counter as weakly taken
          {}

    void update(bool taken) {
        if (taken) {
            incr();
        } else {
            decr();
        }
    }

    void incr() {
        if (!is_saturated()) {
            counter++;
        }
    }

    void decr() {
        if (counter > 0) {
            counter--;
        }
    }

    bool predict() const {
        // MSB is 1, we predict taken.
        if (msb()) {
            return true;
        } else { // else MSB is 0, we predict not taken.
            return false;
        }
    }

    bool msb() const {
        return (counter >> (N-1)) == 1;
    }
    
    // Helper function; returns true if prediction matches taken
    bool is_correct(bool taken) const {
        return predict() == taken;
    }

    template <uint32_t M>
    static SaturatingCounter<M> sum(const SaturatingCounter<M> a, const SaturatingCounter<M> b) {
        return a.counter | b.counter;
    }

    friend bool operator<(const SaturatingCounter &c1, const SaturatingCounter &c2)
    {
        return c1.counter < c2.counter;
    }

    friend bool operator>(const SaturatingCounter &c1, const SaturatingCounter &c2)
    {
        return c1.counter > c2.counter;
    }
};


TEST_SUITE("SaturatingCounter") {
    TEST_CASE("SaturatingCounter initializes correctly") {
        SaturatingCounter<3> sc = SaturatingCounter<3> {};
    	CHECK(sc.num_of_bits == 3);
    	CHECK(sc.max_val == 7);
    	CHECK(sc.counter == 4);
    }

    TEST_CASE("SaturatingCounter predict() returns the correct prediction based on its state") {
        SaturatingCounter<3> sc = SaturatingCounter<3> {};
        // initializes as 0b100 = 4, so predict will be true
    	CHECK(sc.predict() == true);
     
        sc.incr(); // 5
    	CHECK(sc.predict() == true);
        sc.incr(); // 6
    	CHECK(sc.predict() == true);
        sc.incr(); // 7
    	CHECK(sc.predict() == true);

        sc.decr(); // 6
        sc.decr(); // 5
        sc.decr(); // 4
        sc.decr(); // 3
    	CHECK(sc.predict() == false);
        sc.decr(); // 2
    	CHECK(sc.predict() == false);
        sc.decr(); // 1
    	CHECK(sc.predict() == false);
        sc.decr(); // 0
    	CHECK(sc.predict() == false);
    }


    TEST_CASE("SaturatingCounter saturates") {
        SaturatingCounter<3> sc = SaturatingCounter<3> {};
    
        // Increments way over its max value...
        for (uint32_t i = 0; i < sc.max_val + 10; i++)
        {
            sc.incr();
        }
        // ...but saturates at max_val
    	CHECK(sc.counter == sc.max_val);
    
        // Try for lower bound saturation, too
        for (uint32_t i = 0; i < sc.max_val + 10; i++)
        {
            sc.decr();
        }
    	CHECK(sc.counter == 0);
    }

    // TODO: tests for update()

    // TODO: disassemble elements of these into unit tests
    TEST_CASE("SaturatingCounter counts down correctly") {
        SaturatingCounter<3> sc = SaturatingCounter<3> {};
                int num, diff, taken, new_val;
                bool condition, correct_predict; 
                CHECK(sc.counter == 4);
                for(int i = 0; i < 10; i++)
                { 
                    //printf("%d, %d\n", sc.counter, sc.predict());
                    taken = 0; 
                    num = sc.counter;
                    sc.update(taken);
                    new_val = sc.counter; 
                    diff = num - new_val;
                    condition = (diff == 1 || diff == 0) ? 1 : 0;
                    CHECK(sc.counter >= 0);
                    CHECK(sc.counter <= sc.max_val);
                    CHECK(condition == 1);
                    correct_predict = (sc.counter < 4 && sc.predict() == 0) || (sc.counter >= 4 && sc.predict() == 1) ? 1: 0;     
                    CHECK(correct_predict == 1);
                }
    }

    TEST_CASE("SaturatingCounter counts up correctly") {
        SaturatingCounter<3> sc = SaturatingCounter<3> {};
                int num, diff, taken, new_val;
                bool condition, correct_predict; 
                CHECK(sc.counter == 4);
                for(int i = 0; i < 10; i++)
                { 
                    //printf("%d, %d\n", sc.counter, sc.predict());
                    taken = 1; 
                    num = sc.counter;
                    sc.update(taken);
                    new_val = sc.counter; 
                    diff = new_val - num;
                    condition = (diff == 1 || diff == 0) ? 1 : 0;
                    CHECK(sc.counter >= 0);
                    CHECK(sc.counter <= sc.max_val);
                    CHECK(condition == 1);
                    correct_predict = (sc.counter < 4 && sc.predict() == 0) || (sc.counter >= 4 && sc.predict() == 1) ? 1: 0;     
                    CHECK(correct_predict == 1);

                }
    }

    TEST_CASE("try satcounter counts up correctly w/ two bits") {
        SaturatingCounter<2> sc = SaturatingCounter<2> {};
                int num, diff, taken, new_val;
                bool condition, correct_predict; 
                CHECK(sc.counter == 2);
                for(int i = 0; i < 10; i++)
                { 
                    //printf("%d, %d\n", sc.counter, sc.predict());
                    taken = 1; 
                    num = sc.counter;
                    sc.update(taken);
                    new_val = sc.counter; 
                    diff = new_val - num;
                    condition = (diff == 1 || diff == 0) ? 1 : 0;
                    CHECK(sc.counter >= 0);
                    CHECK(sc.counter <= sc.max_val);
                    CHECK(condition == 1);
                    correct_predict = (sc.counter < 2 && sc.predict() == 0) || (sc.counter >= 2 && sc.predict() == 1) ? 1: 0;     
                    CHECK(correct_predict == 1);
                }
    }

    TEST_CASE("try satcounter counts down correctly w/ two bits") {
        SaturatingCounter<2> sc = SaturatingCounter<2> {};
                int num, diff, taken, new_val;
                bool condition, correct_predict; 
                CHECK(sc.counter == 2);
                for(int i = 0; i < 10; i++)
                { 
                    //printf("%d, %d\n", sc.counter, sc.predict());
                    taken = 0; 
                    num = sc.counter;
                    sc.update(taken);
                    new_val = sc.counter; 
                    diff = num - new_val;
                    condition = (diff == 1 || diff == 0) ? 1 : 0;
                    CHECK(sc.counter >= 0);
                    CHECK(sc.counter <= sc.max_val);
                    CHECK(condition == 1);
                    correct_predict = (sc.counter < 2 && sc.predict() == 0) || (sc.counter >= 2 && sc.predict() == 1) ? 1: 0;     
                    CHECK(correct_predict == 1);
                }
    }
}

#endif //SATURATINGCOUNTER_H
