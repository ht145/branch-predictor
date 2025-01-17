import subprocess
import sys
import concurrent.futures
import os

traces = [
        "DIST-FP-1",
        "DIST-FP-2",
        "DIST-FP-3",
        "DIST-FP-4",
        "DIST-FP-5",
        "DIST-INT-1",
        "DIST-INT-2",
        "DIST-INT-3",
        "DIST-INT-4",
        "DIST-INT-5",
        "DIST-MM-1",
        "DIST-MM-2",
        "DIST-MM-3",
        "DIST-MM-4",
        "DIST-MM-5",
        "DIST-SERV-1",
        "DIST-SERV-2",
        "DIST-SERV-3",
        "DIST-SERV-4",
        "DIST-SERV-5"
        ]
    

def return_cmd(arg1, arg2):
    if(arg1 == "-r"):
        cmds_list = []
        for trace in traces:
            cmds_list.append("./predictor traces/" + trace + " " + arg2)
    return cmds_list


def run_subprocess(command):
    try:
        # Run the command and capture the output
        result = subprocess.run(command, shell=True, capture_output=True, text=True, check=True)
        return result.stdout.strip()  # Return the stripped output
    except subprocess.CalledProcessError as e:
        print(f"Error executing command '{command}': {e}")
        return None

def run_multiple_subprocesses(commands):
    outputs = []
    with concurrent.futures.ThreadPoolExecutor() as executor:  
        # Submit subprocesses to the executor
        futures = {executor.submit(run_subprocess, command): command for command in commands}
        # Collect results
        for future in concurrent.futures.as_completed(futures):
            command = futures[future]
            try:
                output = future.result()
                if output is not None:
                    outputs.append(command)
                    outputs.append(output)
            except Exception as e:
                print(f"Error executing command '{command}': {e}")
    return outputs

if __name__ == "__main__": 

    if len(sys.argv) > 4: 
        print("Usage: gen_reports.py -r <-a (Alpha) | -p (Perceptron) | -b (NBBP) | -x(for all tests)> <specify_output_file_name>")
        sys.exit(1)

    arg1 = sys.argv[1]
    arg2 = sys.argv[2]
    arg3 = sys.argv[3]
    
    


    commands = return_cmd(arg1, arg2)
    outputs = run_multiple_subprocesses(commands)
    with open(arg3, 'w') as f:
        for i, output in enumerate(outputs): 
            f.write(output + "\n")
            f.write("-" * 20 + "\n")
