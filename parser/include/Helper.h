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
#include <unistd.h>

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

/****************** A2 Prototypes ********************/
int validateDoc (xmlDoc* doc, char* schemaFile, xmlSchemaPtr schema);
xmlDoc* createTree(SVGimage* img);
void addAttr (List* list, xmlNodePtr node);
void addLists(Group* cur_group, xmlNodePtr root_node);
bool checkRectangles(SVGimage* img);
bool checkCircles(SVGimage* img);
bool checkPathAttr(SVGimage* img);
bool checkGroupAttr(SVGimage* img);
void setCircAttr(List* list, int index, Attribute* attribute);
void setRectAttr(List* list, int index, Attribute* attribute);
void setPathAttr(List* list, int index, Attribute* attribute);
void setGroupAttr(List* list, int index, Attribute* attribute);
bool setOtherAttr(List* list, Attribute* attribute);

char* getTableInfo (char* filename);
char* getTitle (char* filename);
char* getDesc (char* filename);
char* getCircInfo (char* filename);
char* getRectInfo (char* filename);
char* getPathInfo (char* filename);
char* getGroupInfo (char* filename);
void setDescr(char* filename, char* newDesc);
void setTitle(char* filename, char* newTitle);
bool newSVGFile(char* filename, char* title, char* desc);
bool addNewCirc(char* filename, char* cx, char* cy, char* r, char* units);
bool addNewRect(char* filename, char* x, char* y, char* width, char* height, char* units);
char* getRectAttr (char* filename, int rectNum);
char* getCircAttr (char* filename, int circNum);
char* getPathAttr (char* filename, int pathNum);
char* getGroupAttr (char* filename, int groupNum);
bool addNewAttribute(char* filename, char* shape, int num, char* name, char* value);
bool validateFile (char* filename);
char* getSVGAttr (char* filename);
char* getSVGInfo (char* filename);
