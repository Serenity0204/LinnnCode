# Generated by Django 4.2.2 on 2023-10-07 02:35

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('linnncode', '0014_submission_user'),
    ]

    operations = [
        migrations.AddField(
            model_name='submission',
            name='success',
            field=models.BooleanField(default=False, null=True),
        ),
    ]