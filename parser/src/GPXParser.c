#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelpers.h"

GPXdoc *createGPXdoc(char *fileName)
{

    /*parse the file and get the DOM */
    xmlDoc *xml_doc = NULL;
    xml_doc = (xmlDoc *)xmlReadFile(fileName, NULL, 0);

    if (xml_doc == NULL)
        return NULL;

    GPXdoc *doc = (GPXdoc *)malloc(sizeof(GPXdoc));
    xmlNode *root_element = NULL;
    root_element = xmlDocGetRootElement(xml_doc);

    // If any of these return a NULL value this whole object should be NULL.
    char *xml_creator = (char *)xmlGetProp(root_element, (const xmlChar *)"creator");

    // This is absolute garbage
    char *xml_namespace = (char *)root_element->ns->href;
    char *xml_version_tmp = (char *)xmlGetProp(root_element, (const xmlChar *)"version");

    if (xml_creator == NULL || xml_namespace == NULL || xml_version_tmp == NULL)
        return NULL;

    double xml_version = atof(xml_version_tmp);

    doc->creator = malloc(strlen(xml_creator) + 1);
    strcpy(doc->creator, xml_creator);

    doc->version = xml_version;

    strcpy(doc->namespace, xml_namespace);

    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareRoutes);

    populateList(root_element->children, doc->waypoints, doc->routes, doc->tracks);

    free(xml_creator);
    free(xml_version_tmp);

    if (xml_doc != NULL)
        xmlFreeDoc(xml_doc);
    xmlCleanupParser();
    return doc;
}

char *GPXdocToString(GPXdoc *doc)
{
    if (doc == NULL)
        return NULL;

    // Add stuff for the header
    int header_len = strlen(doc->namespace) + strlen(doc->creator) + 50;

    // Init the main string
    char str_header[header_len];
    sprintf(str_header, "Namespace: %s, Creator: %s, Version: %lf\n", doc->namespace, doc->creator, doc->version);

    char *str_waypoints = toString(doc->waypoints);
    char *str_routes = toString(doc->routes);
    char *str_tracks = toString(doc->tracks);

    int data_len = strlen(str_routes) + strlen(str_waypoints) + strlen(str_tracks) + 50;
    char str_data[data_len];

    sprintf(str_data, "%s%s%s", str_waypoints, str_routes, str_tracks);

    char *str = (char *)malloc(header_len + data_len + 1);
    sprintf(str, "%s%s", str_header, str_data);

    if (str_waypoints)
        free(str_waypoints);
    if (str_routes)
        free(str_routes);
    if (str_tracks)
        free(str_tracks);

    return str;
}

void deleteGPXdoc(GPXdoc *doc)
{
    xmlCleanupParser();
    if (doc == NULL)
        return;
    freeList(doc->waypoints);
    freeList(doc->routes);
    freeList(doc->tracks);
    free(doc->creator);
    free(doc);
}

int getNumWaypoints(const GPXdoc *doc)
{
    if (doc == NULL)
        return 0;

    return doc->waypoints->length;
}

int getNumRoutes(const GPXdoc *doc)
{
    if (doc == NULL)
        return 0;

    return doc->routes->length;
}

int getNumTracks(const GPXdoc *doc)
{
    if (doc == NULL)
        return 0;

    return doc->tracks->length;
}

int getNumSegments(const GPXdoc *doc)
{
    if (doc == NULL)
        return 0;

    int count = 0;
    void *element;
    ListIterator iter_trk = createIterator(doc->tracks);
    while ((element = nextElement(&iter_trk)) != NULL)
    {
        Track *trk = (Track *)element;
        count += trk->segments->length;
    }
    return count;
}

