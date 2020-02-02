/**
* Annaliese Schropp
* 1013472
* aschropp@uoguelph.ca
* January 28,2020
* CIS*2750
* SVG Parser Assignment 1
**/

//Includes
#include <math.h>

//Prototypes of Helper functions
void get_elements(xmlNode* a_node, SVGimage *img);
void initImg(SVGimage *img);
Circle* createCircle (xmlNode* cur_node);
Rectangle* createRect (xmlNode* cur_node);
Path* createPath(xmlNode* cur_node);
Group* createGroup(xmlNode* cur_node, SVGimage* img);
void initGroup(Group* group);
void getNestedRects(List* rects, Group* cur_group);
void getNestedCircles(List* circs, Group* cur_group);
void getNestedPaths(List* paths, Group* cur_group);
void getNestedGroups(List* grps, Group* cur_group);
void freeListNotNodes(List* list);