/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "common/map/MapEngine.h"
#include "common/map/GeocodingMapEngine.h"
#include "common/map/GoogleMapEngine.h"
#include "common/map/ESRIMapEngine.h"
#include "common/map/OSMMapEngine.h"
#include "common/RhodesApp.h"
#include "rubyext/GeoLocation.h"

#include <algorithm>

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "MapEngine"

namespace rho { namespace common { namespace map {

MapProvider &MapProvider::getInstance()
{
    static MapProvider instance;
    return instance;
}

MapProvider::MapProvider()
{
    // Initialize predefined engines
    //registerMapEngine("google", new GoogleMapEngine());
    registerMapEngine("esri", new ESRIMapEngine());
    registerMapEngine("rhogoogle", new GoogleMapEngine());
    registerMapEngine("osm", new OSMMapEngine());
}

void MapProvider::registerMapEngine(String const &id, IMapEngine *engine)
{
    RAWTRACE2("Register map engine: id=%s, engine=%p", id.c_str(), engine);
    IMapEngine *old = m_engines.get(id);
    if (old)
        delete old;
    m_engines.put(id, engine);
}

void MapProvider::unregisterMapEngine(String const &id)
{
    RAWTRACE1("Unregister map engine: id=%s", id.c_str());
    IMapEngine *engine = m_engines.get(id);
    if (engine)
        delete engine;
    m_engines.remove(id);
}

bool MapProvider::isRegisteredMapEngine(String const &id) {
    IMapEngine *engine = m_engines.get(id);
    if (!engine)
        return false;
    return true;
}


IMapView *MapProvider::createMapView(String const &id, IDrawingDevice *device)
{
    IMapEngine *engine = m_engines.get(id);
    if (!engine)
        return 0;
    IMapView *view = engine->createMapView(device);
    if (!view)
        return 0;
    m_cache.put(view, engine);
    return view;
}

void MapProvider::destroyMapView(IMapView *view)
{
    if (!view)
        return;
    IMapEngine *engine = m_cache.get(view);
    if (!engine)
        return;
    m_cache.remove(view);
    engine->destroyMapView(view);
}

String Annotation::make_address(double latitude, double longitude)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "%.5lf,%.5lf", latitude, longitude);
    return buf;
}

String Annotation::make_url(const String& base_url, bool pass_location, double lat, double lon)
{
    if (!pass_location)
        return base_url;

    String base(base_url, 0, base_url.find('?'));
    String query(base_url, base.length(), base_url.find('#'));
    String anchor(base_url, base.length() + query.length());
    char buf[64];

    if (query.empty())
        snprintf(buf, 64, "?latitude=%.5lf&longitude=%.5lf", lat, lon);
    else
        snprintf(buf, 64, "&latitude=%.5lf&longitude=%.5lf", lat, lon);

    return base + query + buf + anchor;
}

    class EmptyDrawingDevice : public IDrawingDevice
    {
    public:
        virtual ~EmptyDrawingDevice(){}
        
        IDrawingImage* createImage(String const &path, bool useAlpha) {return NULL;}
        IDrawingImage* createImage(void const *p, size_t s, bool useAlpha) {return NULL;}
        IDrawingImage* createImageEx(void const *p, size_t s, int x, int y, int w, int h, bool useAlpha) {return NULL;}
        IDrawingImage* cloneImage(IDrawingImage *image) {return NULL;}
        void destroyImage(IDrawingImage* image){}
        
        IDrawingImage* createCalloutImage(String const &title, String const &subtitle, String const& url, int* x_offset, int* y_offset) {return NULL;}
        
        void requestRedraw() {}
    };
    
} // namespace map
} // namespace common
} // namespace rho

namespace rhomap = rho::common::map;


bool rho_map_check_param(rho_param *p) {
    if (!p || p->type != RHO_PARAM_HASH)
        rho_ruby_raise_argerror("Wrong input parameter (expect Hash)");

    rho_param *provider = NULL;
    for (int i = 0, lim = p->v.hash->size; i < lim; ++i)
    {
        char *name = p->v.hash->name[i];
        rho_param *value = p->v.hash->value[i];
        if (!name || !value)
            continue;

        if (strcasecmp(name, "provider") == 0)
            provider = value;
    }

    std::string providerId = "google";
    if (provider)
    {
        if (provider->type != RHO_PARAM_STRING)
            rho_ruby_raise_argerror("Wrong 'provider' value (expect String)");
        providerId = provider->v.string;
    }
    std::transform(providerId.begin(), providerId.end(), providerId.begin(), &::tolower);
	return RHOMAPPROVIDER().isRegisteredMapEngine(providerId);
}


