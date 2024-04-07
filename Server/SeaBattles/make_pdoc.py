from pdoc.cli import main, parser, os

OUTPUT_DIR = "../docs"

# Programmatically provide settings module
SETTINGS_MODULE = "SeaBattles.settings"
os.environ.setdefault('DJANGO_SETTINGS_MODULE', SETTINGS_MODULE)

# Setup Django
import django
django.setup()

cmdline_args = ["--html", "-o" , OUTPUT_DIR, "Games.views"]

if __name__ == "__main__":
    main(parser.parse_args(cmdline_args))
