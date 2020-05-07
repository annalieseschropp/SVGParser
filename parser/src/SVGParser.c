/**
* Annaliese Schropp
* 1013472
* aschropp@uoguelph.ca
* January 28,2020
* CIS*2750
* SVG Parser Assignment 1
**/
#include "SVGParser.h"
#include "Helper.h"

/**********************************************************
 * Author: Dodji Seketeli
 * Code Version: Unknown
 * Title: tree1
 * http://www.xmlsoft.org/examples/tree1.c
 * 
 * The following lines have used this code as a source:
 * 60, 61, 64, 76, 84, 86, 89, 90, 104, 106, 107, 147
 * 
 **********************************************************/

/** Function to initialize all lists within the SVGimage stuct and set the other attributes to empty.
*@post Everthing within the struct is initialized or empty.
*@param img - a malloced pointer to the SVG
**/
void initImg (SVGimage *img) {
  strcpy(img->namespace, "");
  strcpy(img->title, "");
  strcpy(img->description, "");
  img->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
  img->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
  img->paths = initializeList(&pathToString, &deletePath, &comparePaths);
  img->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
  img->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
}

/** Function to initialize all lists within the Group stuct.
*@post Everthing within the struct is initialized (empty).
*@param group - a malloced pointer to the Group.
**/
void initGroup(Group* group) {
  group->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
  group->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
  group->paths = initializeList(&pathToString, &deletePath, &comparePaths);
  group->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
  group->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
}

/** Function to create an SVG object based on the contents of an SVG file.
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid SVGimage has been created and its address was returned
		or 
		An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the SVG file
**/
SVGimage* createSVGimage(char* fileName) {
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  SVGimage *newImg = malloc(sizeof(SVGimage));

  LIBXML_TEST_VERSION
  initImg(newImg);

  //Checks if it is an svg file
  if (fileName == NULL) {
    deleteSVGimage(newImg);
    return NULL;
  } else if (strstr(fileName, ".svg") == NULL) {
    deleteSVGimage(newImg);
    return NULL;
  }

  doc = xmlReadFile(fileName, NULL, 0);

  if (doc == NULL) {
    xmlFreeDoc(doc);
    xmlCleanupParser();
    deleteSVGimage(newImg);
    return NULL;
  }

  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);

  get_elements(root_element, newImg);

  if (strlen(newImg->namespace) == 0) {
    xmlFreeDoc(doc);
    xmlCleanupParser();
    deleteSVGimage(newImg);
    return NULL;
  }

  /*Clean up the parser*/
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return newImg;
}

/** Function wrapper traverses an SVG tree and will pull the necessary information out and store it in various structs
* which are stored in one big SVGimage struct.
*@pre the xmlNode is not null and the img has been initialized
*@post a populated img struct
*@return void
*@param a_node - the head/root node of the tree
*@param img - the initalized img struct
**/
void get_elements(xmlNode* a_node, SVGimage* img) {
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {

      if ((strcmp((char*)cur_node->name, "svg") == 0) || (strcmp((char*)cur_node->name, "SVG") == 0)) {
        if (cur_node->ns->href) {
          strcpy(img->namespace, (char*)cur_node->ns->href);
        } else {
          return;
        }

        for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
          Attribute* attr = malloc(sizeof(Attribute));
          attr->name = malloc(sizeof(char) * strlen((char*)tmpAttr->name) + 1);
          strcpy(attr->name, (char*)tmpAttr->name);
          attr->value = malloc(sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
          strcpy(attr->value, (char*)tmpAttr->children->content);
          insertBack(img->otherAttributes, attr);
        }

      } else if ((strcmp((char*)cur_node->name, "title") == 0) || (strcmp((char*)cur_node->name, "TITLE") == 0)) {
        if (strlen((char*)cur_node->children->content) < 256) {
          strcpy(img->title, (char*)cur_node->children->content);
        } else {
          strncpy(img->title, (char*)cur_node->children->content, 256);
          strcat(img->title, "\0");
        }
        
      } else if ((strcmp((char*)cur_node->name, "desc") == 0) || (strcmp((char*)cur_node->name, "DESC") == 0)){
        if (strlen((char*)cur_node->children->content) < 256) {
          strcpy(img->description, (char*)cur_node->children->content);
        } else {
          strncpy(img->description, (char*)cur_node->children->content, 256);
          strcat(img->title, "\0");
        }

      } else if ((strcmp((char*)cur_node->name, "g") == 0) && (strcmp((char*)cur_node->parent->name, "g") != 0)) {
        Group* newGroup = createGroup(cur_node, img);
        insertBack(img->groups, newGroup);

      } else if ((strcmp((char*)cur_node->name, "path") == 0) && (strcmp((char*)cur_node->parent->name, "g") != 0)) {
        Path* newPath = createPath(cur_node);
        insertBack(img->paths, newPath);

      } else if ((strcmp((char*)cur_node->name, "circle") == 0) && (strcmp((char*)cur_node->parent->name, "g") != 0)) {
        Circle* newCircle = createCircle(cur_node);
        insertBack(img->circles, newCircle);

      } else if ((strcmp((char*)cur_node->name, "rect") == 0) && (strcmp((char*)cur_node->parent->name, "g") != 0)) {
        Rectangle* newRect = createRect(cur_node);
        insertBack(img->rectangles, newRect);

      } else {
        //Do nothing
      }
    }
    get_elements(cur_node->children, img);
  }
}

/** Function creates a new circle struct and populates it with the desired data
*@pre current node and SVG image is not NULL
*@post populated circle struct
*@return a newly allocated Circle struct to add to the list
*@param cur_node - current node in the SVG parsed tree
*@param img - pointer to the svg img
**/
Circle* createCircle (xmlNode* cur_node) {
  Circle* newCircle = malloc(sizeof(Circle));
  newCircle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  newCircle->cx = 0;
  newCircle->cy = 0;
  newCircle->r = 0;
  strcpy(newCircle->units, "");
  char* units;

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if ((strcmp((char*)tmpAttr->name, "cx") == 0) || (strcmp((char*)tmpAttr->name, "CX") == 0)) {
      newCircle->cx = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newCircle->units, units);

    } else if ((strcmp((char*)tmpAttr->name, "cy") == 0) || (strcmp((char*)tmpAttr->name, "CY") == 0)) {
      newCircle->cy = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newCircle->units, units);

    } else if ((strcmp((char*)tmpAttr->name, "r") == 0) || (strcmp((char*)tmpAttr->name, "R") == 0)) {
      newCircle->r = atof((char*)tmpAttr->children->content);

    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(sizeof(char) * strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
      strcpy(attr->value, (char*)tmpAttr->children->content);

      insertBack(newCircle->otherAttributes, attr);
    }
  }

  return newCircle;
}

/** Function creates a new rectangle struct and populates it with the desired data
*@pre current node and SVG image is not NULL
*@post populated rectangle struct
*@return a newly allocated Rectangle struct to add to the list
*@param cur_node - current node in the SVG parsed tree
*@param img - pointer to the svg img
**/
Rectangle* createRect (xmlNode* cur_node) {
  Rectangle* newRect = malloc(sizeof(Rectangle));
  newRect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
  newRect->x = 0;
  newRect->y = 0;
  newRect->height = 0;
  newRect->width = 0;
  strcpy(newRect->units, "");
  char* units;

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if ((strcmp((char*)tmpAttr->name, "x") == 0) || (strcmp((char*)tmpAttr->name, "X") == 0)) {
      newRect->x = atof((char*)tmpAttr->children->content);

    } else if ((strcmp((char*)tmpAttr->name, "y") == 0) || (strcmp((char*)tmpAttr->name, "Y") == 0)) {
      newRect->y = atof((char*)tmpAttr->children->content);

    } else if ((strcmp((char*)tmpAttr->name, "width") == 0) || (strcmp((char*)tmpAttr->name, "WIDTH") == 0)) {
      newRect->width = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newRect->units, units);

    } else if ((strcmp((char*)tmpAttr->name, "height") == 0) || (strcmp((char*)tmpAttr->name, "HEIGHT") == 0)) {
      newRect->height = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newRect->units, units);
    
    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(sizeof(char) * strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
      strcpy(attr->value, (char*)tmpAttr->children->content);

      insertBack(newRect->otherAttributes, attr);
    }
  }
  return newRect;
}

/** Function creates a new path struct and populates it with the desired data
*@pre current node and SVG image is not NULL
*@post populated path struct
*@return a newly allocated Path struct to add to the list
*@param cur_node - current node in the SVG parsed tree
*@param img - pointer to the svg img
**/
Path* createPath(xmlNode* cur_node) {
  Path* newPath = malloc(sizeof(Path));
  newPath->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  newPath->data = malloc(sizeof(char) * 1);
  strcpy(newPath->data, "");

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if ((strcmp((char*)tmpAttr->name, "d") == 0) || (strcmp((char*)tmpAttr->name, "D") == 0)) {
      newPath->data = realloc(newPath->data, sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
      strcpy(newPath->data, (char*)tmpAttr->children->content);

    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(sizeof(char) * strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
      strcpy(attr->value, (char*)tmpAttr->children->content);

      insertBack(newPath->otherAttributes, attr);
    }
  }
  return newPath;
}

/** Function creates a new group struct and populates it with the desired data
*@pre current node and SVG image is not NULL
*@post populated group struct
*@return a newly allocated Group struct to add to the list
*@param cur_node - current node in the SVG parsed tree
*@param img - pointer to the svg img
**/
Group* createGroup(xmlNode* cur_node, SVGimage* img) {
  Group* newGroup = malloc(sizeof(Group));
  initGroup(newGroup);
  
  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    Attribute* attr = malloc(sizeof(Attribute));

    attr->name = malloc(sizeof(char) * strlen((char*)tmpAttr->name) + 1);
    strcpy(attr->name, (char*)tmpAttr->name);
    attr->value = malloc(sizeof(char) * strlen((char*)tmpAttr->children->content) + 1);
    strcpy(attr->value, (char*)tmpAttr->children->content);

    insertBack(newGroup->otherAttributes, attr);
  }

  cur_node = cur_node->children;
  for (xmlNode* tmpNode = cur_node; tmpNode != NULL; tmpNode = tmpNode->next) {
    if ((((strcmp((char*)tmpNode->name, "g") == 0)) || (strcmp((char*)tmpNode->name, "G") == 0)) && ((strcmp((char*)tmpNode->parent->name, "g") == 0) || (strcmp((char*)tmpNode->parent->name, "G") == 0))) {
        Group* nestedGroup = createGroup(tmpNode, img);
        insertBack(newGroup->groups, nestedGroup);

    } else if (((strcmp((char*)tmpNode->name, "path") == 0) || (strcmp((char*)tmpNode->name, "PATH") == 0)) && ((strcmp((char*)tmpNode->parent->name, "g") == 0) || (strcmp((char*)tmpNode->parent->name, "G") == 0))) {
        Path* newPath = createPath(tmpNode);
        insertBack(newGroup->paths, newPath);

    } else if (((strcmp((char*)tmpNode->name, "circle") == 0) || (strcmp((char*)tmpNode->name, "CIRCLE") == 0)) && ((strcmp((char*)tmpNode->parent->name, "g") == 0) || (strcmp((char*)tmpNode->parent->name, "G") == 0))) {
        Circle* newCircle = createCircle(tmpNode);
        insertBack(newGroup->circles, newCircle);

    } else if (((strcmp((char*)tmpNode->name, "rect") == 0) || (strcmp((char*)tmpNode->name, "RECT") == 0)) && ((strcmp((char*)tmpNode->parent->name, "g") == 0) || (strcmp((char*)tmpNode->parent->name, "G") == 0))) {
        Rectangle* newRect = createRect(tmpNode);
        insertBack(newGroup->rectangles, newRect);

    } else {}
  }
  return newGroup;
}

