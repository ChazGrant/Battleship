# Generated by Django 5.0.6 on 2024-05-11 20:50

import django.core.validators
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('RestfulRequests', '0016_alter_missedcell_x_pos_alter_missedcell_y_pos_and_more'),
    ]

    operations = [
        migrations.AlterField(
            model_name='game',
            name='game_invite_id',
            field=models.CharField(default='', max_length=30, null=True),
        ),
        migrations.AlterField(
            model_name='game',
            name='winner_id',
            field=models.IntegerField(default=0, null=True),
        ),
        migrations.AlterField(
            model_name='ship',
            name='ship_length',
            field=models.IntegerField(validators=[django.core.validators.MinValueValidator(1), django.core.validators.MaxValueValidator(4)]),
        ),
        migrations.AlterField(
            model_name='weapon',
            name='weapon_amount',
            field=models.IntegerField(default=0, validators=[django.core.validators.MinValueValidator(0)]),
        ),
        migrations.AlterField(
            model_name='weapontype',
            name='weapon_x_range',
            field=models.IntegerField(validators=[django.core.validators.MinValueValidator(0), django.core.validators.MaxValueValidator(9)]),
        ),
        migrations.AlterField(
            model_name='weapontype',
            name='weapon_y_range',
            field=models.IntegerField(validators=[django.core.validators.MinValueValidator(0), django.core.validators.MaxValueValidator(9)]),
        ),
    ]
