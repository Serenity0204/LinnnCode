from django.shortcuts import render, redirect
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.contrib import messages
from .models import Problem
from django.core.paginator import Paginator
from .forms import CodeForm


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
    code = None
    if request.method == "POST":
        form = CodeForm(request.POST)
        if form.is_valid():
            code = form.cleaned_data["code_form"]
            ## run driver later
    else:
        pass

    print(str(code))
    context = {"problem": problem}
    return render(request, "problem_detail.html", context)


# request.session.set_expiry(900)
#     context = {}
#     exercise = Exercise.objects.get(id=exercise_id)
#     answer = exercise.answer

#     if request.method == "POST":
#         form = CppForm(request.POST)
#         if form.is_valid():
#             cpp_code = form.cleaned_data["cpp"]
#             correct_msg = ""
#             correct = False
#             try:
#                 output, error = run_cpp(cpp_code)
#                 if str(output).strip() == str(answer).strip():
#                     correct_msg = f"The Answer Is Correct, Congrats!"
#                     correct = True
#                 else:
#                     if error:
#                         correct_msg = "Compilation Error"
#                     else:
#                         correct_msg = f"The Answer Is Not Correct.\nExpected:\n{answer}\nBut Found:\n{output}"
#                 context = {
#                     "form": form,
#                     "error": error,
#                     "correct_msg": correct_msg,
#                     "correct": correct,
#                     "exercise": exercise,
#                 }
#             except Exception as e:
#                 correct_msg = str(e)
#                 context = {
#                     "form": form,
#                     "error": str(e),
#                     "correct_msg": correct_msg,
#                     "correct": correct,
#                     "exercise": exercise,
#                 }
#     else:
#         form = CppForm(initial={"cpp": exercise.prewritten_code})
#         context = {
#             "form": form,
#             "exercise": exercise,
#         }
#     return render(request, "exercise_detail.html", context)
