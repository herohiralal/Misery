@echo off

call "%~dp0Source\__Brahma\brahma.bat" -package Misery -lib_search_dir "%~dp0Source" -intermediate_output "%~dp0Intermediate\__Brahma" -out "%~dp0Binaries" -clangd "%~dp0." -warnings -warn_as_err %*