rhomap::IMapView *rho_map_create(rho_param *p, rhomap::IDrawingDevice *device, int width, int height)
{
    if (!p || p->type != RHO_PARAM_HASH)
        rho_ruby_raise_argerror("Wrong input parameter (expect Hash)");

    rho_param *provider = NULL;
    rho_param *settings = NULL;
    rho_param *annotations = NULL;
    for (int i = 0, lim = p->v.hash->size; i < lim; ++i)
    {
        char *name = p->v.hash->name[i];
        rho_param *value = p->v.hash->value[i];
        if (!name || !value)
            continue;

        if (strcasecmp(name, "provider") == 0)
            provider = value;
        else if (strcasecmp(name, "settings") == 0)
            settings = value;
        else if (strcasecmp(name, "annotations") == 0)
            annotations = value;
    }

    std::string providerId = "google";
    if (provider)
    {
        if (provider->type != RHO_PARAM_STRING)
            rho_ruby_raise_argerror("Wrong 'provider' value (expect String)");
        providerId = provider->v.string;
    }
    std::transform(providerId.begin(), providerId.end(), providerId.begin(), &::tolower);

    const char *map_type = "roadmap";
    bool use_center_radius = false;
    double latitude = 0;
    double longitude = 0;
    double latitudeSpan = 0;
    double longitudeSpan = 0;
    char *center = NULL;
    double radius = 0;
    bool zoom_enabled = true;
    bool scroll_enabled = true;
    bool shows_user_location = true;

    bool region_setted = false;
    
    if (settings)
    {
        if (settings->type != RHO_PARAM_HASH)
            rho_ruby_raise_argerror("Wrong 'settings' value (expect Hash)");

        for (int i = 0, lim = settings->v.hash->size; i < lim; ++i)
        {
            const char *name = settings->v.hash->name[i];
            rho_param *value = settings->v.hash->value[i];
            if (!name || !value)
                continue;

            RAWTRACE1("Processing map parameter: %s", name);

            if (strcasecmp(name, "map_type") == 0)
            {
                if (value->type != RHO_PARAM_STRING)
                    rho_ruby_raise_argerror("Wrong 'map_type' value (expect String)");
                map_type = value->v.string;
                if (strcasecmp(map_type,"standard") == 0) {
                    map_type = "roadmap";
                }
            }
            else if (strcasecmp(name, "region") == 0)
            {
                if (value->type == RHO_PARAM_ARRAY)
                {
                    if (value->v.array->size != 4)
                        rho_ruby_raise_argerror("'region' array should contain exactly 4 items");

                    rho_param *lat = value->v.array->value[0];
                    if (!lat) continue;
                    rho_param *lon = value->v.array->value[1];
                    if (!lon) continue;
                    rho_param *latSpan = value->v.array->value[2];
                    if (!latSpan) continue;
                    rho_param *lonSpan = value->v.array->value[3];
                    if (!lonSpan) continue;

                    latitude = lat->type == RHO_PARAM_STRING ? strtod(lat->v.string, NULL) : 0;
                    longitude = lon->type == RHO_PARAM_STRING ? strtod(lon->v.string, NULL) : 0;
                    latitudeSpan = latSpan->type == RHO_PARAM_STRING ? strtod(latSpan->v.string, NULL) : 0;
                    longitudeSpan = lonSpan->type == RHO_PARAM_STRING ? strtod(lonSpan->v.string, NULL) : 0;

                    region_setted = true;
                    
                    use_center_radius = false;
                }
                else if (value->type == RHO_PARAM_HASH)
                {
                    for (int j = 0, limm = value->v.hash->size; j < limm; ++j)
                    {
                        char *rname = value->v.hash->name[j];
                        rho_param *rvalue = value->v.hash->value[j];
                        if (!rname || !rvalue)
                            continue;
                        if (strcasecmp(rname, "center") == 0)
                        {
                            if (rvalue->type != RHO_PARAM_STRING)
                                rho_ruby_raise_argerror("Wrong 'center' value (expect String)");
                            center = rvalue->v.string;
                        }
                        else if (strcasecmp(rname, "radius") == 0)
                        {
                            if (rvalue->type != RHO_PARAM_STRING)
                                rho_ruby_raise_argerror("Wrong 'radius' value (expect String or Float)");
                            radius = strtod(rvalue->v.string, NULL);
                        }
                    }

                    use_center_radius = true;
                }
                else
                    rho_ruby_raise_argerror("Wrong 'region' value (expect Array or Hash");
            }
            else if (strcasecmp(name, "zoom_enabled") == 0)
            {
                if (value->type != RHO_PARAM_STRING)
                    rho_ruby_raise_argerror("Wrong 'zoom_enabled' value (expect boolean)");
                zoom_enabled = strcasecmp(value->v.string, "true") == 0;
            }
            else if (strcasecmp(name, "scroll_enabled") == 0)
            {
                if (value->type != RHO_PARAM_STRING)
                    rho_ruby_raise_argerror("Wrong 'scroll_enabled' value (expect boolean)");
                scroll_enabled = strcasecmp(value->v.string, "true") == 0;
            }
            else if (strcasecmp(name, "shows_user_location") == 0)
            {
                if (value->type != RHO_PARAM_STRING)
                    rho_ruby_raise_argerror("Wrong 'shows_user_location' value (expect boolean)");
                shows_user_location = strcasecmp(value->v.string, "true") == 0;
            }
        }
    }

    typedef rhomap::Annotation ann_t;
    typedef std::vector<ann_t> ann_list_t;
    ann_list_t ann_list;
    if (annotations)
    {
        if (annotations->type != RHO_PARAM_ARRAY)
            rho_ruby_raise_argerror("Wrong 'annotations' value (expect Array)");
        for (int i = 0, lim = annotations->v.array->size; i < lim; ++i)
        {
            rho_param *ann_params = annotations->v.array->value[i];
            if (!ann_params)
                continue;
            if (ann_params->type != RHO_PARAM_HASH)
                rho_ruby_raise_argerror("Wrong annotation value found (expect Hash)");

            bool latitude_set = false;
            double cur_lat = 0;
            bool longitude_set = false;
            double cur_lon = 0;
            char const *address = "";
            char const *title = "";
            char const *subtitle = "";
            char const *url = "";
            char const *image = NULL;
            int x_off = 0;
            int y_off = 0;
            bool pass_location = false;

            for (int j = 0, limm = ann_params->v.hash->size; j < limm; ++j)
            {
                char *name = ann_params->v.hash->name[j];
                rho_param *value = ann_params->v.hash->value[j];
                if (!name || !value)
                    continue;

                if (value->type != RHO_PARAM_STRING)
                    rho_ruby_raise_argerror("Wrong annotation value");

                char *v = value->v.string;
                if (strcasecmp(name, "latitude") == 0)
                {
                    cur_lat = strtod(v, NULL);
                    latitude_set = true;
                }
                else if (strcasecmp(name, "longitude") == 0)
                {
                    cur_lon = strtod(v, NULL);
                    longitude_set = true;
                }
                else if (strcasecmp(name, "street_address") == 0)
                    address = v;
                else if (strcasecmp(name, "title") == 0)
                    title = v;
                else if (strcasecmp(name, "subtitle") == 0)
                    subtitle = v;
                else if (strcasecmp(name, "url") == 0)
                    url = v;
                else if (strcasecmp(name, "image") == 0)
                    image = v;
                else if (strcasecmp(name, "image_x_offset") == 0) {
                    x_off = (int)strtod(v, NULL);
                }
                else if (strcasecmp(name, "image_y_offset") == 0) {
                    y_off = (int)strtod(v, NULL);
                }
                else if (strcasecmp(name, "pass_location") == 0) {
                    pass_location = strcasecmp(value->v.string, "true") == 0 || strcasecmp(value->v.string, "1") == 0;
                }
             }

            if (latitude_set && longitude_set) {
                ann_t ann(title, subtitle, cur_lat, cur_lon, url, pass_location);
                if (image != NULL) {
                    ann.setImageFileName(image, x_off, y_off);
                }
                ann_list.push_back(ann);
            }
            else {
                ann_t ann(title, subtitle, address, url, pass_location);
                if (image != NULL) {
                    ann.setImageFileName(image, x_off, y_off);
                }
                ann_list.push_back(ann);
            }
        }
    }

    rhomap::IMapView *mapview = RHOMAPPROVIDER().createMapView(providerId, device);
    if (!mapview)
        return NULL;

    mapview->setSize(width, height);

    if (map_type)
        mapview->setMapType(map_type);

    if (use_center_radius)
    {
        mapview->moveTo(center);
        mapview->setZoom(radius, radius);
    }
    else
    {
        mapview->moveTo(latitude, longitude);
        mapview->setZoom(latitudeSpan, longitudeSpan);
    }

    mapview->setZoomEnabled(zoom_enabled);
    mapview->setScrollEnabled(scroll_enabled);

    for (ann_list_t::iterator it = ann_list.begin(), lim = ann_list.end(); it != lim; ++it)
        mapview->addAnnotation(*it);

    if (shows_user_location)
    {
        mapview->setMyLocation(rho_geo_latitude(), rho_geo_longitude());
    }

    return mapview;
}

