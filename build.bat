@echo off

call "%~dp0Source\__Brahma\brahma.bat" -lib_search_dir "%~dp0Source" -build_tool_path "%~dp0Intermediate\__Brahma\brahma" %*
