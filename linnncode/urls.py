from django.urls import path
from . import views

urlpatterns = [
    path("", views.home_view, name="home"),
    path("login/", views.login_view, name="login"),
    path("register/", views.register_view, name="register"),
    path("logout/", views.logout_view, name="logout"),
    path("problems/", views.problem_view, name="problems"),
    path(
        "problems/<int:problem_id>/", views.problem_detail_view, name="problem_detail"
    ),
    path(
        "problems/<int:problem_id>/submissions",
        views.submission_view,
        name="submissions",
    ),
    path("search-problem/", views.problem_search_view, name="problem_search"),
    path("my-submissions/", views.my_submission_view, name="my_submissions")
]
