# will get the base code and build the test cases with user's code into 1 single file
class TestBuilder:
    def __init__(self, language: str) -> None:
        self._language = language

    def build(self):
        pass


# will take the TestBuilder's 1 single file and execute them
class TestDriver:
    def __init__(self) -> None:
        pass

    def execute_cpp(self):
        pass

    def execute_python(self):
        pass

    def execute_javascript(self):
        pass