int getNumGPXData(const GPXdoc *doc)
{
    if (doc == NULL)
        return 0;

    int count = 0;
    void *element;
    void *element_child;
    void *element_child_child;
    ListIterator iter_wpt;
    ListIterator iter_rte;
    ListIterator iter_trk;
    ListIterator iter_seg;

    count += doc->tracks->length;
    iter_trk = createIterator(doc->tracks);
    while ((element = nextElement(&iter_trk)) != NULL)
    {
        // Now we are looking at a single track
        Track *trk = (Track *)element;
        count += trk->otherData->length;

        iter_seg = createIterator(trk->segments);
        while ((element_child = nextElement(&iter_seg)) != NULL)
        {
            // Now we are looking at a single segment
            TrackSegment *seg = (TrackSegment *)element_child;

            // Count GPX in seg->waypoints
            iter_wpt = createIterator(seg->waypoints);
            while ((element_child_child = nextElement(&iter_wpt)) != NULL)
            {
                // Now we are looking at a single waypoint
                Waypoint *wpt = (Waypoint *)element_child_child;
                if (wpt != NULL)
                    if (strcmp(wpt->name, "") != 0)
                        count++;
                count += wpt->otherData->length;
            }
        }
    }

    count += doc->routes->length;
    iter_rte = createIterator(doc->routes);
    while ((element = nextElement(&iter_rte)) != NULL)
    {
        Route *rte = (Route *)element;

        // Count GPX in rte->otherData
        count += rte->otherData->length;

        // Count GPX in rte->waypoints
        iter_wpt = createIterator(rte->waypoints);
        while ((element_child = nextElement(&iter_wpt)) != NULL)
        {
            Waypoint *wpt = (Waypoint *)element_child;
            if (wpt != NULL)
                if (strcmp(wpt->name, "") != 0)
                    count++;
            count += wpt->otherData->length;
        }
    }

    // Count GPX in waypoints
    count += doc->waypoints->length;
    iter_wpt = createIterator(doc->waypoints);
    while ((element = nextElement(&iter_wpt)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        count += wpt->otherData->length;
    }

    return count;
}

Waypoint *getWaypoint(const GPXdoc *doc, char *name)
{
    if (doc == NULL)
        return NULL;

    void *element;

    ListIterator iter = createIterator(doc->waypoints);
    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        if (strcmp(wpt->name, name) == 0)
        {
            return wpt;
        }
    }

    return NULL;
}

Track *getTrack(const GPXdoc *doc, char *name)
{
    if (doc == NULL)
        return NULL;

    void *element;

    ListIterator iter = createIterator(doc->tracks);
    while ((element = nextElement(&iter)) != NULL)
    {
        Track *trk = (Track *)element;
        if (strcmp(trk->name, name) == 0)
        {
            return trk;
        }
    }

    return NULL;
}

Route *getRoute(const GPXdoc *doc, char *name)
{
    if (doc == NULL)
        return NULL;

    void *element;

    ListIterator iter = createIterator(doc->routes);
    while ((element = nextElement(&iter)) != NULL)
    {
        Route *rte = (Route *)element;
        if (strcmp(rte->name, name) == 0)
        {
            return rte;
        }
    }

    return NULL;
}

// HELPERS

void deleteGpxData(void *data)
{
    if (data == NULL)
        return;

    GPXData *gpx = (GPXData *)data;

    free(gpx);
}
char *gpxDataToString(void *data)
{
    if (data == NULL)
        return NULL;

    GPXData *gpx = (GPXData *)data;
    int len = strlen(gpx->name) + strlen(gpx->value) + 50;
    char *tmpStr = (char *)malloc(sizeof(char) * len);

    sprintf(tmpStr, "GPXData | Name: %s, Value:%s\n", gpx->name, gpx->value);

    return tmpStr;
}
int compareGpxData(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }

    GPXData *gpx_first = (GPXData *)first;
    GPXData *gpx_second = (GPXData *)second;

    return (strcmp(gpx_first->name, gpx_second->name));
}

void deleteWaypoint(void *data)
{
    if (data == NULL)
        return;

    Waypoint *wpt = (Waypoint *)data;

    if (wpt->name != NULL)
        free(wpt->name);
    if (wpt->otherData != NULL)
        freeList(wpt->otherData);
    free(wpt);
}
char *waypointToString(void *data)
{
    if (data == NULL)
        return NULL;

    Waypoint *wpt = (Waypoint *)data;
    int len = strlen(wpt->name) + sizeof(double) * 2 + 50;
    char *tmpStr = (char *)malloc(sizeof(char) * len);

    sprintf(tmpStr, "Waypoint | Name: %s, Lat: %f, Lon: %f\n", wpt->name, wpt->latitude, wpt->longitude);

    return tmpStr;
}
int compareWaypoints(const void *first, const void *second)
{
    Waypoint *wpt_first = (Waypoint *)first;
    Waypoint *wpt_second = (Waypoint *)second;

    return (strcmp(wpt_first->name, wpt_second->name));
}

