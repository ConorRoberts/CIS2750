#include "LinkedListAPI.h"
#include "GPXParser.h"
#include "GPXHelpers.h"

int xmlNameCompare(xmlNode *xml_node, char *pattern)
{
    if (xml_node == NULL)
        return 0;
    return (strcmp((char *)xml_node->name, pattern) == 0 ? 1 : 0);
}

void checkAndAssignWptName(xmlNode *xml_node, Waypoint *wpt)
{
    if (xml_node->children == NULL)
    {
        wpt->name = malloc(5);
        strcpy(wpt->name, "");
        return;
    }
    else if (xmlNameCompare(xml_node->children->next, "name"))
    {
        char *tmp = (char *)xmlNodeGetContent(xml_node->children->next);
        wpt->name = malloc(strlen(tmp) + 1);
        strcpy(wpt->name, tmp);
        free(tmp);
        return;
    }
    wpt->name = malloc(5);
    strcpy(wpt->name, "");
}
void checkAndAssignRteName(xmlNode *xml_node, Route *rte)
{
    if (xml_node->children == NULL)
    {
        rte->name = malloc(5);
        strcpy(rte->name, "");
        return;
    }
    else if (xmlNameCompare(xml_node->children->next, "name"))
    {
        char *tmp = (char *)xmlNodeGetContent(xml_node->children->next);
        rte->name = malloc(strlen(tmp) + 1);
        strcpy(rte->name, tmp);
        free(tmp);
        return;
    }
    rte->name = malloc(5);
    strcpy(rte->name, "");
}
void checkAndAssignTrkName(xmlNode *xml_node, Track *trk)
{
    if (xml_node->children == NULL)
    {
        trk->name = malloc(5);
        strcpy(trk->name, "");
        return;
    }
    else if (xmlNameCompare(xml_node->children->next, "name"))
    {
        char *tmp = (char *)xmlNodeGetContent(xml_node->children->next);
        trk->name = malloc(strlen(tmp) + 1);
        strcpy(trk->name, tmp);
        free(tmp);
        return;
    }
    trk->name = malloc(5);
    strcpy(trk->name, "");
}

void assignLatLon(xmlNode *xml_node, Waypoint *wpt)
{
    // Getting lat/lon
    char *xml_longitude_tmp = (char *)xmlGetProp(xml_node, (xmlChar *)"lon");
    char *xml_latitude_tmp = (char *)xmlGetProp(xml_node, (xmlChar *)"lat");
    wpt->longitude = atof(xml_longitude_tmp);
    wpt->latitude = atof(xml_latitude_tmp);
    free(xml_latitude_tmp);
    free(xml_longitude_tmp);
}

void addMiscGPXData(xmlNode *xml_node, List *gpxList)
{
    // "text" elements are worthless and should be discarded
    // Name is already used
    if (xml_node != NULL)
    {
        if (!xmlNameCompare(xml_node, "text") && !xmlNameCompare(xml_node, "name") && !xmlNameCompare(xml_node, "wpt") && !xmlNameCompare(xml_node, "rte") && !xmlNameCompare(xml_node, "rtept") && !xmlNameCompare(xml_node, "trk") && !xmlNameCompare(xml_node, "trkseg") && !xmlNameCompare(xml_node, "trpkt"))
        {
            // Allocate for new GPXData
            char *tmp_node_name = (char *)xml_node->name;
            char *tmp_value = (char *)xmlNodeGetContent(xml_node);
            GPXData *tmp_gpx = (GPXData *)calloc(1, sizeof(GPXData) + 256 + strlen(tmp_value));

            // Get name
            strcpy(tmp_gpx->name, tmp_node_name);

            // Get value
            strcpy(tmp_gpx->value, tmp_value);
            free(tmp_value);

            insertBack(gpxList, (void *)tmp_gpx);
        }
        addMiscGPXData(xml_node->next, gpxList);
    }
}

