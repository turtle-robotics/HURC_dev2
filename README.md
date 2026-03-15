# Hatchling Universal Robot Controller
this is the platformio project to be uploaded to the actual controller.

# Mac Address Automation Script
To automate Mac Address updating, export the response form as a .csv with the EXACT name 'responses.csv'

Then in your terminal run the code:
    cd "G:\My Drive\VSCode\HURC\HURC_dev2" // file path for HURC_dev2 repository (change to the correct path)
    python scripts\import_peers.py responses.csv src\main.cpp
The script will automatically update main.cpp with the new peers and output a backup file labeled 'main.cpp.bak'.

# How to get responses.csv
For the current semester, there is a forms folder where the mac address form and responses spreadsheets are located. Save the responses spreadsheet as 'responses.csv' and replace the file in the repository with the recent version.

Turtle Development Branch\Hatchling\Hatchling <Semester 20XX> -- <Project Name>
    e.g. for Spring 2026: Turtle Development Branch\Hatchling\Hatchling Spring 2026 -- Mothership TURTLE