void deleteRoute(void *data)
{
    if (data == NULL)
        return;

    Route *rte = (Route *)data;

    if (rte->name != NULL)
        free(rte->name);
    if (rte->otherData != NULL)
        freeList(rte->otherData);
    if (rte->waypoints != NULL)
        freeList(rte->waypoints);
    free(rte);
}
char *routeToString(void *data)
{
    if (data == NULL)
        return NULL;

    Route *rte = (Route *)data;
    char *wptStr = toString(rte->waypoints);
    int len = strlen(rte->name) + 50 + strlen(wptStr);
    char *str = (char *)malloc(len);

    // Probably print the waypoints under it too
    sprintf(str, "Route | Name: %s\n%s", rte->name, wptStr);

    free(wptStr);

    return str;
}
int compareRoutes(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }

    // Ideally I should be using the toString methods and comparing those
    Route *rte_first = (Route *)first;
    Route *rte_second = (Route *)second;

    return (strcmp(rte_first->name, rte_second->name));
}

void deleteTrackSegment(void *data)
{
    if (data == NULL)
        return;

    TrackSegment *seg = (TrackSegment *)data;

    if (seg->waypoints != NULL)
        freeList(seg->waypoints);
    free(seg);
}
char *trackSegmentToString(void *data)
{
    if (data == NULL)
        return NULL;

    TrackSegment *seg = (TrackSegment *)data;
    char *wptStr = toString(seg->waypoints);
    char *str = (char *)malloc(25 + strlen(wptStr));

    // Probably print the waypoints under it too
    sprintf(str, "TrackSegment\n%s", wptStr);

    free(wptStr);

    return str;
}
int compareTrackSegments(const void *first, const void *second)
{
    // I have decided to compare track segments by latitude sum
    if (first == NULL || second == NULL)
    {
        return 0;
    }

    double sum_first = 0;
    double sum_second = 0;

    TrackSegment *trk_first = (TrackSegment *)first;
    TrackSegment *trk_second = (TrackSegment *)second;
    void *element;

    ListIterator iter = createIterator(trk_first->waypoints);
    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        sum_first += wpt->latitude;
    }

    iter = createIterator(trk_second->waypoints);
    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        sum_second = wpt->latitude;
    }

    return (sum_first > sum_second ? 1 : sum_first == sum_second ? 0
                                                                 : -1);
}

void deleteTrack(void *data)
{
    if (data == NULL)
        return;

    Track *trk = (Track *)data;

    if (trk->name != NULL)
        free(trk->name);
    if (trk->segments != NULL)
        freeList(trk->segments);
    if (trk->otherData != NULL)
        freeList(trk->otherData);
    free(trk);
}
char *trackToString(void *data)
{
    if (data == NULL)
        return NULL;

    Track *trk = (Track *)data;
    char *segStr = toString(trk->segments);
    int len = strlen(trk->name) + 50 + strlen(segStr);
    char *str = (char *)malloc(len);

    // Probably print the waypoints under it too
    sprintf(str, "Track | Name: %s\n%s", trk->name, segStr);

    free(segStr);

    return str;
}
int compareTracks(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }

    Track *trk_first = (Track *)first;
    Track *trk_second = (Track *)second;

    return (strcmp(trk_first->name, trk_second->name));
}

// A2

GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile)
{
    // Check if file name is valid
    xmlDocPtr doc = xmlReadFile(fileName, NULL, XML_PARSE_NOERROR);

    if (doc == NULL || strcmp(fileName, "") == 0)
        return NULL;

    // Validate XML doc
    bool valid = validateXmlDoc(doc, gpxSchemaFile);

    // Cleanup
    xmlFreeDoc(doc);

    // If valid, return new GPX doc
    if (valid)
    {
        GPXdoc *gpx_doc = createGPXdoc(fileName);
        return gpx_doc;
    }

    return NULL;
}

bool validateGPXDoc(GPXdoc *doc, char *gpxSchemaFile)
{
    if (doc == NULL || gpxSchemaFile == NULL)
        return false;

    if (!validateList(doc->waypoints, validateWaypoint) || !validateList(doc->routes, validateRoute) || !validateList(doc->routes, validateRoute) || !validateList(doc->tracks, validateTrack))
        return false;

    if (strcmp(doc->namespace, "") == 0 || strcmp(doc->creator, "") == 0 || doc->creator == NULL || doc->waypoints == NULL || doc->tracks == NULL || doc->routes == NULL)
        return false;

    xmlDocPtr xml_doc = gpxToXmlDoc(doc);

    if (!validateXmlDoc(xml_doc, gpxSchemaFile))
        return false;

    xmlFreeDoc(xml_doc);

    return true;
}