void handleMiscGPXData(xmlNode *xml_node, List *otherData)
{
    if (xml_node->children != NULL)
    {
        addMiscGPXData(xml_node->children, otherData);
    }
}

Waypoint *nodeToWaypoint(xmlNode *xml_node)
{
    Waypoint *tmp_wpt = (Waypoint *)calloc(1, sizeof(Waypoint));
    checkAndAssignWptName(xml_node, tmp_wpt);
    assignLatLon(xml_node, tmp_wpt);

    tmp_wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    handleMiscGPXData(xml_node, tmp_wpt->otherData);

    return tmp_wpt;
}

Route *nodeToRoute(xmlNode *xml_node)
{
    Route *tmp_rte = (Route *)calloc(1, sizeof(Route));
    tmp_rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    checkAndAssignRteName(xml_node, tmp_rte);

    // This block is called regardless because of garbage "text" elements
    // Should probably fix but w/e
    tmp_rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    handleMiscGPXData(xml_node, tmp_rte->otherData);

    return tmp_rte;
}
Track *nodeToTrack(xmlNode *xml_node)
{
    Track *tmp_trk = (Track *)calloc(1, sizeof(Track));
    tmp_trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    checkAndAssignTrkName(xml_node, tmp_trk);

    xmlNode *xml_child = NULL;
    for (xml_child = xml_node->children; xml_child != NULL; xml_child = xml_child->next)
    {
        // We are looking at children of the track

        // Check if it's not a track segment; continue
        if (!xmlNameCompare(xml_child, "trkseg"))
            continue;

        // Make a new track segment
        TrackSegment *tmp_trkseg = (TrackSegment *)malloc(sizeof(TrackSegment));
        tmp_trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

        xmlNode *xml_trkpt = NULL;
        for (xml_trkpt = xml_child->children; xml_trkpt != NULL; xml_trkpt = xml_trkpt->next)
        {
            // We are looking at waypoints
            if (xmlNameCompare(xml_trkpt, "text"))
                continue;

            // Make a new waypoint
            Waypoint *wpt = nodeToWaypoint(xml_trkpt);
            insertBack(tmp_trkseg->waypoints, wpt);
        }

        insertBack(tmp_trk->segments, tmp_trkseg);
    }

    // This block is called regardless because of garbage "text" elements
    // Should probably fix but w/e
    tmp_trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    handleMiscGPXData(xml_node, tmp_trk->otherData);

    return tmp_trk;
}

void populateList(xmlNode *parent, List *waypoints, List *routes, List *tracks)
{
    if (parent == NULL)
        return;

    if (xmlNameCompare(parent, "wpt") || xmlNameCompare(parent, "rtept"))
    {
        // Create and push new wpt to waypoints
        Waypoint *tmp_wpt = nodeToWaypoint(parent);
        insertBack(waypoints, (void *)tmp_wpt);
    }
    else if (xmlNameCompare(parent, "rte") || xmlNameCompare(parent, "trkseg"))
    {
        // Create and push new rte to routes
        Route *tmp_rte = nodeToRoute(parent);
        insertBack(routes, tmp_rte);
        if (parent->children->next != NULL)
        {
            // my brain is throbbing. this was a great way to do this
            populateList(parent->children->next, tmp_rte->waypoints, routes, tracks);
        }
    }
    else if (xmlNameCompare(parent, "trk"))
    {
        // Create and push new trk to tracks
        // Okay this is really stupid LOL. Why is TrackSegment a thing
        // Why can't it be called a Route??? What is the problem??
        Track *tmp_trk = nodeToTrack(parent);
        insertBack(tracks, tmp_trk);
    }

    // gg go next
    if (parent->next != NULL)
    {
        populateList(parent->next, waypoints, routes, tracks);
    }
}

