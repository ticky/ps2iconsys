@echo off
echo Clear existing data...
del /s /q .\doc\doxygen\*.*
echo  done.
echo Running Doxygen...
doxygen DOXYGEN.CFG
echo  done.
pause