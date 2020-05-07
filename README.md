# SVG Parser GUI
Created by: Annaliese Schropp

Date: May 7, 2020

Version: 2.0

This program creates a library file that will parse a simple SVG (Only contains group, paths, circles, rectangles) files into a valid SVG struct that includes titles (if any), description (if any) and namespace as well as lists, in hierarchical order, of the elements in the file. The final list otherAttributes contains any other features of each element such as the fill property and stores it in a list here. 

In addition to parsing the file, the functionality of this allows the user to get a doubly linked list of **all** (includes nested elements in groups) the rectangle, circles, paths or groups. You can check if specific areas or path values are inside the SVG image as well as get the number of otherAttributes in the entire file. 

New Features Since Last Update in January:
* DB queries and complete front end usability using mysql, expressjs and nodejs
##Features:##
* Login
* Table that lists known files on the server
  * upload button to put existing files on the server
* Table that allows you to view a specific file's features and make changes
  * can change the title, description
  * can view the nest features that make up the svg and be able to edit those
  * added scalability to specific objects
* Option to create a new file with a title and description
* Can add a specified shape to an existing file with all the features associated with that shape
* Can access database information:
  * Status: the current state of the database
  * Clear data: removes all the data stored on the database
  * Store files: will store all existing files stored on the server into a table on the database
  * Execute Query: a dropdown that gives the user a change to interact with the database tables, after the proper form is filled out the user to will see a table that shows the desired information
