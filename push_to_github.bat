@echo off
REM Vital Synthesizer - GitHub Push Script (Windows)
REM This script will push your Vital synthesizer project to GitHub

echo =========================================
echo   Vital Synthesizer - GitHub Push
echo =========================================
echo.

REM Check if we're in a git repository
if not exist ".git" (
    echo Error: Not in a git repository!
    echo Please run this script from the vital_application directory
    pause
    exit /b 1
)

echo Git repository detected
echo.

REM Set GitHub repository URL
set GITHUB_REPO=https://github.com/12Matt3r/vital.git

echo Target Repository: %GITHUB_REPO%
echo.

REM Get current branch
for /f "tokens=*" %%i in ('git branch --show-current') do set CURRENT_BRANCH=%%i

REM Rename branch to main if it's master
if "%CURRENT_BRANCH%"=="master" (
    echo Renaming branch 'master' to 'main'...
    git branch -M main
    echo Branch renamed to 'main'
) else (
    echo Current branch: %CURRENT_BRANCH%
)
echo.

REM Check if remote exists
git remote | findstr /C:"origin" >nul
if errorlevel 1 (
    echo Adding remote origin...
    git remote add origin %GITHUB_REPO%
    echo Remote added successfully
) else (
    echo Remote 'origin' already exists
    for /f "tokens=*" %%i in ('git remote get-url origin') do echo    Current URL: %%i
)
echo.

REM Show current status
echo Current Status:
git log --oneline -1
echo.

REM Ask for confirmation
echo WARNING: This will push to GitHub
echo    Repository: %GITHUB_REPO%
for /f "tokens=*" %%i in ('git branch --show-current') do echo    Branch: %%i
echo.
set /p CONFIRM="Do you want to continue? (y/N): "

if /i not "%CONFIRM%"=="y" (
    echo Push cancelled
    pause
    exit /b 0
)

REM Push to GitHub
echo.
echo Pushing to GitHub...
echo.

for /f "tokens=*" %%i in ('git branch --show-current') do set PUSH_BRANCH=%%i
git push -u origin %PUSH_BRANCH%

if errorlevel 0 (
    echo.
    echo =========================================
    echo SUCCESS! Project pushed to GitHub
    echo =========================================
    echo.
    echo Your Vital synthesizer is now on GitHub!
    echo.
    echo View it at:
    echo    https://github.com/12Matt3r/vital
    echo.
    echo Next steps:
    echo 1. Visit the repository on GitHub
    echo 2. Update repository description and topics
    echo 3. Enable GitHub Actions for CI/CD
    echo 4. Share your amazing work!
) else (
    echo.
    echo =========================================
    echo PUSH FAILED
    echo =========================================
    echo.
    echo Common reasons:
    echo 1. Authentication failed - You may need to:
    echo    - Use GitHub CLI: gh auth login
    echo    - Or use Personal Access Token
    echo    - Or set up SSH keys
    echo.
    echo 2. Repository doesn't exist or you don't have access
    echo    - Create the repository on GitHub first
    echo    - Make sure you have push permissions
    echo.
    echo 3. Repository already has content
    echo    - Use: git push -u origin main --force
    echo    (Warning: This will overwrite existing content^)
    echo.
    echo For help, visit: https://docs.github.com/en/get-started
)

echo.
pause
