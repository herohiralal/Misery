@echo off

call "%~dp0Source\__Brahma\brahma.bat" -package MiseryDependencies -lib_search_dir "%~dp0Source" -intermediate_output "%~dp0Intermediate\__Brahma" -out "%~dp0Source\MiseryDeps\Dependencies" -nodebuginfo -optimised %*
