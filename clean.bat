@echo off

if exist build (
    rmdir /s /q build Release Debug
)

if exist GlCraft.exe (
    del /q GlCraft.exe
)