void populateWptNode(Waypoint *wpt, xmlNode *parent, xmlNsPtr ns)
{
    // There's just no way you put in numbers with more than 100 decimal places
    char lat[100], lon[100];

    // Convert float to string
    sprintf(lat, "%f", wpt->latitude);
    sprintf(lon, "%f", wpt->longitude);

    // Add lat/lon properties
    xmlNewProp(parent, BAD_CAST "lat", BAD_CAST lat);
    xmlNewProp(parent, BAD_CAST "lon", BAD_CAST lon);

    populateNodeOther(wpt->otherData, parent, ns);

    // Check if wpt has a name
    if (strcmp(wpt->name, "") != 0)
        xmlNewChild(parent, ns, BAD_CAST "name", BAD_CAST wpt->name);
}

void populateNodeOther(List *other, xmlNode *parent, xmlNsPtr ns)
{
    void *element;
    ListIterator iter = createIterator(other);

    while ((element = nextElement(&iter)) != NULL)
    {
        GPXData *gpx = (GPXData *)element;
        xmlNewChild(parent, ns, BAD_CAST gpx->name, BAD_CAST gpx->value);
    }
}

void populateRteNode(Route *rte, xmlNode *parent, xmlNsPtr ns)
{
    void *element;
    ListIterator iter = createIterator(rte->waypoints);
    xmlNodePtr node = NULL;

    // Check if rte has a name
    if (strcmp(rte->name, "") != 0)
        node = xmlNewChild(parent, ns, BAD_CAST "name", BAD_CAST rte->name);

    populateNodeOther(rte->otherData, parent, ns);
    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        node = xmlNewChild(parent, ns, BAD_CAST "rtept", NULL);

        populateWptNode(wpt, node, ns);
    }
}
void populateSegNode(TrackSegment *seg, xmlNode *parent, xmlNsPtr ns)
{
    void *element;
    ListIterator iter = createIterator(seg->waypoints);
    xmlNodePtr node = NULL;

    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        node = xmlNewChild(parent, ns, BAD_CAST "trkpt", NULL);
        populateWptNode(wpt, node, ns);
    }
}
void populateTrkNode(Track *trk, xmlNode *parent, xmlNsPtr ns)
{
    void *element;
    xmlNodePtr node = NULL;

    if (strcmp(trk->name, "") != 0)
        node = xmlNewChild(parent, ns, BAD_CAST "name", BAD_CAST trk->name);

    populateNodeOther(trk->otherData, parent, ns);

    ListIterator iter = createIterator(trk->segments);
    while ((element = nextElement(&iter)) != NULL)
    {
        TrackSegment *seg = (TrackSegment *)element;
        node = xmlNewChild(parent, ns, BAD_CAST "trkseg", NULL);
        populateSegNode(seg, node, ns);
    }
}

xmlDocPtr gpxToXmlDoc(GPXdoc *doc)
{
    xmlDocPtr xml_doc = NULL;
    xmlNodePtr root_node = NULL;

    xml_doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");

    xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST doc->namespace, NULL);
    xmlSetNs(root_node, ns);

    xmlDocSetRootElement(xml_doc, root_node);

    // Convert version to string
    char ver[100];

    // Convert float to string
    sprintf(ver, "%.1f", doc->version);
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST ver);

    // Write properties to GPX root
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST doc->creator);

    ListIterator iter;
    void *element;

    // Iterate through waypoints
    iter = createIterator(doc->waypoints);
    while ((element = nextElement(&iter)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)element;
        xmlNodePtr wpt_node = xmlNewChild(root_node, ns, BAD_CAST "wpt", NULL);
        populateWptNode(wpt, wpt_node, ns);
    }

    // Iterate through routes
    iter = createIterator(doc->routes);
    while ((element = nextElement(&iter)) != NULL)
    {
        Route *rte = (Route *)element;
        xmlNodePtr rte_node = xmlNewChild(root_node, ns, BAD_CAST "rte", NULL);
        populateRteNode(rte, rte_node, ns);
    }

    // Iterate through tracks
    iter = createIterator(doc->tracks);
    while ((element = nextElement(&iter)) != NULL)
    {
        Track *trk = (Track *)element;
        xmlNodePtr trk_node = xmlNewChild(root_node, ns, BAD_CAST "trk", NULL);
        populateTrkNode(trk, trk_node, ns);
    }

    return xml_doc;
}