/** Function to create a string representation of an SVG object.
 *@pre SVGimgage exists, is not null, and is valid
 *@post SVGimgage has not been modified in any way, and a string representing the SVG contents has been created
 *@return a string contaning a humanly readable representation of an SVG object
 *@param img - a pointer to an SVG struct
**/
char* SVGimageToString(SVGimage* img) {
  if (img) {
    char* string = malloc(strlen(img->namespace) + strlen(img->title) + strlen(img->description) + 3);
    strcpy(string, img->namespace);
    if (strlen(img->namespace) > 0) {
      strcat(string, "\n");
    }
    strcat(string, img->title);
    if (strlen(img->title) > 0) {
      strcat(string, "\n");
    }
    strcat(string, img->description);

    return string;
  } else {
    char* string = malloc(strlen("Image is NULL") + 1);
    strcpy(string, "Image is NULL");
    return string;
  }
}

/** Function to delete image content and free all the memory.
 *@pre SVGimgage  exists, is not null, and has not been freed
 *@post SVSVGimgageG  had been freed
 *@return void
 *@param img - a pointer to an SVG struct
**/
void deleteSVGimage(SVGimage* img) {
	if (img) {
    freeList(img->rectangles);
    freeList(img->circles);
    freeList(img->paths);
    freeList(img->groups);
    freeList(img->otherAttributes);
    free(img);
  }
}

/** Function creates a list of pointers to all the rectangles in the img
*@pre SVGimgage exists, is not null, and has not been freed
*@post SVGimgage has not been modified in any way
*@return a newly allocated List of components.  While the List struct itself is new, the components in it are just pointers
to the ones in the image.
*@param obj - a pointer to an SVG struct
*/
List* getRects(SVGimage* img) {
  List* rectList = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
  
  if (img) {
    int numRects = getLength(img->rectangles);
    int lenRect;
    int lenGroup;
    Group* gg;

    if (numRects > 0) {
      ListIterator iter = createIterator(img->rectangles); 
      Node* tmp;

      for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter)) {
        insertBack(rectList, ((Rectangle*)tmp));
      }
    }
    numRects = getLength(img->groups);

    if (numRects > 0) {
      ListIterator groupIter = createIterator(img->groups);
      Node* groupTmp;
      for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
        gg = ((Group*)groupTmp);
        lenGroup = getLength(gg->groups);
        lenRect = getLength(gg->rectangles);
        
        if ((lenGroup > 0) || (lenRect > 0)) {
          getNestedRects(rectList, gg);
        }
      }
    }
  }
  return rectList;
}

/** Helper function to deal with nested/recursive rectangles
*@pre The current group is not null
*@post the nested paths are in the rectangle list
*@return void
*@param paths - the current list of all aths in the img
*@param cur_group - the current rectangle to check
**/
void getNestedRects(List* rects, Group* cur_group) {
  int numRects = getLength(cur_group->rectangles);
  if (numRects > 0) {
    ListIterator iter = createIterator(cur_group->rectangles); 
    Node* tmp;
    for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter)) {
      insertBack(rects, ((Rectangle*)tmp));
    }
  }
  numRects = getLength(cur_group->groups);
  if (numRects > 0) {
    ListIterator groupIter = createIterator(cur_group->groups);
    Node* groupTmp;
    for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
      getNestedRects(rects, ((Group*)groupTmp));
    }
  }
}

/** Function creates a list of pointers to all the circles in the img
*@pre SVGimgage exists, is not null, and has not been freed
*@post SVGimgage has not been modified in any way
*@return a newly allocated List of components.  While the List struct itself is new, the components in it are just pointers
to the ones in the image.
*@param obj - a pointer to an SVG struct
*/
List* getCircles(SVGimage* img) {
  List* circList = initializeList(&circleToString, &deleteCircle, &compareCircles);
  
  if (img) {
    int numCirc = getLength(img->circles);
    int lenCirc;
    int lenGroup;
    Group* gg;

    if (numCirc > 0) {
      ListIterator iter = createIterator(img->circles); 
      Node* tmp;

      for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter)) {
        insertBack(circList, ((Circle*)tmp));
      }
    }
    numCirc = getLength(img->groups);

    if (numCirc > 0) {
      ListIterator groupIter = createIterator(img->groups);
      Node* groupTmp;
      for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
        gg = ((Group*)groupTmp);
        lenGroup = getLength(gg->groups);
        lenCirc = getLength(gg->circles);
        
        if ((lenGroup > 0) || (lenCirc > 0)) {
          getNestedCircles(circList, gg);
        }
      }
    }
  }
  return circList; 
}

/** Helper function to deal with nested/recursive circles
*@pre The current group is not null
*@post the nested paths are in the circles list
*@return void
*@param paths - the current list of all circles in the img
*@param cur_group - the current group to check
**/
void getNestedCircles(List* circs, Group* cur_group) {
  int numCircs = getLength(cur_group->circles);
  if (numCircs > 0) {
    ListIterator iter = createIterator(cur_group->circles); 
    Node* tmp;
    for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter)) {
      insertBack(circs, ((Circle*)tmp));
    }
  }
  numCircs = getLength(cur_group->groups);
  if (numCircs > 0) {
    ListIterator groupIter = createIterator(cur_group->groups);
    Node* groupTmp;
    for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
      getNestedCircles(circs, ((Group*)groupTmp));
    }
  }
}

/** Function creates a list of pointers to all the groups in the img
*@pre SVGimgage exists, is not null, and has not been freed
*@post SVGimgage has not been modified in any way
*@return a newly allocated List of components.  While the List struct itself is new, the components in it are just pointers
to the ones in the image.
*@param obj - a pointer to an SVG struct
*/
List* getGroups(SVGimage* img) {
  List* groupList = initializeList(&groupToString, &deleteGroup, &compareGroups);
  
  if (img) {
    int numGroups = getLength(img->groups);
    int lenGroup;
    Group* gg;

    if (numGroups > 0) {
      ListIterator groupIter = createIterator(img->groups);
      Node* groupTmp;
      for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
        gg = ((Group*)groupTmp);
        insertBack(groupList, gg);
        lenGroup = getLength(gg->groups);
        
        if (lenGroup > 0) {
          getNestedGroups(groupList, gg);
        }
      }
    }
  }
  return groupList; 
}

/** Helper function to deal with nested/recursive groups
*@pre The current group is not null
*@post the nested groups are in the groups list
*@return void
*@param paths - the current list of all groups in the img
*@param cur_group - the current group to check
**/
void getNestedGroups(List* grps, Group* cur_group) {
  int numGroups = getLength(cur_group->groups);
  if (numGroups > 0) {
    ListIterator iter = createIterator(cur_group->groups); 
    Node* tmp;
    for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter))  {
      insertBack(grps, ((Group*)tmp));
    }
  }
  numGroups = getLength(cur_group->groups);
  if (numGroups > 0) {
    ListIterator groupIter = createIterator(cur_group->groups);
    Node* groupTmp;
    for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter))  {
      getNestedGroups(grps, ((Group*)groupTmp));
    }
  }
}

/** Function creates a list of pointers to all the paths in the img
*@pre SVGimgage exists, is not null, and has not been freed
*@post SVGimgage has not been modified in any way
*@return a newly allocated List of components.  While the List struct itself is new, the components in it are just pointers
to the ones in the image.
*@param obj - a pointer to an SVG struct
*/
List* getPaths(SVGimage* img) {
  List* pathList = initializeList(&pathToString, &deletePath, &comparePaths);
  
  if (img) {
    int numPath = getLength(img->paths);
    int lenPath;
    int lenGroup;
    Group* gg;

    if (numPath > 0) {
      ListIterator iter = createIterator(img->paths); 
      Node* tmp;

      for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter))  {
        insertBack(pathList, ((Path*)tmp));
      }
    }
    numPath = getLength(img->groups);

    if (numPath > 0) {
      ListIterator groupIter = createIterator(img->groups);
      Node* groupTmp;
      for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
        gg = ((Group*)groupTmp);
        lenGroup = getLength(gg->groups);
        lenPath = getLength(gg->paths);
        
        if ((lenGroup > 0) || (lenPath > 0)) {
          getNestedPaths(pathList, gg);
        }
      }
    }
  }
  return pathList; 
}

/** Helper function to deal with nested/recursive paths
*@pre The current group is not null
*@post the nested paths are in the paths list
*@return void
*@param paths - the current list of all paths in the img
*@param cur_group - the current group to check
**/
void getNestedPaths(List* paths, Group* cur_group) {
  int numPaths = getLength(cur_group->paths);
  
  if (numPaths > 0) {
    ListIterator iter = createIterator(cur_group->paths); 
    Node* tmp;
    for (tmp = nextElement(&iter); tmp != NULL; tmp = nextElement(&iter)) {
      insertBack(paths, ((Path*)tmp));
    }
  }
  numPaths = getLength(cur_group->groups);
  if (numPaths > 0) {
    ListIterator groupIter = createIterator(cur_group->groups);
    Node* groupTmp;
    for (groupTmp = nextElement(&groupIter); groupTmp != NULL; groupTmp = nextElement(&groupIter)) {
      getNestedPaths(paths, ((Group*)groupTmp));
    }
  }
}

