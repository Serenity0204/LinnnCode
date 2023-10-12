from typing import List, Dict
from .utils import run_cpp, match_cpp_output, build_registration
import os


# will get the base code and build the test cases with user's code into 1 single file
class TestBuilder:
    LTF = None

    @classmethod
    def init_LTF(cls) -> None:
        if cls.LTF is not None:
            return
        current_file = os.path.abspath(__file__)
        ltf_file = os.path.join(os.path.dirname(current_file), "LTF", "LTF.h")
        with open(ltf_file, "r") as ltf_file:
            cls.LTF = ltf_file.read()

    def __init__(self, language: str) -> None:
        TestBuilder.init_LTF()
        self._language = language
        self._exe = None

    def build(self) -> str:
        if self._exe is None:
            return ""
        return self._exe

    def language(self) -> str:
        return self._language

    def setup_cpp(
        self, tests: List[str], main: str, code: str, registration_count: int
    ):
        # LTF
        exe = TestBuilder.LTF + "\n"
        # code
        exe += code + "\n"
        # Test case
        for test in tests:
            exe += "\n" + test + "\n"
        # registrations
        exe += build_registration(registration_count) + "\n"
        # main
        exe += main + "\n"
        # assign
        self._exe = exe

    def setup_python(self) -> str:
        pass


# will take the TestBuilder's 1 single file and execute them
class TestDriver:
    @classmethod
    def extract_cpp_output(cls, input_str) -> Dict:
        return match_cpp_output(input_str)

    def __init__(self, exe) -> None:
        self._exe = exe

    def execute_cpp(self):
        try:
            output, error = run_cpp(self._exe)
            return output, error
        except Exception as e:
            return "", str(e)

    def execute_python(self):
        pass