void err(void *ctx, const char *msg, ...)
{
    return;
}
void warn(void *ctx, const char *msg, ...)
{
    return;
}

bool validateXmlDoc(xmlDocPtr doc, char *gpxSchemaFile)
{
    if (doc == NULL)
        return NULL;

    xmlSchemaParserCtxtPtr ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, ctxt);

    xmlSchemaPtr schema = xmlSchemaParse(ctxt);

    // Check that schema is valid
    if (schema)
    {
        xmlSchemaFreeParserCtxt(ctxt);

        xmlSchemaValidCtxtPtr ctxt_valid = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt_valid, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, ctxt_valid);

        int val = xmlSchemaValidateDoc(ctxt_valid, doc);

        xmlSchemaFreeValidCtxt(ctxt_valid);

        xmlSchemaFree(schema);
        xmlSchemaCleanupTypes();

        if (val == 0)
            return true;
    }

    return false;
}

bool validateGPXData(void *data)
{
    GPXData *gpx = (GPXData *)data;
    bool ret = true;
    if (strcmp(gpx->name, "") == 0 || strcmp(gpx->value, "") == 0)
        ret = false;

    return ret;
}
bool validateWaypoint(void *data)
{
    Waypoint *wpt = (Waypoint *)data;
    bool ret = true;
    // Waypoint
    // Name must not be null, may be empty string
    // lat,lon, must both be initialized (aka not NULL)
    // OtherData must not be null
    if (wpt->name == NULL || wpt->otherData == NULL)
        ret = false;

    return ret;
}
bool validateRoute(void *data)
{
    // Name must not be null, may be empty string
    // Waypoints must not be null
    // OtherData must not be null
    Route *rte = (Route *)data;
    bool ret = true;

    if (rte->name == NULL || rte->otherData == NULL || rte->waypoints == NULL)
        ret = false;

    ret = validateList(rte->otherData, validateGPXData);
    ret = validateList(rte->waypoints, validateWaypoint);

    return ret;
}
bool validateTrackSegment(void *data)
{
    TrackSegment *seg = (TrackSegment *)data;
    // waypoints must not be null
    bool ret = true;
    if (seg->waypoints == NULL)
        ret = false;

    ret = validateList(seg->waypoints, validateWaypoint);

    return ret;
}
bool validateTrack(void *data)
{
    Track *trk = (Track *)data;
    // Segments must not be null
    // Name must not be null
    // OtherData must not be null
    bool ret = true;
    if (trk->name == NULL || trk->otherData == NULL || trk->segments == NULL)
        ret = false;

    validateList(trk->segments, validateTrackSegment);
    validateList(trk->otherData, validateGPXData);

    return ret;
}

bool validateList(List *list, bool (*validate)(void *))
{
    if (list == NULL)
        return false;

    void *element;
    ListIterator iter = createIterator(list);
    while ((element = nextElement(&iter)) != NULL)
        if (!validate(element))
            return false;

    return true;
}

float wptHaversine(Waypoint *wpt_1, Waypoint *wpt_2)
{
    if (wpt_1 == NULL || wpt_2 == NULL)
        return 0;

    const float R = 6371000;

    float ph_1 = toRadians(wpt_1->latitude);
    float ph_2 = toRadians(wpt_2->latitude);

    float dl_lat = toRadians(wpt_2->latitude - wpt_1->latitude);
    float dl_lon = toRadians(wpt_2->longitude - wpt_1->longitude);

    float a = pow(sin(dl_lat / 2), 2) + (cos(ph_1) * cos(ph_2) * pow(sin(dl_lon / 2), 2));

    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    float d = R * c;

    return d;
}

