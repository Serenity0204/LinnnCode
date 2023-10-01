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


@lru_cache(maxsize=MAX_CACHE_SIZE)
def run_cpp(code):
    # Access the global variable
    global ltf_content

    # Create a temporary directory
    temp_dir = tempfile.mkdtemp()

    # Create a temporary file for the C++ code
    code_file = os.path.join(temp_dir, "code.cpp")

    with open(code_file, "w") as file:
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