/** Function that returns the number of all rectangles with the specified area
 *@pre SVGimgage exists, is not null, and has not been freed.  The search criterion is valid
 *@post SVGimgage has not been modified in any way
 *@return an int indicating how many objects matching the criterion are contained in the image
 *@param obj - a pointer to an SVG struct
 *@param 2nd - the second param depends on the function.  See details below
 */
int numRectsWithArea(SVGimage* img, float area) {
  int count = 0;
  area = ceil(area);
  
  if (img) {
    Rectangle* cur_rect;
    Node* cur;
    List* rects = getRects(img);
    float cur_area;

    ListIterator rectList = createIterator(rects);

    for (cur = nextElement(&rectList); cur != NULL; cur = nextElement(&rectList)) {
      cur_rect = (Rectangle*)cur;
      cur_area = cur_rect->height * cur_rect->width;
      cur_area = ceil(cur_area);

      if (cur_area == area) {
        count++;
      }
      cur_area = 0;
    }
    freeListNotNodes(rects);
  }

  return count;
}

/** Function that returns the number of all circles with the specified area
 *@pre SVGimgage exists, is not null, and has not been freed.  The search criterion is valid
 *@post SVGimgage has not been modified in any way
 *@return an int indicating how many objects matching the criterion are contained in the image
 *@param obj - a pointer to an SVG struct
 *@param 2nd - the second param depends on the function.  See details below
 */
int numCirclesWithArea(SVGimage* img, float area) {
  int count = 0;
  area = ceil(area);

  if (img) {
    Circle* cur_circ;
    Node* cur;
    List* circs = getCircles(img);
    float cur_area;
    float pi = 3.14159;

    ListIterator circList = createIterator(circs);

    for (cur = nextElement(&circList); cur != NULL; cur = nextElement(&circList)) {
      cur_circ = (Circle*)cur;
      cur_area = (cur_circ->r * cur_circ->r) * pi;
      cur_area = ceil(cur_area);

      if (cur_area == area) {
        count++;
      }
      cur_area = 0;
    }
    freeListNotNodes(circs);
  }
  
  return count;
}

/** Function that returns the number of all paths with the specified data - i.e. Path.data field
 *@pre SVGimgage exists, is not null, and has not been freed.  The search criterion is valid
 *@post SVGimgage has not been modified in any way
 *@return an int indicating how many objects matching the criterion are contained in the image
 *@param obj - a pointer to an SVG struct
 *@param 2nd - the second param depends on the function.  See details below
 */
int numPathsWithdata(SVGimage* img, char* data) {
  int count = 0;
  
  if (img) {
    Path* cur_path;
    Node* cur;
    List* paths = getPaths(img);
    ListIterator pathList = createIterator(paths);

    for (cur = nextElement(&pathList); cur != NULL; cur = nextElement(&pathList)) {
      cur_path = (Path*)cur;

      if (strcmp(cur_path->data, data) == 0) {
        count++;
      }
    }
    freeListNotNodes(paths);
  }
  return count;
}

 /** Function that returns the number of all groups with the specified length
 *@pre SVGimgage exists, is not null, and has not been freed.  The search criterion is valid
 *@post SVGimgage has not been modified in any way
 *@return an int indicating how many objects matching the criterion are contained in the image
 *@param obj - a pointer to an SVG struct
 *@param 2nd - the second param depends on the function.  See details below
 */ 
int numGroupsWithLen(SVGimage* img, int len) {
  int count = 0;
  
  if (img) {
    List* groups = getGroups(img);
    Node* cur;
    Group* cur_group;
    int numElements = 0;

    ListIterator groupList = createIterator(groups);

    for (cur = nextElement(&groupList); cur != NULL; cur = nextElement(&groupList)) {
      cur_group = (Group*)cur;

      numElements = getLength(cur_group->rectangles) + getLength(cur_group->circles) + getLength(cur_group->paths) + getLength(cur_group->groups);
      if (numElements == len) {
        count++;
      }
    }
    freeListNotNodes(groups);
  }

  return count;
}

/*  Function that returns the total number of Attribute structs in the SVGimage - i.e. the number of Attributes
contained in all otherAttributes lists in the structs making up the SVGimage
*@pre SVGimgage  exists, is not null, and has not been freed.  
*@post SVGimage has not been modified in any way
*@return the total length of all attribute structs in the SVGimage
*@param obj - a pointer to an SVG struct
*/
int numAttr(SVGimage* img) {
  int count = 0;
  
  if (img) {
    List* list = img->otherAttributes;
    Rectangle* tmpRec;
    Circle* tmpCirc;
    Path* tmpPath;
    Group* tmpGroup;
    Node* cur;
    int svgAttr = getLength(list);
    int groupAttr = 0;
    int pathAttr = 0;
    int circleAttr = 0;
    int rectAttr = 0;

    list = getRects(img);
    ListIterator iter = createIterator(list);
    for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) { 
      tmpRec = ((Rectangle*)cur);
      rectAttr = rectAttr + getLength(tmpRec->otherAttributes);
    }
    freeListNotNodes(list);

    list = getCircles(img);
    iter = createIterator(list);
    for(cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
      tmpCirc = ((Circle*)cur);
      circleAttr = circleAttr + getLength(tmpCirc->otherAttributes);
    } 
    freeListNotNodes(list);

    list = getPaths(img);
    iter = createIterator(list);
    for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
      tmpPath = ((Path*)cur);
      pathAttr = pathAttr + getLength(tmpPath->otherAttributes);
    }
    freeListNotNodes(list);

    list = getGroups(img);
    iter = createIterator(list);
    for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
      tmpGroup = ((Group*)cur);
      groupAttr = groupAttr + getLength(tmpGroup->otherAttributes);
    }
    freeListNotNodes(list);

    count = svgAttr + groupAttr + pathAttr + circleAttr + rectAttr;
  }

  return count;
}

/** Helper function frees the allocated list but not the actual data
*@pre List is not null
*@post List is free but data remains
*@param list - list to be freed
*@return void 
**/
void freeListNotNodes(List* list) {
  if (list == NULL){
		return;
	}
	
	if (list->head == NULL) {
    if (list->tail == NULL) {
		  return;
    }
	}
	
	Node* tmp;
	
	while (list->head != NULL){
		tmp = list->head;
		list->head = list->head->next;
		free(tmp);
	}
	
	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
  free(list);
}

/** Helper function to free attribute data.
*@pre data cannot be uninitialized, else will abort, but it can be empty
*@post this attribute's memory is freed
*@param data - the data to be freed
**/
void deleteAttribute( void* data) {
  Attribute* tmp = (Attribute*)data;

  if (data) {
    free(tmp->name);
    free(tmp->value);
    free(data);
  }
}

/** This is a helper function used for debugging the attribute struct
*@post will return a char pointer to the string
*@param data - casted to the attribute struct, use to get the information
**/
char* attributeToString( void* data) {
  char* string = malloc(strlen(((Attribute *)data)->name) + 1 + strlen(((Attribute *)data)->value) + 1);

  strcpy(string, ((Attribute *)data)->name);
  strcat(string, " ");
  strcat(string, ((Attribute *)data)->value);
  strcat(string, "\0");

  return string;
}

/** This function calculates the comparison of two attributes.
*@post Will always return 0 as we wish to preserve order in this assignment.
*@param first - first attribute value to compare
*@param second - second attribute value to compare
**/
int compareAttributes(const void *first, const void *second) {
  return 0;
}

/** Helper function to free group data.
*@pre data cannot be uninitialized, else will abort, but it can be empty
*@post this group's memory is freed
*@param data - the data to be freed
**/
void deleteGroup(void* data) {
  Group* tmp = (Group*)data;

  if (data) {
    freeList(tmp->rectangles);
    freeList(tmp->circles);
    freeList(tmp->paths);
    freeList(tmp->groups);
    freeList(tmp->otherAttributes);
    free(data);
  }
}

/** This is a helper function used for debugging the group struct
*@post will return a char pointer to the string
*@param data - casted to the group struct, use to get the information
**/
char* groupToString( void* data) {
  if (data) {
    Group* tmp = (Group*)data;
    char buffer[75];
    sprintf(buffer, "Contains: %d rects, %d circs, %d paths, %d groups\n", getLength(tmp->rectangles), getLength(tmp->circles), getLength(tmp->paths), getLength(tmp->groups));
    char* string = malloc(strlen(buffer) + 1);
    strcpy(string, buffer);
    return string;
  } else {
    return "Group is NULL";
  }
}

/** This function calculates the comparison of two groups.
*@post Will always return 0 as we wish to preserve order in this assignment.
*@param first - first group value to compare
*@param second - second group value to compare
**/
int compareGroups(const void *first, const void *second) {
  return 0;
}

/** Helper function to free rectangle data.
*@pre data cannot be uninitialized, else will abort, but it can be empty
*@post this rectangle's memory is freed
*@param data - the data to be freed
**/
void deleteRectangle(void* data) {
  Rectangle* tmp = (Rectangle*)data;
  if (data) {
    freeList(tmp->otherAttributes);
    free(data);
  }
}

/** This is a helper function used for debugging the rectangle struct
*@post will return a char pointer to the string
*@param data - casted to the rectangle struct, use to get the information
**/
char* rectangleToString(void* data) {
  if (data) {
    Rectangle* tmp = (Rectangle*)data;
    char buffer[50];
    sprintf(buffer, "Width: %0.2f\nHeight: %0.2f\n", tmp->width, tmp->height);
    char* string = malloc(strlen(buffer) + 1);
    strcpy(string, buffer);
    return string;
  } else {
    char* string = malloc(strlen("Rectangle is NULL") + 1);
    strcpy(string, "Rectangle is NULL");
    return string;
  }
}

/** This function calculates the comparison of two rectangles.
*@post Will always return 0 as we wish to preserve order in this assignment.
*@param first - first rectangle value to compare
*@param second - second path rectangle to compare
**/
int compareRectangles(const void *first, const void *second) {
  return 0;
}

/** Helper function to free circle data.
*@pre data cannot be uninitialized, else will abort, but it can be empty
*@post this circle's memory is freed
*@param data - the data to be freed
**/
void deleteCircle(void* data) {
  Circle* tmp = (Circle*)data;
  if (data) {
    freeList(tmp->otherAttributes);
    free(data);
  }
}

