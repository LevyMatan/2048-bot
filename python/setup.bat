@echo off
REM setup.bat

REM Create and activate virtual environment if it doesn't exist
if not exist "venv" (
    echo Creating virtual environment...
    python -m venv venv
)

REM Activate virtual environment and add Scripts to PATH
call venv\Scripts\activate
set PATH=%PATH%;%CD%\venv\Scripts

REM Upgrade pip
python -m pip install --upgrade pip

REM Install build dependencies
pip install hatchling

REM Install the package in editable mode with all extras
pip install -e ".[dev]"

REM Verify installation
python -m game2048.main play --help

echo.
echo Setup complete! The virtual environment is now activated.
echo The game2048 command should now be available.
echo.
echo You can run the game using either:
echo   game2048 play
echo   python -m game2048.main play