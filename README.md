# SVGParser
Created by: Annaliese Schropp

Date: January 28, 2020

Version: 1.0

This program creates a library file that will parse a simple SVG (Only contains group, paths, circles, rectangles) files into a valid SVG struct that includes titles (if any), description (if any) and namespace as well as lists, in hierarchical order, of the elements in the file. The final list otherAttributes contains any other features of each element such as the fill property and stores it in a list here. 

In addition to parsing the file, the functionality of this allows the user to get a doubly linked list of **all** (includes nested elements in groups) the rectangle, circles, paths or groups. You can check if specific areas or path values are inside the SVG image as well as get the number of otherAttributes in the entire file. 

To run, you need an SVG file and a main as this just creates a valid dynamic library file.