void rho_map_destroy(rho::common::map::IMapView *mapview)
{
    RHOMAPPROVIDER().destroyMapView(mapview);
}

static void callPreloadCallback(const char* callback, const char* status, int progress) {
    char body[2048];

    snprintf(body, sizeof(body), "&rho_callback=1&status=%s&progress=%d", status, progress);
    rho_net_request_with_data(RHODESAPP().canonicalizeRhoUrl(callback).c_str(), body);    
}

class CheckProgress : public rho::common::CRhoThread
{
    
public:
    CheckProgress(rhomap::IMapView *view, rho::String callback, int initial_count) {
        m_mapview = view;
        m_callback = callback;
        m_initial_count = initial_count;
    }
    virtual ~CheckProgress() {
        RHOMAPPROVIDER().destroyMapView(m_mapview);
    }
    
    
    virtual void run()
    {
        sleep(100);
        int count = m_mapview->getCountOfTilesToDownload();
        while (count > 0) {
            callPreloadCallback(m_callback.c_str(), "PROGRESS", ((m_initial_count - count)*100)/m_initial_count);
            sleep(100);
            count = m_mapview->getCountOfTilesToDownload();
        }
        callPreloadCallback(m_callback.c_str(), "DONE", 100);
        stop(500);
    }
    
private:
    rhomap::IMapView *m_mapview;
    rho::String m_callback;
    int m_initial_count;
};


