from django.contrib import admin
from .models import Problem, TestCase, TestSuite, Submission


# Register your models here.
class TestCaseInline(admin.TabularInline):
    model = TestCase
    extra = 1


class TestCaseInline(admin.TabularInline):
    model = TestCase
    extra = 1


class TestSuiteAdmin(admin.ModelAdmin):
    inlines = [TestCaseInline]


admin.site.register(TestSuite, TestSuiteAdmin)
admin.site.register(Problem)
admin.site.register(Submission)