bool writeGPXdoc(GPXdoc *doc, char *fileName)
{
    FILE *fp = fopen(fileName, "w+");
    if (doc == NULL || fileName == NULL || fp == NULL)
        return false;

    xmlDocPtr xml_doc = gpxToXmlDoc(doc);

    // Saves to file
    // xmlSaveFormatFile(fileName, xml_doc, 1);
    xmlSaveFormatFileEnc(fileName, xml_doc, "UTF-8", 1);

    fclose(fp);
    if (xml_doc)
        xmlFreeDoc(xml_doc);

    return true;
}

float getRouteLen(const Route *rt)
{
    if (rt == NULL)
        return 0;

    float len = 0;

    void *element, *element_next;
    ListIterator iter = createIterator(rt->waypoints);
    ListIterator iter_next = createIterator(rt->waypoints);

    element_next = nextElement(&iter_next);

    while ((element = nextElement(&iter)) != NULL && (element_next = nextElement(&iter_next)) != NULL)
    {
        Waypoint *wpt_1 = (Waypoint *)element;
        Waypoint *wpt_2 = (Waypoint *)element_next;

        len += wptHaversine(wpt_1, wpt_2);
    }

    return len;
}

float getTrackLen(const Track *tr)
{
    if (tr == NULL)
        return 0;

    float len = 0;

    void *element, *element_next, *element_seg, *element_seg_next;
    ListIterator iter_seg = createIterator(tr->segments);

    while ((element_seg = nextElement(&iter_seg)) != NULL)
    {
        TrackSegment *seg = (TrackSegment *)element_seg;

        ListIterator iter = createIterator(seg->waypoints);
        ListIterator iter_next = createIterator(seg->waypoints);
        element_next = nextElement(&iter_next);

        while ((element = nextElement(&iter)) != NULL && (element_next = nextElement(&iter_next)) != NULL)
            len += wptHaversine((Waypoint *)element, (Waypoint *)element_next);
    }

    iter_seg = createIterator(tr->segments);
    ListIterator iter_seg_next = createIterator(tr->segments);
    element_seg_next = nextElement(&iter_seg_next);

    while ((element_seg = nextElement(&iter_seg)) != NULL && (element_seg_next = nextElement(&iter_seg_next)) != NULL)
    {
        TrackSegment *seg_1 = (TrackSegment *)element_seg;
        TrackSegment *seg_2 = (TrackSegment *)element_seg_next;

        len += wptHaversine((Waypoint *)seg_1->waypoints->tail->data, (Waypoint *)seg_2->waypoints->head->data);
    }

    return len;
}

float round10(float len)
{
    // Remainder after taking all whole 10s out
    float remainder = fmod(len, 10);

    // Original number without remainder
    float rounded = len - remainder;

    remainder = (remainder >= 5 ? 10 : 0);

    return (rounded + remainder);
}

int numRoutesWithLength(const GPXdoc *doc, float len, float delta)
{
    if (doc == NULL || len < 0 || delta < 0)
        return 0;

    int n = 0;

    void *element;
    ListIterator iter = createIterator(doc->routes);
    while ((element = nextElement(&iter)) != NULL)
    {
        Route *rte = (Route *)element;

        if (withinDelta(getRouteLen(rte), len, delta))
            n++;
    }

    return n;
}

int numTracksWithLength(const GPXdoc *doc, float len, float delta)
{
    if (doc == NULL || len < 0 || delta < 0)
        return 0;

    int n = 0;

    void *element;
    ListIterator iter = createIterator(doc->tracks);
    while ((element = nextElement(&iter)) != NULL)
    {
        Track *trk = (Track *)element;

        if (withinDelta(getTrackLen(trk), len, delta))
            n++;
    }

    return n;
}

bool isLoopRoute(const Route *route, float delta)
{
    if (route == NULL || delta < 0)
        return false;

    Waypoint *first = (Waypoint *)getFromFront(route->waypoints);
    Waypoint *last = (Waypoint *)getFromBack(route->waypoints);

    // At least 4 waypoints
    // Have a distance less than delta between first and last points
    if (getLength(route->waypoints) >= 4 && wptHaversine(first, last) < delta)
        return true;

    return false;
}

