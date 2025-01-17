/* Author: Chris Wilkerson;   Created: Thu Aug 12 16:19:58 PDT 2004
 * Description: Branch predictor driver.
*/

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <cstdio>
#include <cstdlib>
#include "tread.h"

// include and define the predictor
#include "predictor.h"
PREDICTOR predictor(OurPredictorType::P_Perceptron); // pass the our predictor type here, TODO: make this a program arg?

// usage: predictor <trace>
int main(int argc, char *argv[])
{
    
    // TODO: this is jank
    bool testSet = 0;
    
    std::vector<std::string> args(argv, argv + argc);
    for (size_t i = 1; i < args.size(); ++i)
    {
       
        if (args[i] == "-t")
        {
            testSet = 1;
        }
        if(args[i] =="-a") 
        {
            predictor = PREDICTOR(OurPredictorType::P_Alpha); 
        }
        if(args[i] =="-p")
        {
            predictor = PREDICTOR(OurPredictorType::P_Perceptron);
        }
        else if(args[i] =="-b")
        {
            predictor = PREDICTOR(OurPredictorType::P_NBBP);
        }
        else if(args[i] == "-c")
        {
            predictor = PREDICTOR(OurPredictorType::P_Combination);
        }
        
    }

    if (testSet)
    {
        doctest::Context context;

        // defaults
        context.setOption("abort-after", 5);   // stop test execution after 5 failed assertions
        context.setOption("order-by", "name"); // sort the test cases by their name

        context.applyCommandLine(argc, argv);

        // overrides
        context.setOption("no-breaks", true); // don't break in the debugger when assertions fail

        int res = context.run(); // run

        if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
            return res;           // propagate the result of the tests
    }
    else
    {
        using namespace std;
        

        if (2 >= argc)
        {
            printf("usage: %s <trace>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        cbp_trace_reader_c cbptr = cbp_trace_reader_c(argv[1]);
        branch_record_c br;

        // read the trace, one branch at a time, placing the branch info in br
        while (cbptr.get_branch_record(&br))
        {

            // ************************************************************
            // Competing predictors must have the following methods:
            // ************************************************************

            // get_prediction() returns the prediction your predictor would like to make
            bool predicted_taken = predictor.get_prediction(&br, cbptr.osptr);

            // predict_branch() tells the trace reader how you have predicted the branch
            bool actual_taken = cbptr.predict_branch(predicted_taken);

            // finally, update_predictor() is used to update your predictor with the
            // correct branch result
            predictor.update_predictor(&br, cbptr.osptr, actual_taken);
        }
    }
}