float toRadians(float f)
{
    return (f * (M_PI / 180));
}

bool withinDelta(float len, float target, float delta)
{
    return (abs(len - target) <= delta ? true : false);
}

void dummyDelete(void *data)
{
    return;
}

char *getFileTableData(char *fileName)
{
    GPXdoc *doc = createValidGPXdoc(fileName, "gpx.xsd");

    char *str = GPXtoJSON(doc == NULL ? NULL : doc);

    deleteGPXdoc(doc);
    return str;
}
char *getRoutes(char *fileName)
{
    GPXdoc *doc = createValidGPXdoc(fileName, "gpx.xsd");

    char *str = routeListToJSONBetter(doc == NULL ? NULL : doc->routes);

    deleteGPXdoc(doc);
    return str;
}
char *getTracks(char *fileName)
{
    GPXdoc *doc = createValidGPXdoc(fileName, "gpx.xsd");

    char *str = trackListToJSONBetter(doc == NULL ? NULL : doc->tracks);

    deleteGPXdoc(doc);
    return str;
}
char *getWaypoints(char *fileName)
{
    GPXdoc *doc = createValidGPXdoc(fileName, "gpx.xsd");

    char *str = waypointListToJSONBetter(doc == NULL ? NULL : doc->waypoints);

    deleteGPXdoc(doc);
    return str;
}

char *trackToJSONBetter(const Track *tr)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (tr == NULL)
        return json;

    strcpy(json, "");
    int n = 0;

    void *element_seg;
    ListIterator iter = createIterator(tr->segments);
    while ((element_seg = nextElement(&iter)) != NULL)
    {
        TrackSegment *seg = (TrackSegment *)element_seg;
        n += seg->waypoints->length;
    }
    char *other = gpxDataListToJSON(tr->otherData);

    json = (char *)realloc(json, 40 + strlen(tr->name) + strlen(other) + 25);
    sprintf(json, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s,\"otherData\":%s}", strcmp(tr->name, "") == 0 ? "None" : tr->name, n, round10(getTrackLen(tr)), isLoopTrack(tr, 10) ? "true" : "false", other);

    free(other);

    return json;
}

char *trackListToJSONBetter(const List *list)
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
        char *json = trackToJSONBetter((Track *)element);

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

char *routeToJSONBetter(const Route *rt)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (rt == NULL)
        return json;

    char *other = gpxDataListToJSON(rt->otherData);
    char *waypoints = waypointListToJSONBetter(rt->waypoints);

    json = (char *)realloc(json, 100 + strlen(rt->name) + strlen(other) + strlen(waypoints));
    sprintf(json, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s,\"waypoints\":%s,\"otherData\":%s}", strcmp(rt->name, "") == 0 ? "None" : rt->name, rt->waypoints->length, round10(getRouteLen(rt)), isLoopRoute(rt, 10) ? "true" : "false", waypoints, other);

    free(other);

    return json;
}

char *routeListToJSONBetter(const List *list)
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
        char *json = routeToJSONBetter((Route *)element);

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

char *gpxDataListToJSON(List *list)
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
        char *json = gpxDataToJSON((GPXData *)element);

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
char *gpxDataToJSON(GPXData *gpx)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (gpx == NULL)
        return json;

    strcpy(json, "");
    strtok(gpx->value, "\n");

    json = (char *)realloc(json, 40 + strlen(gpx->name) + strlen(gpx->value) + 5);

    sprintf(json, "{\"name\":\"%s\",\"value\":\"%s\"}", strcmp(gpx->name, "") == 0 ? "None" : gpx->name, strcmp(gpx->value, "") == 0 ? "None" : gpx->value);

    return json;
}

