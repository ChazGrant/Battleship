@echo off
IF "%~1"=="-drf" ( 
    start python manage.py runserver 127.0.0.1:8000
) ELSE IF "%~1"=="-wss" (
    start uvicorn SeaBattles.asgi:application --host 127.0.0.1 --port 8080 --reload
) ELSE (
    IF "%~1"=="" start python manage.py runserver 127.0.0.1:8000 && start uvicorn SeaBattles.asgi:application --host 127.0.0.1 --port 8080
)
