@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM Usage: checkout-branch <branch-name>
if "%~1"=="" (
    echo Please provide a branch name.
    echo Example: checkout-branch codex/update-button-styles-and-layout
    exit /b 1
)

set BRANCH=%~1

REM Ensure we're inside a Git repo
git rev-parse --is-inside-work-tree 1>nul 2>nul
if errorlevel 1 (
    echo This directory is not a Git repository.
    exit /b 1
)

REM Determine current branch (may be HEAD in detached state)
for /f "usebackq delims=" %%B in (`git rev-parse --abbrev-ref HEAD 2^>nul`) do set CURRENT=%%B

REM Check that the remote branch exists on origin (avoid typos)
for /f "usebackq delims=" %%R in (`git ls-remote --heads origin %BRANCH%`) do set REMOTE_REF=%%R

if not defined REMOTE_REF (
    echo Remote branch "origin/%BRANCH%" was not found.
    echo Tip: list matching remote branches with:  git branch -r ^| findstr /I /C:"%BRANCH%"
    exit /b 1
)

REM Does a local branch of the same name already exist?
git show-ref --verify --quiet refs/heads/%BRANCH%
if errorlevel 1 (
    REM Local branch does not exist yet -> create it to track origin and switch
    echo Creating local branch "%BRANCH%" tracking origin/%BRANCH%...
    git fetch origin %BRANCH% 1>nul
    if errorlevel 1 (
        echo Failed to fetch origin/%BRANCH%.
        exit /b 1
    )
    git switch -c %BRANCH% --track origin/%BRANCH%
    if errorlevel 1 (
        echo Failed to create/switch to branch %BRANCH%.
        exit /b 1
    )
    echo Switched to new tracking branch "%BRANCH%".
    exit /b 0
)

REM Local branch exists already
if /i "%CURRENT%"=="%BRANCH%" (
    REM We're already on the target branch.
    echo Already on "%BRANCH%". Fast-forwarding from origin/%BRANCH%...
    git fetch origin %BRANCH% 1>nul
    if errorlevel 1 (
        echo Failed to fetch origin/%BRANCH%.
        exit /b 1
    )
    REM Try a clean fast-forward update; avoid the "refusing to fetch into checked out branch" issue
    git merge --ff-only origin/%BRANCH%
    if errorlevel 1 (
        echo Fast-forward merge failed. You may have local commits or conflicts.
        echo Consider: git pull --rebase origin %BRANCH%
        exit /b 1
    )
    echo Updated "%BRANCH%" to the latest origin/%BRANCH%.
    exit /b 0
) else (
    REM Different current branch -> ensure we have the latest remote tip, then switch
    echo Fetching origin/%BRANCH% and switching...
    git fetch origin %BRANCH% 1>nul
    if errorlevel 1 (
        echo Failed to fetch origin/%BRANCH%.
        exit /b 1
    )
    git switch %BRANCH%
    if errorlevel 1 (
        echo Failed to switch to branch %BRANCH%.
        exit /b 1
    )
    REM Ensure the local branch tracks origin/%BRANCH% (in case it wasn't set)
    git rev-parse --abbrev-ref --symbolic-full-name @{u} 1>nul 2>nul
    if errorlevel 1 (
        git branch --set-upstream-to=origin/%BRANCH% %BRANCH% 1>nul
    )
    echo Switched to "%BRANCH%" and tracking origin/%BRANCH%.
    exit /b 0
)
