from django.db import models


# will have list of test cases into text field
class TestSuite(models.Model):
    title = models.CharField(max_length=50, null=True)
    test_registration = models.TextField(null=True)

    def __str__(self):
        return f"TestSuite: {self.title}"


class TestCase(models.Model):
    title = models.CharField(max_length=50, null=True)
    test_case = models.TextField()

    # one suite can have many test cases
    test_suite = models.ForeignKey(
        TestSuite, on_delete=models.CASCADE, related_name="test_cases", null=True
    )

    def __str__(self):
        return f"TestCase: {self.title}"


# Create your models here.
class Problem(models.Model):
    title = models.CharField(max_length=50)
    description = models.TextField(null=True, blank=True)
    prewritten_code = models.TextField()

    test_suite = models.OneToOneField(
        TestSuite, on_delete=models.CASCADE, related_name="problem", null=True
    )

    def __str__(self):
        return self.title


# user submission
class Submission(models.Model):
    pass
