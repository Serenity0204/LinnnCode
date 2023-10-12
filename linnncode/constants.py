# Maximum size of the cache (adjust as needed)
MAX_CACHE_SIZE = 100

# Timeout for the subprocess execution (in seconds)
EXECUTION_TIMEOUT = 5

TREE_NODE_DEF = """

struct Node {
	int val;
	shared_ptr<Node> left;
	shared_ptr<Node> right;
	Node(int val)
	{
		this->val = val;
		this->left = nullptr;
		this->right = nullptr;
	}
};

"""

CPP_MAIN = """
const bool debug = false;

int main()
{
    LTF::LTF_RUN_ALL(debug, LTF::MODE::CONSOLE);
    return 0;
}

"""
