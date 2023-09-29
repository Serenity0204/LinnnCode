import subprocess
import tempfile
import os
import shutil
from functools import lru_cache
from subprocess import TimeoutExpired


# Maximum size of the cache (adjust as needed)
MAX_CACHE_SIZE = 100

# Timeout for the subprocess execution (in seconds)
EXECUTION_TIMEOUT = 5

# Variable to store the content of LTF.h
ltf_content = None

test = """
unsigned long long factorial(int n)
{
    if (n == 0 || n == 1)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

// utility functions like
// inline LTF::LTFStatus <your utility function name>(bool debug = false)
inline LTF::LTFStatus test_utils_main1(bool debug = false)
{
    if (debug)
    {
        LTF_COMMENT("hello this is a message 0");
        LTF_COMMENT("hello this is a message 1");
        LTF_COMMENT("hello this is a message 2");
    }

    return LTF::LTFStatus(LTF::SUCCESS, __LINE__);
}

inline LTF::LTFStatus test_utils_main2(bool debug = false)
{
    LTF_TIME(180);
    double result = 0.0;
    for (int i = 0; i < 100000000; ++i) result += (true ? 1.0 : -1.0) * i;


    return LTF::LTFStatus(LTF::FAIL, __LINE__);
}

inline LTF::LTFStatus test_utils_main3(bool debug = false)
{
    if (debug) LTF_COMMENT("HI");

    int a = 1 + 1;

    if (a == 2) return LTF::LTFStatus(LTF::SUCCESS, __LINE__);
    return LTF::LTFStatus(LTF::FAIL, __LINE__);
}

inline LTF::LTFStatus test_utils_main4(bool debug = false)
{
    if (debug) LTF_COMMENT("should be 120");

    long long f = factorial(5);


    if (f == 120) return LTF::LTFStatus(LTF::SUCCESS, __LINE__);
    return LTF::LTFStatus(LTF::FAIL, __LINE__);
}

inline LTF::LTFStatus test_ignore1(bool debug = false)
{
    return LTF::LTFStatus(LTF::SUCCESS, __LINE__);
}

inline LTF::LTFStatus test_ignore2(bool debug = false)
{
    return LTF::LTFStatus(LTF::FAIL, __LINE__);
}

// test registration, format as (<Test Suite Name>, <Test Case Utility Function>)
LTF_TEST(SUITE1, test_utils_main1);

LTF_TEST(SUITE1, test_utils_main2);

LTF_TEST(SUITE1, test_utils_main4);

LTF_TEST(SUITE2, test_utils_main3);

LTF_TEST(SUITE2, test_utils_main4);

LTF_TEST(SUITE3, test_ignore1);

LTF_TEST(SUITE3, test_ignore2);

LTF_TEST(SUITE_HELLO, test_ignore2);

const bool debug = true;

"""


@lru_cache(maxsize=MAX_CACHE_SIZE)
def run_cpp(code):
    # Access the global variable
    global ltf_content

    # Create a temporary directory
    temp_dir = tempfile.mkdtemp()

    # Create a temporary file for the C++ code
    code_file = os.path.join(temp_dir, "code.cpp")

    if ltf_content is None:
        # Load the content of LTF.h if it's not already loaded
        current_file = os.path.abspath(__file__)
        ltf_file = os.path.join(os.path.dirname(current_file), "LTF", "LTF.h")
        with open(ltf_file, "r") as ltf_file:
            ltf_content = ltf_file.read()

    # In utils.py
    current_file = os.path.abspath(__file__)
    ltf_file = os.path.join(os.path.dirname(current_file), "LTF", "LTF.h")

    with open(code_file, "w") as file:
        file.write(ltf_content)
        ## test
        file.write(test)
        file.write(code)

    # Create a temporary file for the executable
    exe_file = os.path.join(temp_dir, "code.exe")

    try:
        # Compile the C++ code using g++
        compile_result = subprocess.run(
            ["g++", code_file, "-o", exe_file], capture_output=True, text=True
        )

        if compile_result.returncode == 0:
            # Execution if compilation succeeds
            result = None  # Initialize the result variable

            try:
                # Execute the compiled C++ code with a timeout
                result = subprocess.run(
                    [exe_file],
                    capture_output=True,
                    text=True,
                    timeout=EXECUTION_TIMEOUT,
                    check=True,
                )

                # Return the output and error (if any)
                return result.stdout, result.stderr

            except TimeoutExpired:
                # Execution exceeded the timeout
                # Terminate the subprocess if it exists
                if result is not None:
                    subprocess.Popen.terminate(result)
                return None, "Execution Timeout"

            except subprocess.CalledProcessError as e:
                if result is not None:
                    subprocess.Popen.terminate(result)
                return None, "CalledProcessError"
            except Exception as e:
                # Handle other exceptions that may occur during subprocess execution
                if result is not None:
                    subprocess.Popen.terminate(result)
                return None, f"An error occurred: {e}"
        else:
            # Execution if compilation fails
            error_message = compile_result.stderr

            return None, "Compilation Error"

    finally:
        # Remove the temporary directory and its contents
        shutil.rmtree(temp_dir)