int mapview_preload_map_tiles(rho_param* p, const char* callback)
{
    
    if (p == NULL) {
        return 0;
    }
    

    if (p->type != RHO_PARAM_HASH) {
        RAWLOG_ERROR("preload_map_tiles: first params must be HASH!");
        return 0;
        
    }
    
    char* engine = NULL;
    char* map_type = NULL;
    double top_latitude = 0;
    double left_longitude = 0;
    double bottom_latitude = 0;
    double right_longitude = 0;
    int min_zoom = 0;
    int max_zoom = 0;
    
    bool top_latitude_setted = false;
    bool left_longitude_setted = false;
    bool bottom_latitude_setted = false;
    bool right_longitude_setted = false;
    bool min_zoom_setted = false;
    bool max_zoom_setted = false;
    

    for (int j = 0, limm = p->v.hash->size; j < limm; ++j) {
        char *name = p->v.hash->name[j];
        rho_param *value = p->v.hash->value[j];
        if (!name || !value)
            continue;
        if (value->type != RHO_PARAM_STRING)
            continue;
        char *v = value->v.string;
        
        if (strcasecmp(name, "engine") == 0) {
            engine = v;
        }
        else if (strcasecmp(name, "map_type") == 0) {
            map_type = v;
        }
        else if (strcasecmp(name, "top_latitude") == 0) {
            top_latitude = strtod(v, NULL);
            top_latitude_setted = true;
        }
        else if (strcasecmp(name, "left_longitude") == 0) {
            left_longitude = strtod(v, NULL);
            left_longitude_setted = true;
        }
        else if (strcasecmp(name, "bottom_latitude") == 0) {
            bottom_latitude = strtod(v, NULL);
            bottom_latitude_setted = true;
        }
        else if (strcasecmp(name, "right_longitude") == 0) {
            right_longitude = strtod(v, NULL);
            right_longitude_setted = true;
        }
        else if (strcasecmp(name, "min_zoom") == 0) {
            min_zoom = (int)strtod(v, NULL);
            min_zoom_setted = true;
        }
        else if (strcasecmp(name, "max_zoom") == 0) {
            max_zoom = (int)strtod(v, NULL);
            max_zoom_setted = true;
        }
    }
    
    const char* s_google = "google";
    const char* s_type = "roadmap";

    if (engine == NULL) {
        engine = (char*)s_google;
    }

    if (map_type == NULL) {
        map_type = (char*)s_type;
    }
    
    
    rhomap::EmptyDrawingDevice empty_device;
    std::string providerId = engine;
    std::transform(providerId.begin(), providerId.end(), providerId.begin(), &::tolower);    
    rhomap::IMapView *mapview = RHOMAPPROVIDER().createMapView(providerId, &empty_device);
    
    if (mapview == NULL) {
        RAWLOG_ERROR1("Can not create MapView for provider=%s", engine);
        return 0;
    }
    
    if (!( top_latitude_setted && left_longitude_setted && bottom_latitude_setted && right_longitude_setted && min_zoom_setted && max_zoom_setted )) {
        RAWLOG_ERROR("Invalid parameters in preload_map_tiles()");
        return 0;
    }
    
    mapview->setMapType(map_type);
    mapview->set_file_caching_enable(true);
    
    int count = mapview->preloadMapTiles(top_latitude, left_longitude, bottom_latitude, right_longitude, min_zoom, max_zoom);
    
    CheckProgress* cp = new CheckProgress(mapview, callback, count);
    
    cp->start(rho::common::IRhoRunnable::epNormal);
    
    return count; 
}