/** This is a helper function used for debugging the circle struct
*@post will return a char pointer to the string
*@param data - casted to the circle struct, use to get the information
**/
char* circleToString(void* data) {
  if (data) {
    Circle* tmp = (Circle*)data;
    char buffer[50];
    sprintf(buffer, "CX: %0.2f\nCY: %0.2f\nR: %0.2f\n", tmp->cx, tmp->cy, tmp->r);
    char* string = malloc(strlen(buffer) + 1);
    strcpy(string, buffer);
    return string;
  } else {
    char* string = malloc(strlen("Circle is NULL") + 1);
    strcpy(string, "Circle is NULL");
    return string;
  }
}

/** This function calculates the comparison of two circles.
*@post Will always return 0 as we wish to preserve order in this assignment.
*@param first - first circle value to compare
*@param second - second circle value to compare
**/
int compareCircles(const void *first, const void *second) {
  return 0;
}

/** Helper function to free path data.
*@pre data cannot be uninitialized, else will abort, but it can be empty
*@post this path's memory is freed
*@param data - the data to be freed
**/
void deletePath(void* data) {
  Path* tmp = (Path*)data;
  if (data) {
    free(tmp->data);
    freeList(tmp->otherAttributes);
    free(data);
  }
}

/** This is a helper function used for debugging the path struct
*@post will return a char pointer to the string
*@param data - casted to the path struct, use to get the information
**/
char* pathToString(void* data) {
  if (data) {
    Path* tmp = (Path *)data;
    char* string = malloc(strlen(tmp->data) + 1);
    strcpy(string, tmp->data);
    return string;
  } else {
    char* string = malloc(strlen("Path is NULL") + 1);
    strcpy(string, "Path is NULL");
    return string;
  }
}

/** This function calculates the comparison of two paths.
*@post Will always return 0 as we wish to preserve order in this assignment.
*@param first - first path value to compare
*@param second - second path value to compare
**/
int comparePaths(const void *first, const void *second) {
  return 0;
}

/** Function to validating an existing a SVGimage object against a SVG schema file
 *@pre SVGimage object exists and is not NULL, schema file name is not NULL/empty, and represents a valid schema file
 *@post SVGimage has not been modified in any way
 *@return the boolean aud indicating whether the SVGimage is valid
 *@param image - a pointer to a SVGimage struct
 *@param schemaFile - the name of a schema file
 **/
bool validateSVGimage(SVGimage* image, char* schemaFile) {
  bool test = false;
  if (image == NULL) {
    return false;
  } else {
    test = checkRectangles(image);
    if (!test) {
      return false;
    }
    test = checkCircles(image);
    if (!test) {
      return false;
    }
    test = checkPathAttr(image);
    if (!test) {
      return false;
    }
    test = checkPathAttr(image);
    if (!test) {
      return false;
    }
    if (image->otherAttributes == NULL) {
      return false;
    }
    Node* curAttr; 
    ListIterator iterAttr = createIterator(image->otherAttributes);
    for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
      Attribute* a = (Attribute*)curAttr;
      if (a->name == NULL) {
        return false;
      }
      if (a->value == NULL) {
        return false;
      }
    }

    xmlDoc *doc = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    xmlSchemaPtr schema = NULL;
    int ret;

    //Checks if the schema file is empty
    if (schemaFile == NULL) {
      deleteSVGimage(image);
      return NULL;
    }
    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(schemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    doc = createTree(image);

    if (doc == NULL) {
      if (schema != NULL) {
        xmlSchemaFree(schema);
      }
      xmlSchemaCleanupTypes();
      deleteSVGimage(image);
      xmlCleanupParser();
      return NULL;
    } else {
      ret = validateDoc(doc, schemaFile, schema);
    }

    /*Clean up the parser*/
    xmlFreeDoc(doc);
    xmlCleanupParser();

    if (ret == 0) {
      return true;
    } else {
      deleteSVGimage(image);
      return false;
    }
  } 
}

bool checkRectangles(SVGimage* img) {
  List* rects = getRects(img);
  Node* cur; //Current node the loop is on
  ListIterator iter = createIterator(rects);
  if (getLength(rects) == 0) {
    free(rects);
    return true;
  }

  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Rectangle* r = ((Rectangle*)cur);
    if (r->x < 0) {
      freeListNotNodes(rects);
      return false;
    }
    if (r->y < 0) {
      freeListNotNodes(rects);
      return false;
    }
    if (r->width < 0) {
      freeListNotNodes(rects);
      return false;
    }
    if (r->height < 0) {
      freeListNotNodes(rects);
      return false;
    }
    if (r->otherAttributes == NULL) {
      freeListNotNodes(rects);
      return false;
    }
    Node* curAttr; 
    ListIterator iterAttr = createIterator(r->otherAttributes);
    for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
      Attribute* a = (Attribute*)curAttr;
      if (a->name == NULL) {
        freeListNotNodes(rects);
        return false;
      }
      if (a->value == NULL) {
        freeListNotNodes(rects);
        return false;
      }
    }
  }  
  freeListNotNodes(rects);
  return true;
}

bool checkCircles(SVGimage* img) {
  List* circles = getCircles(img);
  Node* cur; //Current node the loop is on
  ListIterator iter = createIterator(circles);
  if (getLength(circles) == 0) {
    free(circles);
    return true;
  }

  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Circle* c = ((Circle*)cur);
    if (c->cx < 0) {
      freeListNotNodes(circles);
      return false;
    }
    if (c->cy < 0) {
      freeListNotNodes(circles);
      return false;
    }
    if (c->r < 0) {
      freeListNotNodes(circles);
      return false;
    }
    if (c->otherAttributes == NULL) {
      freeListNotNodes(circles);
      return false;
    }
    Node* curAttr; 
    ListIterator iterAttr = createIterator(c->otherAttributes);
    for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
      Attribute* a = (Attribute*)curAttr;
      if (a->name == NULL) {
        freeListNotNodes(circles);
        return false;
      }
      if (a->value == NULL) {
        freeListNotNodes(circles);
        return false;
      }
    }
  }  
  freeListNotNodes(circles);
  return true;
}

bool checkPathAttr(SVGimage* img) {
  List* paths = getPaths(img);
  Node* cur; //Current node the loop is on
  ListIterator iter = createIterator(paths);
  if (getLength(paths) == 0) {
    free(paths);
    return true;
  }

  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Path* p = ((Path*)cur);
    if (p->otherAttributes == NULL) {
      freeListNotNodes(paths);
      return false;
    }
    Node* curAttr; 
    ListIterator iterAttr = createIterator(p->otherAttributes);
    for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
      Attribute* a = (Attribute*)curAttr;
      if (a->name == NULL) {
        freeListNotNodes(paths);
        return false;
      }
      if (a->value == NULL) {
        freeListNotNodes(paths);
        return false;
      }
    }
  }  
  freeListNotNodes(paths);
  return true;
}

bool checkGroupAttr(SVGimage* img) {
  List* groups = getGroups(img);
  Node* cur; //Current node the loop is on
  ListIterator iter = createIterator(groups);
  if (getLength(groups) == 0) {
    free(groups);
    return true;
  }

  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Group* g = ((Group*)cur);
    if (g->otherAttributes == NULL) {
      freeListNotNodes(groups);
      return false;
    }
    Node* curAttr; 
    ListIterator iterAttr = createIterator(g->otherAttributes);
    for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
      Attribute* a = (Attribute*)curAttr;
      if (a->name == NULL) {
        freeListNotNodes(groups);
        return false;
      }
      if (a->value == NULL) {
        freeListNotNodes(groups);
        return false;
      }
    }
  }  
  freeListNotNodes(groups);
  return true;
}

/** Function to create an SVG object based on the contents of an SVG file.
 * This function must validate the XML tree generated by libxml against a SVG schema file
 * before attempting to traverse the tree and create an SVGimage struct
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
       Schema file name is not NULL/empty, and represents a valid schema file
 *@post Either:
    A valid SVGimage has been created and its address was returned
		or 
		An error occurred, or SVG file was invalid, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the SVG file
 *@param schemaFile - a string containing the name of the XSD file
**/
SVGimage* createValidSVGimage(char* fileName, char* schemaFile) {
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  xmlSchemaParserCtxtPtr ctxt;
  xmlSchemaPtr schema = NULL;
  int ret;
  SVGimage *newImg = malloc(sizeof(SVGimage));

  LIBXML_TEST_VERSION
  initImg(newImg);

  //Checks if it is an svg file
  if (fileName == NULL) {
    deleteSVGimage(newImg);
    return NULL;
  } else if (strstr(fileName, ".svg") == NULL) {
    deleteSVGimage(newImg);
    return NULL;
  }
  //Checks if the schema file is empty
  if (schemaFile == NULL) {
    deleteSVGimage(newImg);
    return NULL;
  }
  xmlLineNumbersDefault(1);

  ctxt = xmlSchemaNewParserCtxt(schemaFile);

  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);

  doc = xmlReadFile(fileName, NULL, 0);

  if (doc == NULL) {
    if (schema != NULL) {
      xmlSchemaFree(schema);
    }
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    deleteSVGimage(newImg);
    return NULL;
  } else {
    ret = validateDoc(doc, schemaFile, schema);
  }

  if (ret == 0) {
		/*Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    get_elements(root_element, newImg);
	} else {
    deleteSVGimage(newImg);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return NULL;
  }

  /*Clean up the parser*/
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return newImg;
}

int validateDoc (xmlDoc* doc, char* schemaFile, xmlSchemaPtr schema) {
  int ret;
  xmlSchemaValidCtxtPtr ctxt;

  ctxt = xmlSchemaNewValidCtxt(schema);
  xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  ret = xmlSchemaValidateDoc(ctxt, doc);
  xmlSchemaFreeValidCtxt(ctxt);

  // free the resource
  if (schema != NULL) {
    xmlSchemaFree(schema);
  }
  xmlSchemaCleanupTypes();

  return ret;
}

/** Function to writing a SVGimage into a file in SVG format.
 *@pre SVGimage object exists, is valid, and and is not NULL. FleName is not NULL, has the correct extension
 *@post SVGimage has not been modified in any way, and a file representing the SVGimage contents in SVG format has been created
 *@return a boolean value indicating success or failure of the write
 *@param doc - a pointer to a SVGimage struct
 *@param fileName - the name of the output file
 **/
bool writeSVGimage(SVGimage* image, char* fileName) {
  if (image == NULL) {
    return false;
  } else if (fileName == NULL) {
    return false;
  } else {
    xmlDoc* doc = createTree(image);
    int check = xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    if (check == -1) {
      return false;
    } else {
      return true;
    }
  }
}

/** Function to convert an SVGimage into an XML tree.
 *@pre SVGimage object exists, is valid, and and is not NULL
 *@post SVGimage has not been modified in any way
 *      XML tree has been created 
 *      or
 *      An error occurred and the tree could not be created
 *@return the xmlDoc struct
 *@param img - a pointer to the SVGimage struct
 **/
