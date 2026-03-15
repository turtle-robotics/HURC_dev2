# Hatchling Universal Robot Controller
this is the platformio project to be uploaded to the actual controller.

To automate Mac Address updating export the response form as a .csv with the EXACT name 'responses.csv'

Then in your terminal run the code (different for each laptop)
    cd "G:\My Drive\VSCode\HURC\HURC_dev2"
    python scripts\import_peers.py responses.csv src\main.cpp

The output should be in the src folder labeled 'main.cpp.bak'. It is an exact copy of main.cpp just with updated peer names.