bool isLoopTrack(const Track *tr, float delta)
{
    if (tr == NULL || delta < 0)
        return false;

    void *element_seg;
    ListIterator iter_seg = createIterator(tr->segments);
    bool waypoint_threshold_met = false;

    // This represents the first element in the list
    // Iterator will be moved forwards
    while ((element_seg = nextElement(&iter_seg)) != NULL)
    {
        TrackSegment *seg = (TrackSegment *)element_seg;

        if (getLength(seg->waypoints) >= 4)
            waypoint_threshold_met = true;
    }

    TrackSegment *seg_first = (TrackSegment *)getFromFront(tr->segments);
    TrackSegment *seg_last = (TrackSegment *)getFromBack(tr->segments);

    Waypoint *first = (Waypoint *)getFromFront(seg_first->waypoints);
    Waypoint *last = (Waypoint *)getFromBack(seg_last->waypoints);

    // At least 4 waypoints
    // Have a distance less than delta between first and last points
    if (waypoint_threshold_met && wptHaversine(first, last) < delta)
        return true;

    return false;
}

List *getRoutesBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    // Return NULL if ther are no routes between the specified locations or doc is NULL
    if (doc == NULL || getLength(doc->routes) == 0 || doc->routes == NULL)
        return NULL;

    List *new_list = initializeList(routeToString, dummyDelete, compareRoutes);

    Waypoint source;
    source.latitude = sourceLat;
    source.longitude = sourceLong;

    Waypoint dest;
    dest.latitude = destLat;
    dest.longitude = destLong;

    void *element_rte;
    ListIterator iter_rte = createIterator(doc->routes);
    while ((element_rte = nextElement(&iter_rte)) != NULL)
    {
        Route *rte = (Route *)element_rte;
        Waypoint *first = (Waypoint *)getFromFront(rte->waypoints);
        Waypoint *last = (Waypoint *)getFromBack(rte->waypoints);

        if (first==NULL || last==NULL)
            continue;

        // Start is within delta metres of source
        // End is within delta metres of dest
        if (wptHaversine(first, &source) < delta && wptHaversine(last, &dest) < delta){
            insertBack(new_list, (void *)rte);
        }
    }

    if (getLength(new_list) == 0)
        return NULL;

    return new_list;
}

List *getTracksBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    // Return NULL if ther are no routes between the specified locations or doc is NULL
    if (doc == NULL || getLength(doc->tracks) == 0)
        return NULL;

    List *new_list = initializeList(trackToString, dummyDelete, compareTracks);

    Waypoint source;
    source.latitude = sourceLat;
    source.longitude = sourceLong;

    Waypoint dest;
    dest.latitude = destLat;
    dest.longitude = destLong;

    void *element_trk;
    ListIterator iter_trk = createIterator(doc->tracks);

    while ((element_trk = nextElement(&iter_trk)) != NULL)
    {
        Track *trk = (Track *)element_trk;

        TrackSegment *seg_first = (TrackSegment *)getFromFront(trk->segments);
        TrackSegment *seg_last = (TrackSegment *)getFromBack(trk->segments);

        Waypoint *first = (Waypoint *)getFromFront(seg_first->waypoints);
        Waypoint *last = (Waypoint *)getFromBack(seg_last->waypoints);

        if (first==NULL || last==NULL)
            continue;

        if (wptHaversine(first, &source) < delta && wptHaversine(last, &dest) < delta)
            insertBack(new_list, (void *)trk);
    }

    if (getLength(new_list) == 0)
        return NULL;

    return new_list;
}

char *trackToJSON(const Track *tr)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (tr == NULL)
        return json;

    strcpy(json, "");

    json = (char *)realloc(json, 40 + strlen(tr->name) + 5);
    sprintf(json, "{\"name\":\"%s\",\"len\":%.1f,\"loop\":%s}", strcmp(tr->name, "") == 0 ? "None" : tr->name, round10(getTrackLen(tr)), isLoopTrack(tr, 10) ? "true" : "false");
    return json;
}

char *routeToJSON(const Route *rt)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (rt == NULL)
        return json;

    json = (char *)realloc(json, 60 + strlen(rt->name) + 5);
    sprintf(json, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", strcmp(rt->name, "") == 0 ? "None" : rt->name, rt->waypoints->length, round10(getRouteLen(rt)), isLoopRoute(rt, 10) ? "true" : "false");
    return json;
}