xmlDoc* createTree(SVGimage* img) {
  xmlDocPtr doc = NULL; 
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  char buff[256] = "";

  LIBXML_TEST_VERSION;

  Node* cur; //Current node the loop is on

  /* Creates a new document, a node and set it as a root node */
  doc = xmlNewDoc(BAD_CAST "1.0");
  
  root_node = xmlNewNode(NULL, BAD_CAST "svg");
  xmlDocSetRootElement(doc, root_node);
  xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST img->namespace, NULL);
  xmlSetNs(root_node, ns);

  /* Adds all the otherAttributes of the svg */
  addAttr(img->otherAttributes, root_node);

  /* Add title and description */
  if (strlen(img->title) > 0) {
    node = xmlNewChild(root_node, NULL, BAD_CAST "title", BAD_CAST img->title); /*******PROBLEM********/
  }
  if (strlen(img->description) > 0) {
    node = xmlNewChild(root_node, NULL, BAD_CAST "desc", BAD_CAST img->description); /*******PROBLEM********/
  }
  /* Add elements contained inside lists */
  /* Adds all the rectangles */
  ListIterator iterRect = createIterator(img->rectangles);

  for (cur = nextElement(&iterRect); cur != NULL; cur = nextElement(&iterRect)) {
    List* tmpRect = ((Rectangle*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);

    sprintf(buff, "%f", ((Rectangle*)cur)->x);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "x", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->y);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "y", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->width);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "width", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->height);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "height", BAD_CAST buff);
    strcpy(buff, "");

    addAttr(tmpRect, node);
  }
  /* Adds all the circles */
  ListIterator iterCirc = createIterator(img->circles);

  for (cur = nextElement(&iterCirc); cur != NULL; cur = nextElement(&iterCirc)) {
    List* tmpCirc = ((Circle*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);

    sprintf(buff, "%f", ((Circle*)cur)->cx);
    strcat(buff, ((Circle*)cur)->units);
    //printf("%s\n", buff);
    xmlNewProp(node, BAD_CAST "cx", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Circle*)cur)->cy);
    strcat(buff, ((Circle*)cur)->units);
    //printf("%s\n", buff);
    xmlNewProp(node, BAD_CAST "cy", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Circle*)cur)->r);
    strcat(buff, ((Circle*)cur)->units);
    //printf("%s\n", buff);
    xmlNewProp(node, BAD_CAST "r", BAD_CAST buff);
    strcpy(buff, "");

    addAttr(tmpCirc, node);
  }
  /* Adds all the paths */
  ListIterator iterPath = createIterator(img->paths);

  for (cur = nextElement(&iterPath); cur != NULL; cur = nextElement(&iterPath)) {
    List* tmpPath = ((Path*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
    char* tmpBuffer = malloc(sizeof(char) * strlen(((Path*)cur)->data) + 1);

    strcpy(tmpBuffer, ((Path*)cur)->data);
    //printf("%s\n", buff);
    xmlNewProp(node, BAD_CAST "d", BAD_CAST tmpBuffer);
    free(tmpBuffer);

    addAttr(tmpPath, node);
  }
  /* Adds all the groups */
  ListIterator iterGroup = createIterator(img->groups);

  for (cur = nextElement(&iterGroup); cur != NULL; cur = nextElement(&iterGroup)) {
    List* tmpGroup = ((Group*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);
    //printf("%s\n", ((char*)node->name));
    addAttr(tmpGroup, node);
    addLists(((Group*)cur), node);
  }
  
  return doc;
}

/** Helper function that adds all the attributes to the attributes properties of each tree node.
 *@pre list is not NULL
 *@post the tree node is now populated with the attributes
 *@return void
 *@param list - list of other attributes to add
 *@param node - where to add these attributes
 **/
void addAttr (List* list, xmlNodePtr node) {
  Node* cur; //Current node the loop is on
  ListIterator iterAttr = createIterator(list);

  for (cur = nextElement(&iterAttr); cur != NULL; cur = nextElement(&iterAttr)) {
    Attribute* attr = ((Attribute*)cur);
    xmlNewProp(node, BAD_CAST attr->name, BAD_CAST attr->value);
  } 
}

/** Helper function that recursively adds all the group/nested elements to the tree.
 *@pre cur_group is not NULL
 *@post tree is now populated with nested elements
 *@return void
 *@param cur_group - the current group to be searching
 *@param root_node - the parent node to add these elements to
 **/
void addLists(Group* cur_group, xmlNodePtr root_node) {
  Node* cur;
  xmlNodePtr node = NULL;
  char buff[256];

  /* Adds all the rectangles */
  ListIterator iterRect = createIterator(cur_group->rectangles);

  for (cur = nextElement(&iterRect); cur != NULL; cur = nextElement(&iterRect)) {
    List* tmpRect = ((Rectangle*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);

    sprintf(buff, "%f", ((Rectangle*)cur)->x);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "x", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->y);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "y", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->width);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "width", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Rectangle*)cur)->height);
    strcat(buff, ((Rectangle*)cur)->units);
    xmlNewProp(node, BAD_CAST "height", BAD_CAST buff);
    strcpy(buff, "");

    addAttr(tmpRect, node);
  }
  /* Adds all the circles */
  ListIterator iterCirc = createIterator(cur_group->circles);

  for (cur = nextElement(&iterCirc); cur != NULL; cur = nextElement(&iterCirc)) {
    List* tmpCirc = ((Circle*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);

    sprintf(buff, "%f", ((Circle*)cur)->cx);
    strcat(buff, ((Circle*)cur)->units);
    xmlNewProp(node, BAD_CAST "cx", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Circle*)cur)->cy);
    strcat(buff, ((Circle*)cur)->units);
    xmlNewProp(node, BAD_CAST "cy", BAD_CAST buff);
    strcpy(buff, "");

    sprintf(buff, "%f", ((Circle*)cur)->r);
    strcat(buff, ((Circle*)cur)->units);
    xmlNewProp(node, BAD_CAST "r", BAD_CAST buff);
    strcpy(buff, "");

    addAttr(tmpCirc, node);
  }
  /* Adds all the paths */
  ListIterator iterPath = createIterator(cur_group->paths);

  for (cur = nextElement(&iterPath); cur != NULL; cur = nextElement(&iterPath)) {
    List* tmpPath = ((Path*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
    char* tmpBuffer = malloc(sizeof(char) * strlen(((Path*)cur)->data) + 1);

    strcpy(tmpBuffer, ((Path*)cur)->data);
    xmlNewProp(node, BAD_CAST "d", BAD_CAST tmpBuffer);
    free(tmpBuffer);

    addAttr(tmpPath, node);
  }
  /* Adds all the groups */
  ListIterator iterGroup = createIterator(cur_group->groups);

  for (cur = nextElement(&iterGroup); cur != NULL; cur = nextElement(&iterGroup)) {
    List* tmpGroup = ((Group*)cur)->otherAttributes;
    node = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);
    addAttr(tmpGroup, node);
    addLists(((Group*)cur), node);
  }
}

/** Function to setting an attribute in an SVGimage or component
 *@pre
    SVGimage object exists, is valid, and and is not NULL.
    newAttribute is not NULL
 *@post The appropriate attribute was set corectly
 *@return void
 *@param image - a pointer to an SVGimage struct
 *@param elemType - enum value indicating elemtn to modify
 *@param elemIndex - index of thje lement to modify
 *@param newAttribute - struct containing name and value of the updated attribute
 **/
void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
  if (image == NULL) {
    return;
  } else if (newAttribute == NULL) {
    return;
  } else if (newAttribute->name == NULL) {
    return;
  }

  if (elemType == SVG_IMAGE) {
    bool found = setOtherAttr(image->otherAttributes, newAttribute);
    if (!found) {
      insertBack(image->otherAttributes, newAttribute);
    }
  } else if (elemType == CIRC) {
    setCircAttr(image->circles, elemIndex, newAttribute);
  } else if (elemType == RECT) {
    setRectAttr(image->rectangles, elemIndex, newAttribute);
  } else if (elemType == PATH) {
    setPathAttr(image->paths, elemIndex, newAttribute);
  } else if (elemType == GROUP) {
    setGroupAttr(image->groups, elemIndex, newAttribute);
  }
}

/** Helper function that iterates through otherAttr list and checks if there is a attribute name that matches the parameter attribute name
 *@pre list is not NULL and attribute is not NULL
 *@post if name found then list attribute value is changed and attribute is freed
 *@return bool
 *@param list - list of otherAttributes for any component
 *@param attribute - the attribute to be changed or added
 **/
bool setOtherAttr(List* list, Attribute* attribute) {
  Node* curAttr;
  ListIterator iterAttr = createIterator(list);
  bool found = false;
  for (curAttr = nextElement(&iterAttr); curAttr != NULL; curAttr = nextElement(&iterAttr)) {
    Attribute* tmpAttr = ((Attribute*)curAttr);
    if (strcmp(tmpAttr->name, attribute->name) == 0) {
      found = true;
      free(tmpAttr->value);
      tmpAttr->value = malloc(sizeof(char) * strlen(attribute->value) + 1);
      strcpy(tmpAttr->value, attribute->value);
      deleteAttribute(attribute);
      break;
    }
  }
  return found;
}

/** Helper function that searches through the list of circles to see if it contains a specific attribute name, if it doesn't add it
 *@pre list and attribute are not NULL
 *@post list has another attribute or a changed attribute
 *@return void
 *@param list - list of circles in the image
 *@param index - which circle that should be checked
 *@param attribute - data to add
 **/
void setCircAttr(List* list, int index, Attribute* attribute) {
  Node* cur;
  ListIterator iterCirc = createIterator(list);
  int count = 0;
  char* buff;

  for (cur = nextElement(&iterCirc); cur != NULL; cur = nextElement(&iterCirc)) {
    Circle* cur_circ = (Circle*)cur;
    if (count == index) {
      if (strcmp(attribute->name, "cx") == 0) {
        cur_circ->cx = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "cy") == 0) {
        cur_circ->cy = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "r") == 0) { 
        cur_circ->r = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "units") == 0) {
        strcpy(cur_circ->units, "");
        strcpy(cur_circ->units, attribute->value);
        deleteAttribute(attribute);
        return;
      } else {
        List* tmpCirc = ((Circle*)cur)->otherAttributes;
        bool found = setOtherAttr(tmpCirc, attribute);
        if (!found) {
          insertBack(cur_circ->otherAttributes, attribute);
          return;
        } else {
          return;
        }
      }
    } 
    count++;
  }
  deleteAttribute(attribute); //Index does not exist in list, therefore free the memory
}

