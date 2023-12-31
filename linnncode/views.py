from django.shortcuts import render, redirect
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.contrib import messages
from .models import Problem, Submission
from django.core.paginator import Paginator
from .forms import CodeForm
from .driver import TestBuilder, TestDriver


def home_view(request):
    return render(request, "home.html")


# Create your views here.
def login_view(request):
    if request.user.is_authenticated:
        request.session.set_expiry(
            900
        )  # Reset session expiry to 15 minutes (900 seconds)
        return redirect("home")
    if request.method == "POST":
        username = request.POST.get("username")
        password = request.POST.get("password")
        user = authenticate(request, username=username, password=password)
        # login if authenticate success
        if user is not None:
            login(request, user)
            request.session.set_expiry(
                900
            )  # Set session expiry to 15 minutes (900 seconds)
            return redirect("home")
        else:
            message = "Incorrect Username or Password Entered"
            messages.error(request, message)
            return render(request, "login.html")
    else:
        return render(request, "login.html")


def register_view(request):
    if request.user.is_authenticated:
        request.session.set_expiry(
            900
        )  # Reset session expiry to 15 minutes (900 seconds)
        return redirect("home")
    if request.method == "POST":
        username = request.POST.get("username")
        password = request.POST.get("password")
        email = request.POST.get("email")
        # throw error when user exists
        try:
            user = User.objects.create_user(
                username=username, password=password, email=email
            )
            login(request, user)
            request.session.set_expiry(
                900
            )  # Set session expiry to 15 minutes (900 seconds)
            return redirect("home")
        except IntegrityError:
            messages.error(request, "Username Already Exists")
            return redirect("register")
    else:
        return render(request, "register.html")


def logout_view(request):
    logout(request)
    messages.success(request, "User Logged Out")
    return redirect("login")


def problem_view(request):
    request.session.set_expiry(900)
    problems_list = Problem.objects.all().order_by("id")

    paginator = Paginator(problems_list, 5)  # Show 5 problems per page.
    page_number = request.GET.get("page")
    problems = paginator.get_page(page_number)

    context = {"problems": problems}
    return render(request, "problem.html", context)


@login_required(login_url="login")
def problem_detail_view(request, problem_id):
    request.session.set_expiry(900)
    problem = Problem.objects.get(id=problem_id)
    output = None
    err = None
    results = None
    flag = False

    if request.method == "POST":
        form = CodeForm(request.POST)
        if form.is_valid():
            # code for CodeForm Name, and html textarea name
            code = form.cleaned_data["code"]
            # get the language
            language = request.POST.get("language", "cpp")

            # build the test output based on laguage
            test_builder = TestBuilder(language)

            ## for C++ Only
            if language == "cpp":
                # get all the test cases
                cases = problem.test_suite.test_cases.all()
                # get the registration
                registration_count = len(cases)
                cases_list = []
                # extract the strings of test cases
                for case in cases:
                    cases_list.append(case.test_case)
                # pass in list of tests and main, and code, and registration
                test_builder.setup_cpp(cases_list, code, registration_count)
                # build the file and put it into driver
                test_exe = TestDriver(test_builder.build())
                output, err = test_exe.execute_cpp()

                # handling output
                if err:
                    messages.error(request, err)
                    output = None
                else:
                    # decide which one is correct which one is wrong
                    results = TestDriver.extract_cpp_output(output)
                    # no fail, then success
                    if "FAIL" not in results.values():
                        flag = True

                ## either error or not, create submission to the problem
                submission = Submission.objects.create(
                    code=code, problem=problem, user=request.user, success=flag
                )
                submission.save()
            if language == "python":
                pass
    else:
        form = CodeForm(initial={"code": problem.prewritten_code})

    context = {
        "problem": problem,
        "form": form,
        "error": err,
        "results": results,
    }
    return render(request, "problem_detail.html", context)


@login_required(login_url="login")
def submission_view(request, problem_id):
    request.session.set_expiry(900)
    problem = Problem.objects.get(id=problem_id)

    submissions_list = problem.submissions.all().order_by("-date")
    paginator = Paginator(submissions_list, 11)
    page_number = request.GET.get("page")
    submissions = paginator.get_page(page_number)
    context = {"submissions": submissions, "title": problem.title, "problem": problem}
    return render(request, "submissions.html", context)


def problem_search_view(request):
    request.session.set_expiry(900)  # Reset session expiry to 15 minutes (900 seconds)
    query = request.GET.get("query")
    if not query:
        return redirect("problems")

    problem_list = Problem.objects.filter(title__startswith=query).order_by("id")
    paginator = Paginator(problem_list, 5)  # Show 5 problems per page.
    page_number = request.GET.get("page")
    problems = paginator.get_page(page_number)

    context = {"problems": problems}
    return render(request, "problem.html", context)


@login_required(login_url="login")
def my_submission_view(request):
    request.session.set_expiry(900)
    submissions_list = Submission.objects.filter(user=request.user).order_by("-date")
    paginator = Paginator(submissions_list, 11)
    page_number = request.GET.get("page")
    submissions = paginator.get_page(page_number)
    context = {
        "submissions": submissions,
        "title": f"All Submissions Submitted By {request.user}",
        "problem": None,
    }
    return render(request, "submissions.html", context)
