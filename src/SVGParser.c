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
    deleteSVGimage(newImg);
    return NULL;
  }

  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);

  get_elements(root_element, newImg);

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

      if (strcmp((char*)cur_node->name, "svg") == 0) {
        strcpy(img->namespace, (char*)cur_node->ns->href);

        for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
          Attribute* attr = malloc(sizeof(Attribute));
          attr->name = malloc(strlen((char*)tmpAttr->name) + 1);
          strcpy(attr->name, (char*)tmpAttr->name);
          attr->value = malloc(strlen((char*)tmpAttr->children->content) + 1);
          strcpy(attr->value, (char*)tmpAttr->children->content);
          insertBack(img->otherAttributes, attr);
        }

      } else if (strcmp((char*)cur_node->name, "title") == 0) {
        strcpy(img->title, (char*)cur_node->children->content);

      } else if (strcmp((char*)cur_node->name, "desc") == 0) {
        strcpy(img->description, (char*)cur_node->children->content);

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
  char* units;

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if (strcmp((char*)tmpAttr->name, "cx") == 0) {
      newCircle->cx = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newCircle->units, units);

    } else if (strcmp((char*)tmpAttr->name, "cy") == 0) {
      newCircle->cy = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newCircle->units, units);

    } else if (strcmp((char*)tmpAttr->name, "r") == 0) {
      newCircle->r = atof((char*)tmpAttr->children->content);

    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(strlen((char*)tmpAttr->children->content) + 1);
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
  strcpy(newRect->units, "");
  char* units;

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if (strcmp((char*)tmpAttr->name, "x") == 0) {
      newRect->x = atof((char*)tmpAttr->children->content);

    } else if (strcmp((char*)tmpAttr->name, "y") == 0) {
      newRect->y = atof((char*)tmpAttr->children->content);

    } else if (strcmp((char*)tmpAttr->name, "width") == 0) {
      newRect->width = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newRect->units, units);

    } else if (strcmp((char*)tmpAttr->name, "height") == 0) {
      newRect->height = strtof((char*)tmpAttr->children->content, &units);
      strcpy(newRect->units, units);
    
    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(strlen((char*)tmpAttr->children->content) + 1);
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

  for (xmlAttr* tmpAttr = cur_node->properties; tmpAttr != NULL; tmpAttr = tmpAttr->next) {
    if (strcmp((char*)tmpAttr->name, "d") == 0) {
      newPath->data = malloc(strlen((char*)tmpAttr->children->content) + 1);
      strcpy(newPath->data, (char*)tmpAttr->children->content);

    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(strlen((char*)tmpAttr->name) + 1);
      strcpy(attr->name, (char*)tmpAttr->name);
      attr->value = malloc(strlen((char*)tmpAttr->children->content) + 1);
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

    attr->name = malloc(strlen((char*)tmpAttr->name) + 1);
    strcpy(attr->name, (char*)tmpAttr->name);
    attr->value = malloc(strlen((char*)tmpAttr->children->content) + 1);
    strcpy(attr->value, (char*)tmpAttr->children->content);

    insertBack(newGroup->otherAttributes, attr);
  }

  cur_node = cur_node->children;
  while(strcmp((char*)cur_node->parent->name, "g") == 0) {
    if (strcmp((char*)cur_node->name, "g") == 0) {
        Group* nestedGroup = createGroup(cur_node, img);
        insertBack(newGroup->groups, nestedGroup);

    } else if (strcmp((char*)cur_node->name, "path") == 0){
        Path* newPath = createPath(cur_node);
        insertBack(newGroup->paths, newPath);

    } else if (strcmp((char*)cur_node->name, "circle") == 0){
        Circle* newCircle = createCircle(cur_node);
        insertBack(newGroup->circles, newCircle);

    } else if (strcmp((char*)cur_node->name, "rect") == 0) {
        Rectangle* newRect = createRect(cur_node);
        insertBack(newGroup->rectangles, newRect);

    } else if (strcmp((char*)cur_node->name, "text") == 0) {
      //ignore if this occurs
    } else {
      Attribute* attr = malloc(sizeof(Attribute));

      attr->name = malloc(strlen((char*)cur_node->properties->name) + 1);
      strcpy(attr->name, (char*)cur_node->properties->name);
      attr->value = malloc(strlen((char*)cur_node->properties->children->content) + 1);
      strcpy(attr->value, (char*)cur_node->properties->children->content);
      insertBack(newGroup->otherAttributes, attr);

      cur_node->properties = cur_node->properties->next;
    }

    if (cur_node->next) {
      cur_node = cur_node->next;
    } else {
      break;
    }
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
	
	if (list->head == NULL && list->tail == NULL){
		return;
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
    return "Rectangle is NULL";
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
    return "Circle is NULL";
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
    return tmp->data;
  } else {
    return "Path is NULL";
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
