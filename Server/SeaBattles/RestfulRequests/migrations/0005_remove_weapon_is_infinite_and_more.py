# Generated by Django 5.0.4 on 2024-04-21 21:10

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('RestfulRequests', '0004_weapon_is_infinite'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='weapon',
            name='is_infinite',
        ),
        migrations.AlterField(
            model_name='weapontype',
            name='weapon_type_name',
            field=models.CharField(choices=[('Самолёт', 'Airplane'), ('Ядерная бомба', 'Nuke Bomb'), ('Мина', 'Mine'), ('Бомба', 'Default Bomb')], max_length=50, unique=True),
        ),
    ]