char *routeListToJSON(const List *list)
{
    const char *default_str = "[]";

    char *str = malloc(strlen(default_str) + 1);
    strcpy(str, default_str);

    if (list == NULL || list->head == NULL)
        return str;

    strcpy(str, "[");

    void *element, *element_next;
    ListIterator iter = createIterator((List *)list);
    ListIterator iter_next = createIterator((List *)list);
    element_next = nextElement(&iter_next);
    while ((element = nextElement(&iter)) != NULL)
    {
        char *json = routeToJSON((Route *)element);

        int new_len = strlen(json) + strlen(str) + 4;
        str = (char *)realloc(str, new_len);

        if ((element_next = nextElement(&iter_next)) == NULL)
        {
            strcat(str, json);
        }
        else
        {
            strcat(str, json);
            strcat(str, ",");
        }
        free(json);
    }

    strcat(str, "]");

    return str;
}

char *trackListToJSON(const List *list)
{
    const char *default_str = "[]";

    char *str = malloc(strlen(default_str) + 1);
    strcpy(str, default_str);

    if (list == NULL || list->head == NULL)
        return str;

    strcpy(str, "[");

    void *element, *element_next;
    ListIterator iter = createIterator((List *)list);
    ListIterator iter_next = createIterator((List *)list);
    element_next = nextElement(&iter_next);
    while ((element = nextElement(&iter)) != NULL)
    {
        char *json = trackToJSON((Track *)element);

        int new_len = strlen(json) + strlen(str) + 4;
        str = (char *)realloc(str, new_len);

        if ((element_next = nextElement(&iter_next)) == NULL)
        {
            strcat(str, json);
        }
        else
        {
            strcat(str, json);
            strcat(str, ",");
        }
        free(json);
    }

    strcat(str, "]");

    return str;
}

char *GPXtoJSON(const GPXdoc *gpx)
{
    const char *default_str = "{}";

    char *json = malloc(strlen(default_str) + 1);
    strcpy(json, default_str);

    if (gpx == NULL)
        return json;

    strcpy(json, "");

    int n_waypoints = getLength(gpx->waypoints);
    int n_routes = getLength(gpx->routes);
    int n_tracks = getLength(gpx->tracks);

    json = (char *)realloc(json, 80 + strlen(gpx->creator) + 5);
    sprintf(json, "{\"version\":%.1lf,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, n_waypoints, n_routes, n_tracks);

    return json;
}

void addWaypoint(Route *rt, Waypoint *pt)
{
    if (rt == NULL || pt == NULL)
        return;
    insertBack(rt->waypoints, (void *)pt);
}

void addRoute(GPXdoc *doc, Route *rt)
{
    if (rt == NULL || doc == NULL)
        return;

    insertBack(doc->routes, (void *)rt);
}

GPXdoc *JSONtoGPX(const char *gpxString)
{
    if (gpxString == NULL || strcmp("{}", gpxString) == 0)
        return NULL;

    char version[100], creator[100];
    sscanf(gpxString, "{\"version\":%[0-9.],\"creator\":\"%100[0-9a-zA-Z.,- ]\"", version, creator);
    float fl_version = atof(version);

    GPXdoc *doc = (GPXdoc *)malloc(sizeof(GPXdoc));

    doc->version = fl_version;

    doc->creator = (char *)malloc(strlen(creator) + 1);
    strcpy(doc->creator, creator);

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");

    doc->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    doc->routes = initializeList(routeToString, deleteRoute, compareRoutes);
    doc->tracks = initializeList(trackToString, deleteTrack, compareTracks);

    return doc;
}

Waypoint *JSONtoWaypoint(const char *gpxString)
{
    if (gpxString == NULL)
        return NULL;

    char lat[100], lon[100];
    sscanf(gpxString, "{\"lat\":%[0-9.-],\"lon\":%[0-9.-]}", lat, lon);

    Waypoint *wpt = (Waypoint *)malloc(sizeof(Waypoint));

    wpt->latitude = atof(lat);
    wpt->longitude = atof(lon);
    wpt->name = (char *)malloc(5);
    strcpy(wpt->name, "");

    wpt->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
    return wpt;
}

Route *JSONtoRoute(const char *gpxString)
{
    if (gpxString == NULL)
        return NULL;

    char name[256];
    sscanf(gpxString, "{\"name\":\"%[A-z ]\"}", name);

    Route *rte = (Route *)malloc(sizeof(Route));

    rte->name = (char *)malloc(strlen(name) + 1);
    strcpy(rte->name, name);

    rte->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    rte->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    return rte;
}