/** Helper function that searches through the list of rectangles to see if it contains a specific attribute name, if it doesn't add it
 *@pre list and attribute are not NULL
 *@post list has another attribute or a changed attribute
 *@return void
 *@param list - list of rectangles in the image
 *@param index - which rectangle that should be checked
 *@param attribute - data to add
 **/
void setRectAttr(List* list, int index, Attribute* attribute) {
  Node* cur;
  ListIterator iterRect = createIterator(list);
  int count = 0;
  char* buff;

  for (cur = nextElement(&iterRect); cur != NULL; cur = nextElement(&iterRect)) {
    Rectangle* cur_rect = (Rectangle*)cur;
    if (count == index) {
      if (strcmp(attribute->name, "x") == 0) {
        cur_rect->x = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "y") == 0) {
        cur_rect->y = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "width") == 0) {
        cur_rect->width = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "height") == 0) {
        cur_rect->height = strtof(attribute->value, &buff);
        deleteAttribute(attribute);
        return;
      } else if (strcmp(attribute->name, "units") == 0) {
        strcpy(cur_rect->units, "");
        strcpy(cur_rect->units, attribute->value);
        deleteAttribute(attribute);
        return;
      } else {
        List* tmpRect = ((Rectangle*)cur)->otherAttributes;
        bool found = setOtherAttr(tmpRect, attribute);
        if (!found) {
          insertBack(cur_rect->otherAttributes, attribute);
          return;
        } else {
          return;
        }
      }
    }
    count++;
  }
  deleteAttribute(attribute); //Index does not exist in list, therefore free the memory
}

/** Helper function that searches through the list of paths to see if it contains a specific attribute name, if it doesn't add it
 *@pre list and attribute are not NULL
 *@post list has another attribute or a changed attribute
 *@return void
 *@param list - list of paths in the image
 *@param index - which path that should be checked
 *@param attribute - data to add
 **/
void setPathAttr(List* list, int index, Attribute* attribute) {
  Node* cur;
  ListIterator iterPath = createIterator(list);
  int count = 0;

  for (cur = nextElement(&iterPath); cur != NULL; cur = nextElement(&iterPath)) {
    Path* cur_path = (Path*)cur;
    if (count == index) {
      if (strcmp(attribute->name, "d") == 0) {
        strcpy(cur_path->data, "");
        strcpy(cur_path->data, attribute->value);
        deleteAttribute(attribute);
        return;
      } else {
        List* tmpPath = ((Path*)cur)->otherAttributes;
        bool found = setOtherAttr(tmpPath, attribute);
        if (!found) {
          insertBack(cur_path->otherAttributes, attribute);
          return;
        } else {
          return;
        }
      }
    }
    count++;
  }
  deleteAttribute(attribute); //Index does not exist in list, therefore free the memory
}

/** Helper function that searches through the list of groups to see if it contains a specific attribute name, if it doesn't add it
 *@pre list and attribute are not NULL
 *@post list has another attribute or a changed attribute
 *@return void
 *@param list - list of groups in the image
 *@param index - which group that should be checked
 *@param attribute - data to add
 **/
void setGroupAttr(List* list, int index, Attribute* attribute) {
  Node* cur;
  ListIterator iterGroup = createIterator(list);
  int count = 0;
  for (cur = nextElement(&iterGroup); cur != NULL; cur = nextElement(&iterGroup)) {
    Group* cur_group = (Group*)cur;
    if (count == index) {
      List* tmpGroup = ((Group*)cur)->otherAttributes;
      bool found = setOtherAttr(tmpGroup, attribute);
      if (!found) {
        insertBack(cur_group->otherAttributes, attribute);
        return;
      } else {
          return;
      }
    }
    count++;
  }
  deleteAttribute(attribute); //Index does not exist in list, therefore free the memory
}

/** Function to adding an element - Circle, Rectngle, or Path - to an SVGimage
 *@pre
    SVGimage object exists, is valid, and and is not NULL.
    newElement is not NULL
 *@post The appropriate element was added correctly
 *@return N/A
 *@param
    image - a pointer to an SVGimage struct
    elemType - enum value indicating elemtn to modify
    newElement - pointer to the element sgtruct (Circle, Rectngle, or Path)
 **/
void addComponent(SVGimage* image, elementType type, void* newElement) {
  if (image == NULL) {
    return;
  } else if (newElement == NULL) {
    return;
  }

  if (type == CIRC) {
    insertBack(image->circles, (Circle*)newElement);
  } else if (type == RECT) {
    insertBack(image->rectangles, (Rectangle*)newElement);
  } else if (type == PATH) {
    insertBack(image->paths, (Path*)newElement);
  } else if (type == GROUP) {
    insertBack(image->groups, (Group*)newElement);
  }
}

