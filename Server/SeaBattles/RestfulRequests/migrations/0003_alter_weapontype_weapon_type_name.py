# Generated by Django 5.0.4 on 2024-04-21 20:07

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('RestfulRequests', '0002_alter_field_game'),
    ]

    operations = [
        migrations.AlterField(
            model_name='weapontype',
            name='weapon_type_name',
            field=models.CharField(choices=[('Пушечный выстрел', 'Single Shoot'), ('Самолёт', 'Airplane'), ('Ядерная бомба', 'Nuke Bomb'), ('Мина', 'Mine'), ('Бомба', 'Default Bomb')], max_length=50, unique=True),
        ),
    ]