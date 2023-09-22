from django.db import models


# Create your models here.
class Problem(models.Model):
    pass


## has 3 fields, c++, python, and javascript, and will be associate to problem
class Solution(models.Model):
    pass


# will have list of test cases into text field
class TestSuite(models.Model):
    pass