/** Function to converting an Attribute into a JSON string
*@pre Attribute is not NULL
*@post Attribute has not been modified in any way
*@return A string in JSON format
*@param a - a pointer to an Attribute struct
**/
char* attrToJSON(const Attribute *a) {
  char* string;
  if (a != NULL) {
    string = malloc(sizeof(char) * (strlen(a->name) + strlen(a->value)) + 25);
    strcpy(string, "{\"name\":\"");
    strcat(string, a->name);
    strcat(string, "\",\"value\":\"");
    strcat(string, a->value);
    strcat(string, "\"}");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a Circle into a JSON string
*@pre Circle is not NULL
*@post Circle has not been modified in any way
*@return A string in JSON format
*@param c - a pointer to a Circle struct
**/
char* circleToJSON(const Circle *c) {
  char* string;
  if (c != NULL) {
    char tmpCX[25] = "";
    char tmpCY[25] = "";
    char tmpR[25] = "";
    char tmpU[50] = "";
    snprintf(tmpCX, 25, "%.2f", c->cx); 
    snprintf(tmpCY, 25, "%.2f", c->cy); 
    snprintf(tmpR, 25, "%.2f", c->r); 
    snprintf(tmpU, 50, "%s", c->units); 
    char numAttr[5] = "";
    snprintf(numAttr, 5, "%d", getLength(c->otherAttributes));
    
    string = malloc(sizeof(char) * (strlen(tmpCX) + strlen(tmpCY) + strlen(tmpR) + strlen(tmpU) + strlen(numAttr)) + 50);
    strcpy(string, "{\"cx\":");
    strcat(string, tmpCX);
    strcat(string, ",\"cy\":");
    strcat(string, tmpCY);
    strcat(string, ",\"r\":");
    strcat(string, tmpR);
    strcat(string, ",\"numAttr\":");
    strcat(string, numAttr);
    strcat(string, ",\"units\":\"");
    strcat(string, tmpU);
    strcat(string, "\"}");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a Rectangle into a JSON string
*@pre Rectangle is not NULL
*@post Rectangle has not been modified in any way
*@return A string in JSON format
*@param event - a pointer to a Rectangle struct
**/
char* rectToJSON(const Rectangle *r) {
  char* string;
  if (r != NULL) {
    char tmpX[25] = "";
    char tmpY[25] = "";
    char tmpH[25] = "";
    char tmpW[25] = "";
    char tmpU[50] = "";
    snprintf(tmpX, 25, "%.2f", r->x); 
    snprintf(tmpY, 25, "%.2f", r->y); 
    snprintf(tmpH, 25, "%.2f", r->height); 
    snprintf(tmpW, 25, "%.2f", r->width); 
    snprintf(tmpU, 50, "%s", r->units); 
    char numAttr[5] = "";
    snprintf(numAttr, 5, "%d", getLength(r->otherAttributes));
    
    string = malloc(sizeof(char) * (strlen(tmpX) + strlen(tmpY) + strlen(tmpH) + strlen(tmpW) + strlen(tmpU) + strlen(numAttr)) + 50);
    strcpy(string, "{\"x\":");
    strcat(string, tmpX);
    strcat(string, ",\"y\":");
    strcat(string, tmpY);
    strcat(string, ",\"w\":");
    strcat(string, tmpW);
    strcat(string, ",\"h\":");
    strcat(string, tmpH);
    strcat(string, ",\"numAttr\":");
    strcat(string, numAttr);
    strcat(string, ",\"units\":\"");
    strcat(string, tmpU);
    strcat(string, "\"}");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a Path into a JSON string
*@pre Path is not NULL
*@post Path has not been modified in any way
*@return A string in JSON format
*@param event - a pointer to a Path struct
**/
char* pathToJSON(const Path *p) {
  char* string;
  if (p != NULL) {
    char tmp[65] = "";
    snprintf(tmp, 65, "%s", p->data);
    char numAttr[5] = "";
    snprintf(numAttr, 5, "%d", getLength(p->otherAttributes));

    string = malloc(sizeof(char) * 65 + strlen(numAttr) + 25);
    strcpy(string, "{\"d\":\"");
    strcat(string, tmp);
    strcat(string, "\",\"numAttr\":");
    strcat(string, numAttr);
    strcat(string, "}");
    
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a Group into a JSON string
*@pre Group is not NULL
*@post Group has not been modified in any way
*@return A string in JSON format
*@param g - a pointer to a Group struct
**/
char* groupToJSON(const Group *g) {
  char* string;
  if (g != NULL) {
    char strElements[5] = "";
    int numElements = getLength(g->rectangles) + getLength(g->circles) + getLength(g->paths) + getLength(g->groups);
    snprintf(strElements, 5, "%d", numElements);
    char numAttr[5] = "";
    snprintf(numAttr, 5, "%d", getLength(g->otherAttributes));

    string = malloc(sizeof(char) * strlen(strElements) + strlen(numAttr) + 25);
    strcpy(string, "{\"children\":");
    strcat(string, strElements);
    strcat(string, ",\"numAttr\":");
    strcat(string, numAttr);
    strcat(string, "}");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a list of Attribute structs into a JSON string
*@pre Attribute list is not NULL
*@post Attribute list has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to a List struct
**/
char* attrListToJSON(const List *list) {
  char* string;
  if (list != NULL) {
    List* attrs = (List*)list;
    int cur_size = 3;
    string = malloc(sizeof(char) * cur_size);
    strcpy(string, "[");
    Node* cur;
    ListIterator iterAttr = createIterator(attrs);
    int count = 1;
    for (cur = nextElement(&iterAttr); cur != NULL; cur = nextElement(&iterAttr)) {
      Attribute* cur_attr = (Attribute*)cur;
      char* tmpString = attrToJSON(cur_attr);
      cur_size += strlen(tmpString) + 2;
      string = realloc(string, sizeof(char) * cur_size);
      strcat(string, tmpString);
      if (count < getLength(attrs)) {
        strcat(string, ",");
      }
      count++;
      free(tmpString);
    }
    strcat(string, "]");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "[]");
  }
  return string;
}

/** Function to converting a list of Circle structs into a JSON string
*@pre Circle list is not NULL
*@post Circle list has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to a List struct
**/
char* circListToJSON(const List *list) {
  char* string;
  if (list != NULL) {
    List* circs = (List*)list;
    int cur_size = 3;
    string = malloc(sizeof(char) * cur_size);
    strcpy(string, "[");
    Node* cur;
    ListIterator iterCirc = createIterator(circs);
    int count = 1;
    for (cur = nextElement(&iterCirc); cur != NULL; cur = nextElement(&iterCirc)) {
      Circle* cur_circ = (Circle*)cur;
      char* tmpString = circleToJSON(cur_circ);
      cur_size += strlen(tmpString) + 2;
      string = realloc(string, sizeof(char) * cur_size);
      strcat(string, tmpString);
      if (count < getLength(circs)) {
        strcat(string, ",");
      }
      count++;
      free(tmpString);
    }
    strcat(string, "]");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "[]");
  }
  return string;
}

/** Function to converting a list of Rectangle structs into a JSON string
*@pre Rectangle list is not NULL
*@post Rectangle list has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to a List struct
**/
char* rectListToJSON(const List *list) {
  char* string;
  if (list != NULL) {
    List* rects = (List*)list;
    int cur_size = 3;
    string = malloc(sizeof(char) * cur_size);
    strcpy(string, "[");
    Node* cur;
    ListIterator iterRect = createIterator(rects);
    int count = 1;
    for (cur = nextElement(&iterRect); cur != NULL; cur = nextElement(&iterRect)) {
      Rectangle* cur_rect = (Rectangle*)cur;
      char* tmpString = rectToJSON(cur_rect);
      cur_size += strlen(tmpString) + 2;
      string = realloc(string, sizeof(char) * cur_size);
      strcat(string, tmpString);
      if (count < getLength(rects)) {
        strcat(string, ",");
      }
      count++;
      
      free(tmpString);
    }
    strcat(string, "]");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "[]");
  }
  return string;
}

/** Function to converting a list of Path structs into a JSON string
*@pre Path list is not NULL
*@post Path list has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to a List struct
**/
char* pathListToJSON(const List *list) {
  char* string;
  if (list != NULL) {
    List* paths = (List*)list;
    int cur_size = 3;
    string = malloc(sizeof(char) * cur_size);
    strcpy(string, "[");
    Node* cur;
    ListIterator iterPath = createIterator(paths);
    int count = 1;
    for (cur = nextElement(&iterPath); cur != NULL; cur = nextElement(&iterPath)) {
      Path* cur_path = (Path*)cur;
      char* tmpString = pathToJSON(cur_path);
      cur_size += strlen(tmpString) + 2;
      string = realloc(string, sizeof(char) * cur_size);
      strcat(string, tmpString);
      if (count < getLength(paths)) {
        strcat(string, ",");
      }
      count++;
      
      free(tmpString);
    }
    strcat(string, "]");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "[]");
  }
  return string;
}

/** Function to converting a list of Group structs into a JSON string
*@pre Group list is not NULL
*@post Group list has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to a List struct
**/
char* groupListToJSON(const List *list) {
  char* string;
  if (list != NULL) {
    List* groups = (List*)list;
    int cur_size = 3;
    string = malloc(sizeof(char) * cur_size);
    strcpy(string, "[");
    Node* cur;
    ListIterator iterGroup = createIterator(groups);
    int count = 1;
    for (cur = nextElement(&iterGroup); cur != NULL; cur = nextElement(&iterGroup)) {
      Group* cur_group = (Group*)cur;
      char* tmpString = groupToJSON(cur_group);
      cur_size += strlen(tmpString) + 2;
      string = realloc(string, sizeof(char) * cur_size);
      strcat(string, tmpString);
      if (count < getLength(groups)) {
        strcat(string, ",");
      }
      count++;
      
      free(tmpString);
    }
    strcat(string, "]");
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "[]");
  }
  return string;
}

/** Function to converting an SVGimage into a JSON string
*@pre SVGimage is not NULL
*@post SVGimage has not been modified in any way
*@return A string in JSON format
*@param list - a pointer to an SVGimage struct
**/
char* SVGtoJSON(const SVGimage* imge) {
  char* string;
  if (imge != NULL) {
    SVGimage* img = (SVGimage*)imge;
    List* tmpCirc = getCircles(img);
    List* tmpRect = getRects(img);
    List* tmpPath = getPaths(img);
    List* tmpGroup = getGroups(img);
    char tmpC[5] = "";
    char tmpR[5] = "";
    char tmpP[5] = "";
    char tmpG[5] = "";
    snprintf(tmpC, 5, "%d", getLength(tmpCirc)); 
    snprintf(tmpR, 5, "%d", getLength(tmpRect)); 
    snprintf(tmpP, 5, "%d", getLength(tmpPath)); 
    snprintf(tmpG, 5, "%d", getLength(tmpGroup)); 
    string = malloc(sizeof(char) * (strlen(tmpC) + strlen(tmpR) + strlen(tmpP) + strlen(tmpG)) + 50);

    strcpy(string, "{\"numRect\":");
    strcat(string, tmpR);
    strcat(string, ",\"numCirc\":");
    strcat(string, tmpC);
    strcat(string, ",\"numPaths\":");
    strcat(string, tmpP);
    strcat(string, ",\"numGroups\":");
    strcat(string, tmpG);
    strcat(string, "}");

    freeListNotNodes(tmpCirc);
    freeListNotNodes(tmpRect);
    freeListNotNodes(tmpPath);
    freeListNotNodes(tmpGroup);
  } else {
    string = malloc(sizeof(char) * 3);
    strcpy(string, "{}");
  }
  return string;
}

/** Function to converting a JSON string into an SVGimage struct
*@pre JSON string is not NULL
*@post String has not been modified in any way
*@return A newly allocated and initialized SVGimage struct
*@param svgString - a pointer to a string
**/
SVGimage* JSONtoSVG(const char* svgString) {
  SVGimage* img;
  if (svgString != NULL) {
    char split[2][50];
    int i = 0;
    char* tmp = (char*)svgString;
    img = malloc(sizeof(SVGimage));
    initImg(img);
    strcpy(img->namespace, "http://www.w3.org/2000/svg");
    char* token = strtok(tmp, ","); 
    while (token != NULL) { 
      strcpy(split[i], "");
      strcpy(split[i], token);
      token = strtok(NULL, ","); 
      i++;
    }
    char split2[2][50];
    i = 0;
    token = strtok(split[0], ":"); 
    while (token != NULL) { 
      strcpy(split2[i], "");
      strcpy(split2[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split2[1]); i++) {
      if (split2[1][i] == '\"') {
      } else {
        strncat(img->title, &split2[1][i], 1);
      }
    }
    char split3[2][50];
    i = 0;
    token = strtok(split[1], ":"); 
    while (token != NULL) { 
      strcpy(split3[i], "");
      strcpy(split3[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split3[1]); i++) {
      if ((split3[1][i] == '\"') || (split3[1][i] == '}')) {
      } else {
        strncat(img->description, &split3[1][i], 1);
      }
    }

    Attribute* a1 = malloc(sizeof(Attribute));
    a1->name = malloc(sizeof(char) * strlen("height") + 1);
    strcpy(a1->name, "height");
    a1->value = malloc(sizeof(char) * strlen("200") + 1);
    strcpy(a1->value, "200");
    insertBack(img->otherAttributes, a1);

    Attribute* a2 = malloc(sizeof(Attribute));
    a2->name = malloc(sizeof(char) * strlen("width") + 1);
    strcpy(a2->name, "width");
    a2->value = malloc(sizeof(char) * strlen("200") + 1);
    strcpy(a2->value, "200");
    insertBack(img->otherAttributes, a2);

    return img;
  } else {
    return NULL;
  }
}

/** Function to converting a JSON string into a Rectangle struct
*@pre JSON string is not NULL
*@post Rectangle has not been modified in any way
*@return A newly allocated and initialized Rectangle struct
*@param svgString - a pointer to a string
**/
Rectangle* JSONtoRect(const char* svgString) {
  Rectangle* rect;
  if (svgString != NULL) {
    int i = 0;
    char* tmp = (char*)svgString;
    char buffer[50] = "";
    rect = malloc(sizeof(Rectangle));
    rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    rect->x = 0;
    rect->y = 0;
    rect->height = 0;
    rect->width = 0;
    strcpy(rect->units, "");
    char split[5][50];

    char* token = strtok(tmp, ","); 
    while (token != NULL) { 
      strcpy(split[i], "");
      strcpy(split[i], token);
      token = strtok(NULL, ","); 
      i++;
    }
    char split2[2][50];
    i = 0;
    token = strtok(split[0], ":"); 
    while (token != NULL) { 
      strcpy(split2[i], "");
      strcpy(split2[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split2[1]); i++) {
      if (split2[1][i] == '\"') {
      } else {
        strncat(buffer, &split2[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteRectangle(rect);
      return NULL;
    }
    rect->x = atof(buffer);
    strcpy(buffer, "");

    char split3[2][50];
    i = 0;
    token = strtok(split[1], ":"); 
    while (token != NULL) { 
      strcpy(split3[i], "");
      strcpy(split3[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split3[1]); i++) {
      if (split3[1][i] == '\"') {
      } else {
        strncat(buffer, &split3[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteRectangle(rect);
      return NULL;
    }
    rect->y = atof(buffer);
    strcpy(buffer, "");

    char split4[2][50];
    i = 0;
    token = strtok(split[2], ":"); 
    while (token != NULL) { 
      strcpy(split4[i], "");
      strcpy(split4[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split4[1]); i++) {
      if (split4[1][i] == '\"') {
      } else {
        strncat(buffer, &split4[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteRectangle(rect);
      return NULL;
    }
    rect->width = atof(buffer);
    strcpy(buffer, "");

    char split5[2][50];
    i = 0;
    token = strtok(split[3], ":"); 
    while (token != NULL) { 
      strcpy(split5[i], "");
      strcpy(split5[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split5[1]); i++) {
      if (split5[1][i] == '\"') {
      } else {
        strncat(buffer, &split5[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteRectangle(rect);
      return NULL;
    }
    rect->height = atof(buffer);
    strcpy(buffer, "");

    char split6[2][50];
    i = 0;
    token = strtok(split[4], ":"); 
    while (token != NULL) { 
      strcpy(split6[i], "");
      strcpy(split6[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split6[1]); i++) {
      if ((split6[1][i] == '\"') || (split6[1][i] == '}')) {
      } else {
        strncat(rect->units, &split6[1][i], 1);
      }
    }
    return rect;
  } else {
    return NULL;
  }
}

/** Function to converting a JSON string into a Circle struct
*@pre JSON string is not NULL
*@post Circle has not been modified in any way
*@return A newly allocated and initialized Circle struct
*@param svgString - a pointer to a string
**/
Circle* JSONtoCircle(const char* svgString) {
  Circle* circ;
  if (svgString != NULL) {
    int i = 0;
    char* tmp = (char*)svgString;
    char buffer[50] = "";
    circ = malloc(sizeof(Circle));
    circ->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    circ->cx = 0;
    circ->cy = 0;
    circ->r = 0;
    strcpy(circ->units, "");
    char split[5][50];

    char* token = strtok(tmp, ","); 
    while (token != NULL) { 
      strcpy(split[i], "");
      strcpy(split[i], token);
      token = strtok(NULL, ","); 
      i++;
    }
    char split2[2][50];
    i = 0;
    token = strtok(split[0], ":"); 
    while (token != NULL) { 
      strcpy(split2[i], "");
      strcpy(split2[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split2[1]); i++) {
      if (split2[1][i] == '\"') {
      } else {
        strncat(buffer, &split2[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteCircle(circ);
      return NULL;
    }
    circ->cx = atof(buffer);
    strcpy(buffer, "");
  
    char split3[2][50];
    i = 0;
    token = strtok(split[1], ":"); 
    while (token != NULL) { 
      strcpy(split3[i], "");
      strcpy(split3[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split3[1]); i++) {
      if (split3[1][i] == '\"') {
      } else {
        strncat(buffer, &split3[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteCircle(circ);
      return NULL;
    }
    circ->cy = atof(buffer);
    strcpy(buffer, "");

    char split4[2][50];
    i = 0;
    token = strtok(split[2], ":"); 
    while (token != NULL) { 
      strcpy(split4[i], "");
      strcpy(split4[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split4[1]); i++) {
      if (split4[1][i] == '\"') {
      } else {
        strncat(buffer, &split4[1][i], 1);
      }
    }
    if (buffer == NULL) {
      deleteCircle(circ);
      return NULL;
    }
    circ->r = atof(buffer);
    strcpy(buffer, "");

    char split5[2][50];
    i = 0;
    token = strtok(split[3], ":"); 
    while (token != NULL) { 
      strcpy(split5[i], "");
      strcpy(split5[i], token);
      token = strtok(NULL, ":"); 
      i++;
    }
    for (i = 0; i < strlen(split5[1]); i++) {
      if ((split5[1][i] == '\"')  || (split5[1][i] == '}')) {
      } else {
        strncat(circ->units, &split5[1][i], 1);
      }
    }
    return circ;
  } else {
    return NULL;
  }
}

char* getTableInfo (char* filename) {
  char* info = NULL;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  info = SVGtoJSON(img);
  deleteSVGimage(img);
  
  return info;
}

char* getTitle (char* filename) {
  char* info = NULL;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  info = malloc(sizeof(char) * (strlen(img->title)) + 30);
  strcpy(info, "\" ");
  strcat(info, img->title);
  strcat(info, "\"");
  deleteSVGimage(img);
  return info;
}

char* getDesc (char* filename) {
  char* info = NULL;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  info = malloc(sizeof(char) * (strlen(img->description)) + 30);
  strcpy(info, "\" ");
  strcat(info, img->description);
  strcat(info, "\"");
  deleteSVGimage(img);
  return info;
}

char* getCircInfo (char* filename) {
  char* circInfo;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  circInfo = circListToJSON(img->circles);
  deleteSVGimage(img);
  
  return circInfo;
}

char* getRectInfo (char* filename) {
  char* rectInfo;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  rectInfo = rectListToJSON(img->rectangles);
  deleteSVGimage(img);
  
  return rectInfo;
}

char* getPathInfo (char* filename) {
  char* pathInfo;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  pathInfo = pathListToJSON(img->paths);
  deleteSVGimage(img);
  
  return pathInfo;
}

char* getGroupInfo (char* filename) {
  char* groupInfo;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  groupInfo = groupListToJSON(img->groups);
  deleteSVGimage(img);
  
  return groupInfo;
}

void setTitle(char* filename, char* newTitle) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");

  strcpy(img->title, newTitle);
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    writeSVGimage(img, fullPath);
  }
  deleteSVGimage(img);
}

void setDescr(char* filename, char* newDesc) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");

  strcpy(img->description, newDesc);
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    writeSVGimage(img, fullPath);
  }
  deleteSVGimage(img);
}

bool newSVGFile(char* filename, char* title, char* desc) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);

  char* string = malloc(sizeof(char) * (strlen(title) + strlen(desc)) + 25);
  strcpy(string, "{\"title\":\"");
  strcat(string, title);
  strcat(string, "\",\"descr\":\"");
  strcat(string, desc);
  strcat(string, "\"}");
  //printf("In parser: %s\n", string);

  SVGimage* img = JSONtoSVG(string);
  //printf("Title: %s\n Description: %s\n", img->title, img->description);
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    bool test = writeSVGimage(img, fullPath);
    deleteSVGimage(img);
    free(string);
    return test;
  } else {
    deleteSVGimage(img);
    free(string);
    return false;
  }
}

bool addNewCirc(char* filename, char* cx, char* cy, char* r, char* units) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Circle* newCircle = malloc(sizeof(Circle));
  newCircle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  int temp = atof(cx);
  newCircle->cx = temp;
  temp = atof(cy);
  newCircle->cy = temp;
  temp = atof(r);
  newCircle->r = temp;
  strcpy(newCircle->units, units);
  addComponent(img, CIRC, newCircle);
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    bool test = writeSVGimage(img, fullPath);
    deleteSVGimage(img);
    return test;
  } else {
    deleteSVGimage(img);
    return false;
  }
}

bool addNewRect(char* filename, char* x, char* y, char* width, char* height, char* units) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Rectangle* newRect = malloc(sizeof(Rectangle));
  newRect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  int temp = atof(x);
  newRect->x = temp;
  temp = atof(y);
  newRect->y = temp;
  temp = atof(height);
  newRect->height = temp;
  temp = atof(width);
  newRect->width = temp;
  strcpy(newRect->units, units);
  addComponent(img, RECT, newRect);
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    bool test = writeSVGimage(img, fullPath);
    deleteSVGimage(img);
    return test;
  } else {
    deleteSVGimage(img);
    return false;
  }
}

char* getRectAttr (char* filename, int rectNum) {
  int index = rectNum - 1;
  char* string;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Node* cur;
  ListIterator iter = createIterator(img->rectangles);
  int count = 0;
  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Rectangle* cur_r = (Rectangle*)cur;
    if (count == index) {
      const List* list = cur_r->otherAttributes;
      string = attrListToJSON(list);
      break;
    }
    count++;
  }
  deleteSVGimage(img);
  return string;
}

char* getCircAttr (char* filename, int circNum) {
  int index = circNum - 1;
  char* string;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Node* cur;
  ListIterator iter = createIterator(img->circles);
  int count = 0;
  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Circle* cur_c = (Circle*)cur;
    if (count == index) {
      const List* list = cur_c->otherAttributes;
      string = attrListToJSON(list);
      break;
    }
    count++;
  }
  deleteSVGimage(img);
  return string;
}

char* getPathAttr (char* filename, int pathNum) {
  int index = pathNum - 1;
  char* string = NULL;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Node* cur;
  ListIterator iter = createIterator(img->paths);
  int count = 0;
  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Path* cur_p = (Path*)cur;
    if (count == index) {
      const List* list = cur_p->otherAttributes;
      string = attrListToJSON(list);
      break;
    }
    count++;
  }

  deleteSVGimage(img);
  return string;
}

char* getGroupAttr (char* filename, int groupNum) {
  int index = groupNum - 1;
  char* string = NULL;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  Node* cur;
  ListIterator iter = createIterator(img->groups);
  int count = 0;
  for (cur = nextElement(&iter); cur != NULL; cur = nextElement(&iter)) {
    Group* cur_g = (Group*)cur;
    if (count == index) {
      const List* list = cur_g->otherAttributes;
      string = attrListToJSON(list);
      break;
    }
    count++;
  }
  deleteSVGimage(img);
  return string;
}

bool addNewAttribute(char* filename, char* shape, int num, char* name, char* value) {
  int index = num - 1;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");

  Attribute* attr = malloc(sizeof(Attribute));
  attr->name = malloc(sizeof(char) * strlen(name) + 1);
  strcpy(attr->name, name);
  attr->value = malloc(sizeof(char) * strlen(value) + 1);
  strcpy(attr->value, value);

  if (strcmp(shape, "Rectangle") == 0) {
    setAttribute(img, RECT, index, attr);
  } else if (strcmp(shape, "Circle") == 0) {
    setAttribute(img, CIRC, index, attr);
  } else if (strcmp(shape, "Path") == 0) {
    setAttribute(img, PATH, index, attr);
  } else if (strcmp(shape, "Group") == 0) {
    setAttribute(img, GROUP, index, attr);
  } else if (strcmp(shape, "SVG") == 0) {
  }
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    bool test = writeSVGimage(img, fullPath);
    deleteSVGimage(img);
    return test;
  } else {
    return false;
  }
}

bool validateFile (char* filename) {
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  if (img == NULL) {
    return false;
  }
  bool check = validateSVGimage(img, "parser/bin/svg.xsd");
  if (check) {
    deleteSVGimage(img);
    return true;
  } else {
    return false;
  }
}

char* getSVGAttr (char* filename) {
  char* string;
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  const List* list = img->otherAttributes;
  string = attrListToJSON(list);
  deleteSVGimage(img);
  return string;
}

char* getSVGInfo (char* filename) {
  char* string;
  char temp[5];
  char fullPath[100] = "uploads/";
  strcat(fullPath, filename);
  SVGimage* img = createValidSVGimage(fullPath, "parser/bin/svg.xsd");
  int num = getLength(img->otherAttributes);
  sprintf(temp, "%d", num); 
  string = malloc(sizeof(char) * 20);
  strcpy(string, "[{\"numAttr\":");
  strcat(string, temp);
  strcat(string, "}]");
  deleteSVGimage(img);
  return string;
}
