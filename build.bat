@echo off

call "%~dp0Source\__Brahma\brahma.bat" -lib_search_dir "%~dp0Source" -intermediate_output "%~dp0Intermediate\__Brahma" -out "%~dp0Binaries" -warnings -warn_as_err %*