char *waypointToJSONBetter(const Waypoint *wpt)
{
    char *json = malloc(3);
    strcpy(json, "{}");
    if (wpt == NULL)
        return json;

    char *other = gpxDataListToJSON(wpt->otherData);

    json = (char *)realloc(json, 60 + strlen(wpt->name) + strlen(other) + 25);
    sprintf(json, "{\"name\":\"%s\",\"lat\":%f,\"lon\":%f,\"otherData\":%s}", strcmp(wpt->name, "") == 0 ? "None" : wpt->name, wpt->latitude, wpt->longitude, other);

    free(other);

    return json;
}

char *waypointListToJSONBetter(const List *list)
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
        char *json = waypointToJSONBetter((Waypoint *)element);

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

void renameComponent(char *fileName, char *newName, char *type, int index)
{
    // Need to change this to use indexes instead of names
    GPXdoc *gpx = createValidGPXdoc(fileName, "gpx.xsd");

    void *element;
    int i = 0;
    if (strcmp(type, "route") == 0)
    {
        if (index >= gpx->routes->length)
            return;
        ListIterator iter = createIterator(gpx->routes);
        while ((element = nextElement(&iter)) != NULL && i != index)
            i++;
        Route *rte = (Route *)element;
        strcpy(rte->name, newName);
    }
    else if (strcmp(type, "track") == 0)
    {
        if (index >= gpx->tracks->length)
            return;
        ListIterator iter = createIterator(gpx->tracks);
        while ((element = nextElement(&iter)) != NULL && i != index)
            i++;
        Track *trk = (Track *)element;
        strcpy(trk->name, newName);
    }

    xmlDoc *xml = gpxToXmlDoc(gpx);

    xmlSaveFormatFileEnc(fileName, xml, NULL, 1);

    xmlFreeDoc(xml);
    deleteGPXdoc(gpx);
}

int JSONtoGPXFile(char *fileName, char *str)
{
    GPXdoc *gpx = JSONtoGPX(str);

    if (!validateGPXDoc(gpx, "gpx.xsd"))
        return 1;

    writeGPXdoc(gpx, fileName);

    deleteGPXdoc(gpx);
    return 0;
}

void addRouteToFile(char *fileName, char *str)
{
    GPXdoc *gpx = createValidGPXdoc(fileName, "gpx.xsd");

    Route *rte = JSONtoRoute(str);

    insertBack(gpx->routes, (void *)rte);

    if (!validateGPXDoc(gpx, "gpx.xsd"))
        return;

    writeGPXdoc(gpx, fileName);

    deleteGPXdoc(gpx);
}
void addWaypointToRoute(char *fileName, char *str, int index)
{
    GPXdoc *gpx = createValidGPXdoc(fileName, "gpx.xsd");

    if (index >= gpx->routes->length)
        return;

    int i = 0;
    void *element;
    ListIterator iter = createIterator(gpx->routes);
    while ((element = nextElement(&iter)) != NULL && i != index)
        i++;

    Waypoint *wpt = JSONtoWaypoint(str);
    Route *rte = (Route *)element;
    insertBack(rte->waypoints, (void *)wpt);

    if (!validateGPXDoc(gpx, "gpx.xsd"))
        return;

    writeGPXdoc(gpx, fileName);

    deleteGPXdoc(gpx);
}

char *getPathsBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    GPXdoc *gpx = createValidGPXdoc(fileName, "gpx.xsd");

    List *routes = getRoutesBetween(gpx, sourceLat, sourceLong, destLat, destLong, delta);
    List *tracks = getTracksBetween(gpx, sourceLat, sourceLong, destLat, destLong, delta);

    char *str_routes = routeListToJSONBetter(routes);
    char *str_tracks = trackListToJSONBetter(tracks);

    char *str = malloc(strlen(str_routes) + strlen(str_tracks) + 50);
    sprintf(str, "{\"routes\":%s,\"tracks\":%s}", str_routes, str_tracks);
    free(str_routes);
    free(str_tracks);

    freeList(routes);
    freeList(tracks);
    deleteGPXdoc(gpx);

    return str;
}