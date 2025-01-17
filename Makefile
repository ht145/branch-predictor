# Author: Jared Stark;   Created: Mon Aug 16 11:28:20 PDT 2004
# Description: Makefile for building a cbp submission.

CFLAGS = -g -Wall
CXXFLAGS = -g -Wall -std=c++20 -O3

objects = cbp_inst.o main.o op_state.o predictor.o tread.o ours/alpha.o ours/Perceptron.o ours/NBBP.o ours/combination.o

predictor: $(objects)
	$(CXX) -o $@ $(objects)

cbp_inst.o : cbp_inst.h cbp_assert.h cbp_fatal.h cond_pred.h finite_stack.h indirect_pred.h stride_pred.h value_cache.h
main.o : tread.h cbp_inst.h predictor.h op_state.h
alpha.o : ours/alpha.h ours/LRUMap.h ours/SaturatingCounter.h ours/OurPredictor.h
Perceptron.o : ours/Perceptron.h 
combination.o : ours/combination.h 
NBBP.o : ours/NBBP.h
op_state.o : op_state.h
predictor.o : predictor.h op_state.h tread.h cbp_inst.h
tread.o : tread.h cbp_inst.h op_state.h

.PHONY : clean
clean :
	rm -f predictor $(objects)

