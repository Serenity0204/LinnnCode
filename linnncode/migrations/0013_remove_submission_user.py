# Generated by Django 4.1.5 on 2023-10-04 21:04

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('linnncode', '0012_remove_submission_users_submission_user'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='submission',
            name='user',
        ),
    ]
