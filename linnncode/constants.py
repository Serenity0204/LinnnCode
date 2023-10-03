# Maximum size of the cache (adjust as needed)
MAX_CACHE_SIZE = 100

# Timeout for the subprocess execution (in seconds)
EXECUTION_TIMEOUT = 5


CPP_MAIN = """
const bool debug = false;

int main()
{
    LTF::LTF_RUN_ALL(debug, LTF::MODE::CONSOLE);
    return 0;
}

"""
