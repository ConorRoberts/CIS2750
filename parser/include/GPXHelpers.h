#include "GPXParser.h"
#include "LinkedListAPI.h"

#ifndef _GPX_HELPERS_H_
#define _GPX_HELPERS_H_

/**
 * @brief 
 * 
 * @param xml_node 
 * @param pattern 
 * @return int 
 */
int xmlNameCompare(xmlNode *xml_node, char *pattern);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param wpt 
 */
void checkAndAssignWptName(xmlNode *xml_node, Waypoint *wpt);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param rte 
 */
void checkAndAssignRteName(xmlNode *xml_node, Route *rte);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param trk 
 */
void checkAndAssignTrkName(xmlNode *xml_node, Track *trk);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param wpt 
 */
void assignLatLon(xmlNode *xml_node, Waypoint *wpt);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param gpxList 
 */
void addMiscGPXData(xmlNode *xml_node, List *gpxList);

/**
 * @brief 
 * 
 * @param xml_node 
 * @param otherData 
 */
void handleMiscGPXData(xmlNode *xml_node, List *otherData);

/**
 * @brief 
 * 
 * @param xml_node 
 * @return Waypoint* 
 */
Waypoint *nodeToWaypoint(xmlNode *xml_node);

/**
 * @brief 
 * 
 * @param xml_node 
 * @return Route* 
 */
Route *nodeToRoute(xmlNode *xml_node);

/**
 * @brief 
 * 
 * @param xml_node 
 * @return Track* 
 */
Track *nodeToTrack(xmlNode *xml_node);

/**
 * @brief 
 * 
 * @param parent 
 * @param waypoints 
 * @param routes 
 * @param tracks 
 */
void populateList(xmlNode *parent, List *waypoints, List *routes, List *tracks);

/**
 * @brief Populates parent node with data contained in wpt
 * 
 * @param wpt Waypoint pointer
 * @param parent Xml node pointer
 */
void populateWptNode(Waypoint *wpt, xmlNode *parent, xmlNsPtr ns);

/**
 * @brief Populates parent node with data contained in rte
 * 
 * @param rte Route pointer
 * @param parent Xml node pointer
 */
void populateRteNode(Route *rte, xmlNode *parent, xmlNsPtr ns);

/**
 * @brief Populates parent node with data contained in other
 * 
 * @param other List pointer
 * @param parent Xml node pointer 
 */
void populateNodeOther(List *other, xmlNode *parent, xmlNsPtr ns);

/**
 * @brief Populates parent node with data contained in trk
 * 
 * @param other Track pointer
 * @param parent Xml node pointer 
 */
void populateTrkNode(Track *trk, xmlNode *parent, xmlNsPtr ns);

/**
 * @brief Populates parent node with data contained in seg
 * 
 * @param other Segment pointer
 * @param parent Xml node pointer 
 */
void populateSegNode(TrackSegment *seg, xmlNode *parent, xmlNsPtr ns);

/**
 * @brief Converts a GPXdoc to an xmlDoc
 * 
 * @param doc GPX doc pointer
 * @return xmlDocPtr Xml doc pointer
 */
xmlDocPtr gpxToXmlDoc(GPXdoc *doc);

/**
 * @brief Error function for xmlparser/schema errors
 * 
 * @param ctx 
 * @param msg 
 * @param ... 
 */
void err(void *ctx, const char *msg, ...);

/**
 * @brief Warning function for xmlparser/schema warnings
 * 
 * @param ctx 
 * @param msg 
 * @param ... 
 */
void warn(void *ctx, const char *msg, ...);

/**
 * @brief Validates xml doc
 * 
 * @param doc Xml doc pointer
 * @param gpxSchemaFile Name of gpx schema file
 * @return true Doc is valid
 * @return false Doc is not valid
 */
bool validateXmlDoc(xmlDocPtr doc, char *gpxSchemaFile);

/**
 * @brief Validate a GPXdata struct
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool validateGPXData(void *data);

/**
 * @brief Validate a waypoint struct
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool validateWaypoint(void *data);

/**
 * @brief Validate a route struct
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool validateRoute(void *data);

/**
 * @brief Validate a track struct
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool validateTrack(void *data);

/**
 * @brief Validate a tracksegment struct
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool validateTrackSegment(void *data);

/**
 * @brief Uses above functions to validate a list of elements
 * 
 * @param list 
 * @param validate 
 * @return true 
 * @return false 
 */
bool validateList(List *list, bool (*validate)(void *data));

/**
 * @brief Computes the haversine distance between two waypoints
 * 
 * @param wp_1 
 * @param wp_2 
 * @return float 
 */
float wptHaversine(Waypoint *wp_1, Waypoint *wp_2);

/**
 * @brief Converts degrees to radians
 * 
 * @param f 
 * @return float 
 */
float toRadians(float f);

/**
 * @brief Returns whether the given length is within delta of target
 * 
 * @param len 
 * @param target 
 * @param delta 
 * @return true 
 * @return false 
 */
bool withinDelta(float len, float target, float delta);

void dummyDelete(void *data);

char *getFileTableData(char *fileName);

char *getWaypoints(char *fileName);
char *getRoutes(char *fileName);
char *getTracks(char *fileName);

char *gpxDataToJSON(GPXData *data);
char *waypointToJSONBetter(const Waypoint *rt);
char *trackToJSONBetter(const Track *tr);
char *routeToJSONBetter(const Route *rt);

char *gpxDataListToJSON(List *data);
char *waypointListToJSONBetter(const List *list);
char *routeListToJSONBetter(const List *list);
char *trackListToJSONBetter(const List *list);

void renameComponent(char *fileName, char *newName, char *type, int index);
int JSONtoGPXFile(char *fileName, char *str);
void addRouteToFile(char *fileName, char *str);
void addWaypointToRoute(char *fileName, char *str, int index);
char *getPathsBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta);

#endif
