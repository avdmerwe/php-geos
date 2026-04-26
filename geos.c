/***********************************************************************
 *
 *    GEOS - Geometry Engine Open Source
 *    http://trac.osgeo.org/geos
 *
 *    Copyright (C) 2010 Sandro Santilli <strk@kbt.io>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin St, Fifth Floor,
 *    Boston, MA  02110-1301  USA
 *
 ***********************************************************************/

/* PHP stuff */
#include "php.h"
#include "ext/standard/info.h" /* for php_info_... */
#include "Zend/zend_exceptions.h" /* for zend_throw_exception_object */

/* GEOS stuff */
#include "geos_c.h"

/* Own stuff */
#include "php_geos.h"
#include "arginfo.h"

static ZEND_DECLARE_MODULE_GLOBALS(geos);
static PHP_GINIT_FUNCTION(geos);

PHP_MINIT_FUNCTION(geos);
PHP_MSHUTDOWN_FUNCTION(geos);
PHP_RINIT_FUNCTION(geos);
PHP_RSHUTDOWN_FUNCTION(geos);
PHP_MINFO_FUNCTION(geos);
PHP_FUNCTION(GEOSVersion);
PHP_FUNCTION(GEOSPolygonize);
PHP_FUNCTION(GEOSLineMerge);
PHP_FUNCTION(GEOSPolygonizeValid);
PHP_FUNCTION(GEOSPolygonizeCutEdges);
PHP_FUNCTION(GEOSBuildArea);
PHP_FUNCTION(GEOSDisjointSubsetUnion);

#ifdef HAVE_GEOS_SHARED_PATHS
PHP_FUNCTION(GEOSSharedPaths);
#endif

#ifdef HAVE_GEOS_RELATE_PATTERN_MATCH
PHP_FUNCTION(GEOSRelateMatch);
#endif

#if PHP_VERSION_ID < 50399
#define zend_function_entry function_entry
#endif

#if PHP_VERSION_ID >= 70000
# define GEOS_PHP_DTOR_OBJECT zend_object
# define zend_object_value zend_object *
# define zend_uint size_t
# define MAKE_STD_ZVAL(x) x = emalloc(sizeof(zval))
# define GEOS_PHP_RETURN_STRING(x) { RETVAL_STRING((x)); efree((x)); return; }
# define GEOS_PHP_RETURN_STRINGL(x,s) { RETVAL_STRINGL((x),(s)); efree((x)); return; }
# define GEOS_PHP_ADD_ASSOC_ARRAY(a,k,v) { add_assoc_string((a), (k), (v)); efree((v)); }
# define GEOS_PHP_ADD_ASSOC_ZVAL(a,k,v) { add_assoc_zval((a), (k), (v)); efree((v)); }
# define GEOS_PHP_HASH_GET_CUR_KEY(s,k,i) zend_hash_get_current_key((s), (k), (i))
# define GEOS_PHP_HASH_GET_CUR_DATA(h,d) ( d = zend_hash_get_current_data((h)) )
# define GEOS_PHP_ZVAL zval *
#else /* PHP_VERSION_ID < 70000 */
# define GEOS_PHP_DTOR_OBJECT void
# define GEOS_PHP_RETURN_STRING(x) RETURN_STRING((x),0)
# define GEOS_PHP_RETURN_STRINGL(x,s) RETURN_STRINGL((x),(s),0)
# define GEOS_PHP_ADD_ASSOC_ARRAY(a,k,v) add_assoc_string((a), (k), (v), 0)
# define GEOS_PHP_ADD_ASSOC_ZVAL(a,k,v) add_assoc_zval((a), (k), (v))
# define GEOS_PHP_HASH_GET_CUR_KEY(s,k,i) zend_hash_get_current_key((s), (k), (i), 0)
# define zend_string char
# define zend_long long
# define ZSTR_VAL(x) (x)
# define GEOS_PHP_HASH_GET_CUR_DATA(h,d) zend_hash_get_current_data((h),(void**)&(d))
# define GEOS_PHP_ZVAL zval **
#endif


static zend_function_entry geos_functions[] = {
    PHP_FE(GEOSVersion, arginfo_GEOSVersion)
    PHP_FE(GEOSPolygonize, arginfo_GEOSPolygonize)
    PHP_FE(GEOSLineMerge, arginfo_GEOSLineMerge)
    PHP_FE(GEOSPolygonizeValid, arginfo_GEOSPolygonizeValid)
    PHP_FE(GEOSPolygonizeCutEdges, arginfo_GEOSPolygonizeCutEdges)
    PHP_FE(GEOSBuildArea, arginfo_GEOSBuildArea)
    PHP_FE(GEOSDisjointSubsetUnion, arginfo_GEOSDisjointSubsetUnion)

#   ifdef HAVE_GEOS_SHARED_PATHS
    PHP_FE(GEOSSharedPaths, arginfo_GEOSSharedPaths)
#   endif

#   ifdef HAVE_GEOS_RELATE_PATTERN_MATCH
    PHP_FE(GEOSRelateMatch, arginfo_GEOSRelateMatch)
#   endif

#ifdef PHP_FE_END
    PHP_FE_END
#else
    {NULL, NULL, NULL}
#endif
};

zend_module_entry geos_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_GEOS_EXTNAME,
    geos_functions,
    PHP_MINIT(geos),              /* module init function */
    PHP_MSHUTDOWN(geos),          /* module shutdown function */
    PHP_RINIT(geos),              /* request init function */
    PHP_RSHUTDOWN(geos),          /* request shutdown function */
    PHP_MINFO(geos),              /* module info function */
    PHP_GEOS_VERSION,
    PHP_MODULE_GLOBALS(geos),     /* globals descriptor */
    PHP_GINIT(geos),              /* globals ctor */
    NULL,                         /* globals dtor */
    NULL,                         /* post deactivate */
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_GEOS
ZEND_GET_MODULE(geos)
#endif

/* -- Utility functions ---------------------- */

static void noticeHandler(const char *fmt, ...)
{
    TSRMLS_FETCH();
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    php_error_docref(NULL TSRMLS_CC, E_NOTICE, "%s", message);
}

static void errorHandler(const char *fmt, ...)
{
    TSRMLS_FETCH();
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    /* TODO: use a GEOSException ? */
    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
        1 TSRMLS_CC, "%s", message);

}

typedef struct Proxy_t {
#if PHP_VERSION_ID >= 70000
    int id;
    void* relay;
    zend_object std;
#else
    zend_object std;
    void* relay;
#endif
} Proxy;

#if PHP_VERSION_ID >= 70000
static inline Proxy *php_geos_fetch_object(zend_object *obj) {
  return (Proxy *)((char *) obj - XtOffsetOf(Proxy, std));
}
# define Z_GEOS_OBJ_P(zv) (Proxy *)((char *) (Z_OBJ_P(zv)) - XtOffsetOf(Proxy, std))
#else
# ifdef Z_OBJ
#  define Z_GEOS_OBJ_P(zv) (Proxy*)Z_OBJ(*val TSRMLS_CC)
# else
#  define Z_GEOS_OBJ_P(zv) (Proxy*)zend_object_store_get_object(val TSRMLS_CC)
# endif
#endif

static void
setRelay(zval* val, void* obj) {
    TSRMLS_FETCH();

    Proxy* proxy = Z_GEOS_OBJ_P(val);

    proxy->relay = obj;
}

static inline void *
getRelay(zval* val, zend_class_entry* ce) {
    TSRMLS_FETCH();

    Proxy* proxy = Z_GEOS_OBJ_P(val);

    if ( proxy->std.ce != ce ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
#if PHP_VERSION_ID >= 70000
            "Relay object is not an %s", ZSTR_VAL(ce->name));
#else
            "Relay object is not an %s", ce->name);
#endif
    }
    if ( ! proxy->relay ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
#if PHP_VERSION_ID >= 70000
            "Relay object for object of type %s is not set", ZSTR_VAL(ce->name));
#else
            "Relay object for object of type %s is not set", ce->name);
#endif
    }
    return proxy->relay;
}

static long getZvalAsLong(GEOS_PHP_ZVAL val)
{
    long ret;
    zval tmp;

#if PHP_VERSION_ID >= 70000
    tmp = *val;
#else
    tmp = **val;
#endif
    zval_copy_ctor(&tmp);
    convert_to_long(&tmp);
    ret = Z_LVAL(tmp);
    zval_dtor(&tmp);
    return ret;
}

static long getZvalAsDouble(GEOS_PHP_ZVAL val)
{
    double ret;
    zval tmp;

#if PHP_VERSION_ID >= 70000
    tmp = *val;
#else
    tmp = **val;
#endif
    zval_copy_ctor(&tmp);
    convert_to_double(&tmp);
    ret = Z_DVAL(tmp);
    zval_dtor(&tmp);
    return ret;
}

static zend_object_value
Gen_create_obj (zend_class_entry *type,
    void (*dtor)(GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC),
    zend_object_handlers* handlers)
{
    TSRMLS_FETCH();

#if PHP_VERSION_ID >= 70000

    Proxy *obj = (Proxy *) ecalloc(1, sizeof(Proxy) + zend_object_properties_size(type));

    zend_object_std_init(&obj->std, type TSRMLS_CC);
    object_properties_init(&obj->std, type);

    obj->std.handlers = handlers;

    /* TODO: install the destructor ? (dtor) ! */
    /* TODO: do not allocate a full Proxy if we're going to use an object */

    return &obj->std;

#else /* PHP_VERSION_ID < 70000 */

    zend_object_value retval;

    Proxy *obj = (Proxy *)ecalloc(1, sizeof(Proxy));

    obj->std.ce = type;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID < 50399
    zend_hash_copy(obj->std.properties, &type->default_properties,
        (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval *));
#else
    object_properties_init(&(obj->std), type);
#endif

    retval.handle = zend_objects_store_put(obj, NULL, dtor, NULL TSRMLS_CC);
    retval.handlers = handlers;

    return retval;

#endif /* PHP_VERSION_ID < 70000 */
}


/* -- class GEOSGeometry -------------------- */

PHP_METHOD(Geometry, __construct);
PHP_METHOD(Geometry, __toString);
PHP_METHOD(Geometry, project);
PHP_METHOD(Geometry, interpolate);
PHP_METHOD(Geometry, buffer);

#ifdef HAVE_GEOS_OFFSET_CURVE
PHP_METHOD(Geometry, offsetCurve);
#endif

PHP_METHOD(Geometry, envelope);
PHP_METHOD(Geometry, intersection);
PHP_METHOD(Geometry, convexHull);
PHP_METHOD(Geometry, difference);
PHP_METHOD(Geometry, symDifference);
PHP_METHOD(Geometry, boundary);
PHP_METHOD(Geometry, union); /* also does union cascaded */
PHP_METHOD(Geometry, pointOnSurface);
PHP_METHOD(Geometry, centroid);
PHP_METHOD(Geometry, relate);

#ifdef HAVE_GEOS_RELATE_BOUNDARY_NODE_RULE
PHP_METHOD(Geometry, relateBoundaryNodeRule);
#endif

PHP_METHOD(Geometry, simplify); /* also does topology-preserving */
PHP_METHOD(Geometry, normalize);

#ifdef HAVE_GEOS_GEOM_SET_PRECISION
PHP_METHOD(Geometry, setPrecision);
#endif

#ifdef HAVE_GEOS_GEOM_GET_PRECISION
PHP_METHOD(Geometry, getPrecision);
#endif

#ifdef HAVE_GEOS_GEOM_EXTRACT_UNIQUE_POINTS
PHP_METHOD(Geometry, extractUniquePoints);
#endif

PHP_METHOD(Geometry, disjoint);
PHP_METHOD(Geometry, touches);
PHP_METHOD(Geometry, intersects);
PHP_METHOD(Geometry, crosses);
PHP_METHOD(Geometry, within);
PHP_METHOD(Geometry, contains);
PHP_METHOD(Geometry, overlaps);

#ifdef HAVE_GEOS_COVERS
PHP_METHOD(Geometry, covers);
#endif

#ifdef HAVE_GEOS_COVERED_BY
PHP_METHOD(Geometry, coveredBy);
#endif

PHP_METHOD(Geometry, equals);
PHP_METHOD(Geometry, equalsExact);
PHP_METHOD(Geometry, isEmpty);

#ifdef HAVE_GEOS_IS_VALID_DETAIL
PHP_METHOD(Geometry, checkValidity);
#endif

PHP_METHOD(Geometry, isSimple);
PHP_METHOD(Geometry, isRing);
PHP_METHOD(Geometry, hasZ);
PHP_METHOD(Geometry, hasM);
PHP_METHOD(Geometry, getCoordSeq);

#ifdef HAVE_GEOS_IS_CLOSED
PHP_METHOD(Geometry, isClosed);
#endif

PHP_METHOD(Geometry, typeName);
PHP_METHOD(Geometry, typeId);
PHP_METHOD(Geometry, getSRID);
PHP_METHOD(Geometry, setSRID);
PHP_METHOD(Geometry, numGeometries);
PHP_METHOD(Geometry, geometryN);
PHP_METHOD(Geometry, numInteriorRings);

#ifdef HAVE_GEOS_GEOM_GET_NUM_POINTS
PHP_METHOD(Geometry, numPoints);
#endif

#ifdef HAVE_GEOS_GEOM_GET_X
PHP_METHOD(Geometry, getX);
#endif

#ifdef HAVE_GEOS_GEOM_GET_Y
PHP_METHOD(Geometry, getY);
#endif

PHP_METHOD(Geometry, getZ);
PHP_METHOD(Geometry, getM);

PHP_METHOD(Geometry, interiorRingN);
PHP_METHOD(Geometry, exteriorRing);
PHP_METHOD(Geometry, numCoordinates);
PHP_METHOD(Geometry, dimension);

#ifdef HAVE_GEOS_GEOM_GET_COORDINATE_DIMENSION
PHP_METHOD(Geometry, coordinateDimension);
#endif

#ifdef HAVE_GEOS_GEOM_GET_POINT_N
PHP_METHOD(Geometry, pointN);
#endif

#ifdef HAVE_GEOS_GEOM_GET_START_POINT
PHP_METHOD(Geometry, startPoint);
#endif

#ifdef HAVE_GEOS_GEOM_GET_END_POINT
PHP_METHOD(Geometry, endPoint);
#endif

PHP_METHOD(Geometry, area);
PHP_METHOD(Geometry, length);
PHP_METHOD(Geometry, distance);
PHP_METHOD(Geometry, hausdorffDistance);

#ifdef HAVE_GEOS_SNAP
PHP_METHOD(Geometry, snapTo);
#endif

#ifdef HAVE_GEOS_NODE
PHP_METHOD(Geometry, node);
#endif

#ifdef HAVE_GEOS_DELAUNAY_TRIANGULATION
PHP_METHOD(Geometry, delaunayTriangulation);
#endif

#ifdef HAVE_GEOS_VORONOI_DIAGRAM
PHP_METHOD(Geometry, voronoiDiagram);
#endif

#ifdef HAVE_GEOS_CLIP_BY_RECT
PHP_METHOD(Geometry, clipByRect);
#endif

/* Items 3..11 — added methods */
PHP_METHOD(Geometry, makeValid);
PHP_METHOD(Geometry, concaveHull);
PHP_METHOD(Geometry, concaveHullByLength);
PHP_METHOD(Geometry, concaveHullOfPolygons);
PHP_METHOD(Geometry, polygonHullSimplify);
PHP_METHOD(Geometry, minimumBoundingCircle);
PHP_METHOD(Geometry, minimumRotatedRectangle);
PHP_METHOD(Geometry, minimumWidth);
PHP_METHOD(Geometry, minimumClearance);
PHP_METHOD(Geometry, minimumClearanceLine);
PHP_METHOD(Geometry, maximumInscribedCircle);
PHP_METHOD(Geometry, largestEmptyCircle);
PHP_METHOD(Geometry, distanceWithin);
PHP_METHOD(Geometry, distanceIndexed);
PHP_METHOD(Geometry, nearestPoints);
PHP_METHOD(Geometry, frechetDistance);
PHP_METHOD(Geometry, frechetDistanceDensify);
PHP_METHOD(Geometry, hausdorffDistanceDensify);
PHP_METHOD(Geometry, lineSubstring);
PHP_METHOD(Geometry, lineMergeDirected);
PHP_METHOD(Geometry, reverse);
PHP_METHOD(Geometry, densify);
PHP_METHOD(Geometry, removeRepeatedPoints);
PHP_METHOD(Geometry, orientPolygons);
PHP_METHOD(Geometry, equalsIdentical);
PHP_METHOD(Geometry, getXMin);
PHP_METHOD(Geometry, getXMax);
PHP_METHOD(Geometry, getYMin);
PHP_METHOD(Geometry, getYMax);
PHP_METHOD(Geometry, getExtent);
PHP_METHOD(Geometry, buildArea);

static zend_function_entry Geometry_methods[] = {
    PHP_ME(Geometry, __construct, arginfo_Geometry_construct, 0)
    PHP_ME(Geometry, __toString, arginfo_Geometry_toString, 0)
    PHP_ME(Geometry, project, arginfo_Geometry_project, 0)
    PHP_ME(Geometry, interpolate, arginfo_Geometry_interpolate, 0)
    PHP_ME(Geometry, buffer, arginfo_Geometry_buffer, 0)

#   ifdef HAVE_GEOS_OFFSET_CURVE
    PHP_ME(Geometry, offsetCurve, arginfo_Geometry_offsetCurve, 0)
#   endif

    PHP_ME(Geometry, envelope, arginfo_Geometry_envelope, 0)
    PHP_ME(Geometry, intersection, arginfo_Geometry_intersection, 0)
    PHP_ME(Geometry, convexHull, arginfo_Geometry_convexHull, 0)
    PHP_ME(Geometry, difference, arginfo_Geometry_difference, 0)
    PHP_ME(Geometry, symDifference, arginfo_Geometry_symDifference, 0)
    PHP_ME(Geometry, boundary, arginfo_Geometry_boundary, 0)
    PHP_ME(Geometry, union, arginfo_Geometry_union, 0)
    PHP_ME(Geometry, pointOnSurface, arginfo_Geometry_pointOnSurface, 0)
    PHP_ME(Geometry, centroid, arginfo_Geometry_centroid, 0)
    PHP_ME(Geometry, relate, arginfo_Geometry_relate, 0)

#   ifdef HAVE_GEOS_RELATE_BOUNDARY_NODE_RULE
    PHP_ME(Geometry, relateBoundaryNodeRule, arginfo_Geometry_relateBoundaryNodeRule, 0)
#   endif

    PHP_ME(Geometry, simplify, arginfo_Geometry_simplify, 0)
    PHP_ME(Geometry, normalize, arginfo_Geometry_normalize, 0)

#   ifdef HAVE_GEOS_GEOM_SET_PRECISION
    PHP_ME(Geometry, setPrecision, arginfo_Geometry_setPrecision, 0)
#   endif

#   if HAVE_GEOS_GEOM_GET_PRECISION
    PHP_ME(Geometry, getPrecision, arginfo_Geometry_getPrecision, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_EXTRACT_UNIQUE_POINTS
    PHP_ME(Geometry, extractUniquePoints, arginfo_Geometry_extractUniquePoints, 0)
#   endif

    PHP_ME(Geometry, disjoint, arginfo_Geometry_disjoint, 0)
    PHP_ME(Geometry, touches, arginfo_Geometry_touches, 0)
    PHP_ME(Geometry, intersects, arginfo_Geometry_intersects, 0)
    PHP_ME(Geometry, crosses, arginfo_Geometry_crosses, 0)
    PHP_ME(Geometry, within, arginfo_Geometry_within, 0)
    PHP_ME(Geometry, contains, arginfo_Geometry_contains, 0)
    PHP_ME(Geometry, overlaps, arginfo_Geometry_overlaps, 0)

#   ifdef HAVE_GEOS_COVERS
    PHP_ME(Geometry, covers, arginfo_Geometry_covers, 0)
#   endif

#   ifdef HAVE_GEOS_COVERED_BY
    PHP_ME(Geometry, coveredBy, arginfo_Geometry_coveredBy, 0)
#   endif

    PHP_ME(Geometry, equals, arginfo_Geometry_equals, 0)
    PHP_ME(Geometry, equalsExact, arginfo_Geometry_equalsExact, 0)
    PHP_ME(Geometry, isEmpty, arginfo_Geometry_isEmpty, 0)

#   ifdef HAVE_GEOS_IS_VALID_DETAIL
    PHP_ME(Geometry, checkValidity, arginfo_Geometry_checkValidity, 0)
#   endif

    PHP_ME(Geometry, isSimple, arginfo_Geometry_isSimple, 0)
    PHP_ME(Geometry, isRing, arginfo_Geometry_isRing, 0)
    PHP_ME(Geometry, hasZ, arginfo_Geometry_hasZ, 0)
    PHP_ME(Geometry, hasM, arginfo_Geometry_hasM, 0)
    PHP_ME(Geometry, getCoordSeq, arginfo_Geometry_getCoordSeq, 0)

#   ifdef HAVE_GEOS_IS_CLOSED
    PHP_ME(Geometry, isClosed, arginfo_Geometry_isClosed, 0)
#   endif

    PHP_ME(Geometry, typeName, arginfo_Geometry_typeName, 0)
    PHP_ME(Geometry, typeId, arginfo_Geometry_typeId, 0)
    PHP_ME(Geometry, getSRID, arginfo_Geometry_getSRID, 0)
    PHP_ME(Geometry, setSRID, arginfo_Geometry_setSRID, 0)
    PHP_ME(Geometry, numGeometries, arginfo_Geometry_numGeometries, 0)
    PHP_ME(Geometry, geometryN, arginfo_Geometry_geometryN, 0)
    PHP_ME(Geometry, numInteriorRings, arginfo_Geometry_numInteriorRings, 0)

#   ifdef HAVE_GEOS_GEOM_GET_NUM_POINTS
    PHP_ME(Geometry, numPoints, arginfo_Geometry_numPoints, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_GET_X
    PHP_ME(Geometry, getX, arginfo_Geometry_getX, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_GET_Y
    PHP_ME(Geometry, getY, arginfo_Geometry_getY, 0)
#   endif

    PHP_ME(Geometry, getZ, arginfo_Geometry_getZ, 0)
    PHP_ME(Geometry, getM, arginfo_Geometry_getM, 0)

    PHP_ME(Geometry, interiorRingN, arginfo_Geometry_interiorRingN, 0)
    PHP_ME(Geometry, exteriorRing, arginfo_Geometry_exteriorRing, 0)
    PHP_ME(Geometry, numCoordinates, arginfo_Geometry_numCoordinates, 0)
    PHP_ME(Geometry, dimension, arginfo_Geometry_dimension, 0)

#   ifdef HAVE_GEOS_GEOM_GET_COORDINATE_DIMENSION
    PHP_ME(Geometry, coordinateDimension, arginfo_Geometry_coordinateDimension, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_GET_POINT_N
    PHP_ME(Geometry, pointN, arginfo_Geometry_pointN, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_GET_START_POINT
    PHP_ME(Geometry, startPoint, arginfo_Geometry_startPoint, 0)
#   endif

#   ifdef HAVE_GEOS_GEOM_GET_END_POINT
    PHP_ME(Geometry, endPoint, arginfo_Geometry_endPoint, 0)
#   endif

    PHP_ME(Geometry, area, arginfo_Geometry_area, 0)
    PHP_ME(Geometry, length, arginfo_Geometry_length, 0)
    PHP_ME(Geometry, distance, arginfo_Geometry_distance, 0)
    PHP_ME(Geometry, hausdorffDistance, arginfo_Geometry_hausdorffDistance, 0)

#   if HAVE_GEOS_SNAP
    PHP_ME(Geometry, snapTo, arginfo_Geometry_snapTo, 0)
#   endif

#   ifdef HAVE_GEOS_NODE
    PHP_ME(Geometry, node, arginfo_Geometry_node, 0)
#   endif

#   ifdef HAVE_GEOS_DELAUNAY_TRIANGULATION
    PHP_ME(Geometry, delaunayTriangulation, arginfo_Geometry_delaunayTriangulation, 0)
#   endif

#   ifdef HAVE_GEOS_VORONOI_DIAGRAM
    PHP_ME(Geometry, voronoiDiagram, arginfo_Geometry_voronoiDiagram, 0)
#   endif

#   ifdef HAVE_GEOS_CLIP_BY_RECT
    PHP_ME(Geometry, clipByRect, arginfo_Geometry_clipByRect, 0)
#   endif

    /* Items 3..11 — added methods */
    PHP_ME(Geometry, makeValid, arginfo_Geometry_makeValid, 0)
    PHP_ME(Geometry, concaveHull, arginfo_Geometry_concaveHull, 0)
    PHP_ME(Geometry, concaveHullByLength, arginfo_Geometry_concaveHullByLength, 0)
    PHP_ME(Geometry, concaveHullOfPolygons, arginfo_Geometry_concaveHullOfPolygons, 0)
    PHP_ME(Geometry, polygonHullSimplify, arginfo_Geometry_polygonHullSimplify, 0)
    PHP_ME(Geometry, minimumBoundingCircle, arginfo_Geometry_minimumBoundingCircle, 0)
    PHP_ME(Geometry, minimumRotatedRectangle, arginfo_Geometry_minimumRotatedRectangle, 0)
    PHP_ME(Geometry, minimumWidth, arginfo_Geometry_minimumWidth, 0)
    PHP_ME(Geometry, minimumClearance, arginfo_Geometry_minimumClearance, 0)
    PHP_ME(Geometry, minimumClearanceLine, arginfo_Geometry_minimumClearanceLine, 0)
    PHP_ME(Geometry, maximumInscribedCircle, arginfo_Geometry_maximumInscribedCircle, 0)
    PHP_ME(Geometry, largestEmptyCircle, arginfo_Geometry_largestEmptyCircle, 0)
    PHP_ME(Geometry, distanceWithin, arginfo_Geometry_distanceWithin, 0)
    PHP_ME(Geometry, distanceIndexed, arginfo_Geometry_distanceIndexed, 0)
    PHP_ME(Geometry, nearestPoints, arginfo_Geometry_nearestPoints, 0)
    PHP_ME(Geometry, frechetDistance, arginfo_Geometry_frechetDistance, 0)
    PHP_ME(Geometry, frechetDistanceDensify, arginfo_Geometry_frechetDistanceDensify, 0)
    PHP_ME(Geometry, hausdorffDistanceDensify, arginfo_Geometry_hausdorffDistanceDensify, 0)
    PHP_ME(Geometry, lineSubstring, arginfo_Geometry_lineSubstring, 0)
    PHP_ME(Geometry, lineMergeDirected, arginfo_Geometry_lineMergeDirected, 0)
    PHP_ME(Geometry, reverse, arginfo_Geometry_reverse, 0)
    PHP_ME(Geometry, densify, arginfo_Geometry_densify, 0)
    PHP_ME(Geometry, removeRepeatedPoints, arginfo_Geometry_removeRepeatedPoints, 0)
    PHP_ME(Geometry, orientPolygons, arginfo_Geometry_orientPolygons, 0)
    PHP_ME(Geometry, equalsIdentical, arginfo_Geometry_equalsIdentical, 0)
    PHP_ME(Geometry, getXMin, arginfo_Geometry_getXMin, 0)
    PHP_ME(Geometry, getXMax, arginfo_Geometry_getXMax, 0)
    PHP_ME(Geometry, getYMin, arginfo_Geometry_getYMin, 0)
    PHP_ME(Geometry, getYMax, arginfo_Geometry_getYMax, 0)
    PHP_ME(Geometry, getExtent, arginfo_Geometry_getExtent, 0)
    PHP_ME(Geometry, buildArea, arginfo_Geometry_buildArea, 0)

    {NULL, NULL, NULL}
};

static zend_class_entry *Geometry_ce_ptr;
/* forward declared for use by Geometry::nearestPoints; defined with the
 * GEOSCoordSeq class below. */
static zend_class_entry *CoordSeq_ce_ptr;

static zend_object_handlers Geometry_object_handlers;

/* Geometry serializer */

static GEOSWKBWriter* Geometry_serializer = 0;

static GEOSWKBWriter* getGeometrySerializer()
{
    TSRMLS_FETCH();

    if ( ! Geometry_serializer ) {
        Geometry_serializer = GEOSWKBWriter_create_r(GEOS_G(handle));
        GEOSWKBWriter_setIncludeSRID_r(GEOS_G(handle), Geometry_serializer, 1);
        GEOSWKBWriter_setOutputDimension_r(GEOS_G(handle), Geometry_serializer, 3);
    }
    return Geometry_serializer;
}

static void delGeometrySerializer()
{
    TSRMLS_FETCH();

    if ( Geometry_serializer ) {
        GEOSWKBWriter_destroy_r(GEOS_G(handle), Geometry_serializer);
    }
}

/* Geometry deserializer */

static GEOSWKBReader* Geometry_deserializer = 0;

static GEOSWKBReader* getGeometryDeserializer()
{
    TSRMLS_FETCH();

    if ( ! Geometry_deserializer ) {
        Geometry_deserializer = GEOSWKBReader_create_r(GEOS_G(handle));
    }
    return Geometry_deserializer;
}

static void delGeometryDeserializer()
{
    TSRMLS_FETCH();

    if ( Geometry_deserializer ) {
        GEOSWKBReader_destroy_r(GEOS_G(handle), Geometry_deserializer);
    }
}

/* Serializer function for GEOSGeometry */

static int
Geometry_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len,
        zend_serialize_data *data TSRMLS_DC)
{
    GEOSWKBWriter *serializer;
    GEOSGeometry *geom;
    char* ret;
    size_t retsize;


    serializer = getGeometrySerializer();
    geom = (GEOSGeometry*)getRelay(object, Geometry_ce_ptr);

    /* NOTE: we might be fine using binary here */
    ret = (char*)GEOSWKBWriter_writeHEX_r(GEOS_G(handle), serializer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) return FAILURE;

    *buffer = (unsigned char*)estrndup(ret, retsize);
    GEOSFree_r(GEOS_G(handle), ret);

    *buf_len = retsize;

    return SUCCESS;
}

static int
Geometry_deserialize(GEOS_PHP_ZVAL object, zend_class_entry *ce, const unsigned char *buf,
        zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
    GEOSWKBReader* deserializer;
    GEOSGeometry* geom;

    deserializer = getGeometryDeserializer();
    geom = GEOSWKBReader_readHEX_r(GEOS_G(handle), deserializer, buf, buf_len);

    /* check zend_class_entry being what we expect! */
    if ( ce != Geometry_ce_ptr ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Geometry_deserialize called with unexpected zend_class_entry");
        return FAILURE;
    }
#if PHP_VERSION_ID >= 70000
    object_init_ex(object, ce);
    setRelay(object, geom);
#else
    object_init_ex(*object, ce);
    setRelay(*object, geom);
#endif

    return SUCCESS;
}

/*
 * Push components of the given geometry
 * to the given array zval.
 * Components geometries are cloned.
 * NOTE: collection components are not descended into
 */
static void
dumpGeometry(GEOSGeometry* g, zval* array)
{
    TSRMLS_FETCH();
    int ngeoms, i;

    ngeoms = GEOSGetNumGeometries_r(GEOS_G(handle), g);
    for (i=0; i<ngeoms; ++i)
    {
        zval *tmp;
        GEOSGeometry* cc;
        const GEOSGeometry* c = GEOSGetGeometryN_r(GEOS_G(handle), g, i);
        if ( ! c ) continue; /* should get an exception */
        /* we _need_ to clone as this one is owned by 'g' */
        cc = GEOSGeom_clone_r(GEOS_G(handle), c);
        if ( ! cc ) continue; /* should get an exception */

        MAKE_STD_ZVAL(tmp);
        object_init_ex(tmp, Geometry_ce_ptr);
        setRelay(tmp, cc);
        add_next_index_zval(array, tmp);
#if PHP_VERSION_ID >= 70000
        efree(tmp);
#endif
    }
}


static void
Geometry_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSGeom_destroy_r(GEOS_G(handle), (GEOSGeometry*)obj->relay);

#if PHP_VERSION_ID >= 70000
    //zend_object_std_dtor(&obj->std);
#else
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
Geometry_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, Geometry_dtor, &Geometry_object_handlers);
}


PHP_METHOD(Geometry, __construct)
{
    php_error_docref(NULL TSRMLS_CC, E_ERROR,
            "GEOSGeometry can't be constructed using new, check WKTReader");

}

PHP_METHOD(Geometry, __toString)
{
    GEOSGeometry *geom;
    GEOSWKTWriter *writer;
    char *wkt;
    char *ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    writer = GEOSWKTWriter_create_r(GEOS_G(handle));
    /* NOTE: if we get an exception before reaching
     *       GEOSWKTWriter_destory below we'll be leaking memory.
     *       One fix could be storing the object in a refcounted
     *       zval.
     */
#   ifdef HAVE_GEOS_WKT_WRITER_SET_TRIM
    GEOSWKTWriter_setTrim_r(GEOS_G(handle), writer, 1);
#   endif

    wkt = GEOSWKTWriter_write_r(GEOS_G(handle), writer, geom);
    /* we'll probably get an exception if wkt is null */
    if ( ! wkt ) RETURN_NULL();

    GEOSWKTWriter_destroy_r(GEOS_G(handle), writer);


    ret = estrdup(wkt);
    GEOSFree_r(GEOS_G(handle), wkt);

    GEOS_PHP_RETURN_STRING(ret);
}

PHP_METHOD(Geometry, project)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    zend_bool normalized = 0;
    double ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|b", &zobj,
            &normalized) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    if ( normalized ) {
        ret = GEOSProjectNormalized_r(GEOS_G(handle), this, other);
    } else {
        ret = GEOSProject_r(GEOS_G(handle), this, other);
    }
    if ( ret < 0 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(ret);
}

PHP_METHOD(Geometry, interpolate)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    zend_bool normalized = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &dist, &normalized) == FAILURE) {
        RETURN_NULL();
    }

    if ( normalized ) {
        ret = GEOSInterpolateNormalized_r(GEOS_G(handle), this, dist);
    } else {
        ret = GEOSInterpolate_r(GEOS_G(handle), this, dist);
    }
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::buffer(dist, [<styleArray>])
 *
 * styleArray keys supported:
 *  'quad_segs'
 *       Type: int
 *       Number of segments used to approximate
 *       a quarter circle (defaults to 8).
 *  'endcap'
 *       Type: long
 *       Endcap style (defaults to GEOSBUF_CAP_ROUND)
 *  'join'
 *       Type: long
 *       Join style (defaults to GEOSBUF_JOIN_ROUND)
 *  'mitre_limit'
 *       Type: double
 *       mitre ratio limit (only affects joins with GEOSBUF_JOIN_MITRE style)
 *       'miter_limit' is also accepted as a synonym for 'mitre_limit'.
 *  'single_sided'
 *       Type: bool
 *       If true buffer lines only on one side, so that the input line
 *       will be a portion of the boundary of the returned polygon.
 *       Only applies to lineal input. Defaults to false.
 */
PHP_METHOD(Geometry, buffer)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    GEOSBufferParams *params;
    static const double default_mitreLimit = 5.0;
    static const int default_endCapStyle = GEOSBUF_CAP_ROUND;
    static const int default_joinStyle = GEOSBUF_JOIN_ROUND;
    static const int default_quadSegs = 8;
    long int quadSegs = default_quadSegs;
    long int endCapStyle = default_endCapStyle;
    long int joinStyle = default_joinStyle;
    double mitreLimit = default_mitreLimit;
    long singleSided = 0;
    zval *style_val = NULL;
    GEOS_PHP_ZVAL data;
    HashTable *style;
    zend_string *key;
    zend_ulong index;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|a",
            &dist, &style_val) == FAILURE) {
        RETURN_NULL();
    }

    params = GEOSBufferParams_create_r(GEOS_G(handle));

    if ( style_val )
    {
        style = HASH_OF(style_val);
        while(GEOS_PHP_HASH_GET_CUR_KEY(style, &key, &index)
              == HASH_KEY_IS_STRING)
        {
            if(!strcmp(ZSTR_VAL(key), "quad_segs"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                quadSegs = getZvalAsLong(data);
                GEOSBufferParams_setQuadrantSegments_r(GEOS_G(handle), params, quadSegs);
            }
            else if(!strcmp(ZSTR_VAL(key), "endcap"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                endCapStyle = getZvalAsLong(data);
                GEOSBufferParams_setEndCapStyle_r(GEOS_G(handle), params, endCapStyle);
            }
            else if(!strcmp(ZSTR_VAL(key), "join"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                joinStyle = getZvalAsLong(data);
                GEOSBufferParams_setJoinStyle_r(GEOS_G(handle), params, joinStyle);
            }
            else if(!strcmp(ZSTR_VAL(key), "mitre_limit"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                mitreLimit = getZvalAsDouble(data);
                GEOSBufferParams_setMitreLimit_r(GEOS_G(handle), params, mitreLimit);
            }
            else if(!strcmp(ZSTR_VAL(key), "single_sided"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                singleSided = getZvalAsLong(data);
                GEOSBufferParams_setSingleSided_r(GEOS_G(handle), params, singleSided);
            }

            zend_hash_move_forward(style);
        }
    }

    ret = GEOSBufferWithParams_r(GEOS_G(handle), this, params, dist);
    GEOSBufferParams_destroy_r(GEOS_G(handle), params);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::offsetCurve(dist, [<styleArray>])
 *
 * styleArray keys supported:
 *  'quad_segs'
 *       Type: int
 *       Number of segments used to approximate
 *       a quarter circle (defaults to 8).
 *  'join'
 *       Type: long
 *       Join style (defaults to GEOSBUF_JOIN_ROUND)
 *  'mitre_limit'
 *       Type: double
 *       mitre ratio limit (only affects joins with GEOSBUF_JOIN_MITRE style)
 *       'miter_limit' is also accepted as a synonym for 'mitre_limit'.
 */
#ifdef HAVE_GEOS_OFFSET_CURVE
PHP_METHOD(Geometry, offsetCurve)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    static const double default_mitreLimit = 5.0;
    static const int default_joinStyle = GEOSBUF_JOIN_ROUND;
    static const int default_quadSegs = 8;
    long int quadSegs = default_quadSegs;
    long int joinStyle = default_joinStyle;
    double mitreLimit = default_mitreLimit;
    zval *style_val = NULL;
    GEOS_PHP_ZVAL data;
    HashTable *style;
    zend_string *key;
    zend_ulong index;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|a",
            &dist, &style_val) == FAILURE) {
        RETURN_NULL();
    }

    if ( style_val )
    {
        style = HASH_OF(style_val);
        while(GEOS_PHP_HASH_GET_CUR_KEY(style, &key, &index)
              == HASH_KEY_IS_STRING)
        {
            if(!strcmp(ZSTR_VAL(key), "quad_segs"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                quadSegs = getZvalAsLong(data);
            }
            else if(!strcmp(ZSTR_VAL(key), "join"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                joinStyle = getZvalAsLong(data);
            }
            else if(!strcmp(ZSTR_VAL(key), "mitre_limit"))
            {
                GEOS_PHP_HASH_GET_CUR_DATA(style, data);
                mitreLimit = getZvalAsDouble(data);
            }

            zend_hash_move_forward(style);
        }
    }

    ret = GEOSOffsetCurve_r(GEOS_G(handle), this, dist, quadSegs, joinStyle, mitreLimit);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

PHP_METHOD(Geometry, envelope)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSEnvelope_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, intersection)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;
    double gridSize = 0.0;
    int has_gridSize = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (ZEND_NUM_ARGS() >= 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od", &zobj,
                &gridSize) == FAILURE) {
            RETURN_NULL();
        }
        has_gridSize = 1;
    } else {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
                == FAILURE) {
            RETURN_NULL();
        }
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    if (has_gridSize) {
        ret = GEOSIntersectionPrec_r(GEOS_G(handle), this, other, gridSize);
    } else {
        ret = GEOSIntersection_r(GEOS_G(handle), this, other);
    }
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::clipByRect(xmin,ymin,xmax,ymax)
 */
#ifdef HAVE_GEOS_CLIP_BY_RECT
PHP_METHOD(Geometry, clipByRect)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double xmin,ymin,xmax,ymax;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dddd",
            &xmin, &ymin, &xmax, &ymax) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSClipByRect_r(GEOS_G(handle), this, xmin, ymin, xmax, ymax);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

PHP_METHOD(Geometry, convexHull)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSConvexHull_r(GEOS_G(handle), this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, difference)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;
    double gridSize = 0.0;
    int has_gridSize = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (ZEND_NUM_ARGS() >= 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od", &zobj,
                &gridSize) == FAILURE) {
            RETURN_NULL();
        }
        has_gridSize = 1;
    } else {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
                == FAILURE) {
            RETURN_NULL();
        }
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    if (has_gridSize) {
        ret = GEOSDifferencePrec_r(GEOS_G(handle), this, other, gridSize);
    } else {
        ret = GEOSDifference_r(GEOS_G(handle), this, other);
    }
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, symDifference)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;
    double gridSize = 0.0;
    int has_gridSize = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (ZEND_NUM_ARGS() >= 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od", &zobj,
                &gridSize) == FAILURE) {
            RETURN_NULL();
        }
        has_gridSize = 1;
    } else {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
                == FAILURE) {
            RETURN_NULL();
        }
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    if (has_gridSize) {
        ret = GEOSSymDifferencePrec_r(GEOS_G(handle), this, other, gridSize);
    } else {
        ret = GEOSSymDifference_r(GEOS_G(handle), this, other);
    }
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, boundary)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSBoundary_r(GEOS_G(handle), this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::union()
 * GEOSGeometry::union(otherGeom)
 * GEOSGeometry::union(otherGeom = null, gridSize)
 */
PHP_METHOD(Geometry, union)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj = NULL;
    double gridSize = 0.0;
    int has_gridSize = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (ZEND_NUM_ARGS() >= 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o!d", &zobj,
                &gridSize) == FAILURE) {
            RETURN_NULL();
        }
        has_gridSize = 1;
    } else {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|o!", &zobj)
                == FAILURE) {
            RETURN_NULL();
        }
    }

    if ( zobj ) {
        other = getRelay(zobj, Geometry_ce_ptr);
        if (has_gridSize) {
            ret = GEOSUnionPrec_r(GEOS_G(handle), this, other, gridSize);
        } else {
            ret = GEOSUnion_r(GEOS_G(handle), this, other);
        }
    } else {
        if (has_gridSize) {
            ret = GEOSUnaryUnionPrec_r(GEOS_G(handle), this, gridSize);
        } else {
#       ifdef HAVE_GEOS_UNARY_UNION
            ret = GEOSUnaryUnion_r(GEOS_G(handle), this);
#       else
            ret = GEOSUnionCascaded_r(GEOS_G(handle), this);
#       endif
        }
    }

    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::pointOnSurface()
 */
PHP_METHOD(Geometry, pointOnSurface)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSPointOnSurface_r(GEOS_G(handle), this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::centroid()
 */
PHP_METHOD(Geometry, centroid)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetCentroid_r(GEOS_G(handle), this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::relate(otherGeom)
 * GEOSGeometry::relate(otherGeom, pattern)
 */
PHP_METHOD(Geometry, relate)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    char* pat = NULL;
#if PHP_VERSION_ID >= 70000
    size_t patlen;
#else
    int patlen;
#endif
    int retInt;
    zend_bool retBool;
    char* retStr;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|s",
        &zobj, &pat, &patlen) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    if ( ! pat ) {
        /* we'll compute it */
        pat = GEOSRelate_r(GEOS_G(handle), this, other);
        if ( ! pat ) RETURN_NULL(); /* should get an exception first */
        retStr = estrdup(pat);
        GEOSFree_r(GEOS_G(handle), pat);
        GEOS_PHP_RETURN_STRING(retStr);
    } else {
        retInt = GEOSRelatePattern_r(GEOS_G(handle), this, other, pat);
        if ( retInt == 2 ) RETURN_NULL(); /* should get an exception first */
        retBool = retInt;
        RETURN_BOOL(retBool);
    }

}

/**
 * GEOSGeometry::relateBoundaryNodeRule(otherGeom, rule)
 */
#ifdef HAVE_GEOS_RELATE_BOUNDARY_NODE_RULE
PHP_METHOD(Geometry, relateBoundaryNodeRule)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    char* pat;
    zend_long bnr = GEOSRELATE_BNR_OGC;
    char* retStr;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ol",
        &zobj, &bnr) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    /* we'll compute it */
    pat = GEOSRelateBoundaryNodeRule_r(GEOS_G(handle), this, other, bnr);
    if ( ! pat ) RETURN_NULL(); /* should get an exception first */
    retStr = estrdup(pat);
    GEOSFree_r(GEOS_G(handle), pat);
    GEOS_PHP_RETURN_STRING(retStr);
}
#endif

/**
 * GEOSGeometry GEOSGeometry::simplify(tolerance)
 * GEOSGeometry GEOSGeometry::simplify(tolerance, preserveTopology)
 */
PHP_METHOD(Geometry, simplify)
{
    GEOSGeometry *this;
    double tolerance;
    zend_bool preserveTopology = 0;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &tolerance, &preserveTopology) == FAILURE) {
        RETURN_NULL();
    }

    if ( preserveTopology ) {
        ret = GEOSTopologyPreserveSimplify_r(GEOS_G(handle), this, tolerance);
    } else {
        ret = GEOSSimplify_r(GEOS_G(handle), this, tolerance);
    }

    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::setPrecision(gridsize, [flags])
 */
#ifdef HAVE_GEOS_GEOM_SET_PRECISION
PHP_METHOD(Geometry, setPrecision)
{
    GEOSGeometry *this;
    double gridSize;
    zend_long flags = 0;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|l",
            &gridSize, &flags) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSGeom_setPrecision_r(GEOS_G(handle), this, gridSize, flags);

    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

/**
 * double GEOSGeometry::getPrecision()
 */
#ifdef HAVE_GEOS_GEOM_GET_PRECISION
PHP_METHOD(Geometry, getPrecision)
{
    GEOSGeometry *geom;
    double prec;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    prec = GEOSGeom_getPrecision_r(GEOS_G(handle), geom);
    if ( prec < 0 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(prec);
}
#endif

/**
 * GEOSGeometry GEOSGeometry::normalize()
 */
PHP_METHOD(Geometry, normalize)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_clone_r(GEOS_G(handle), this);

    if ( ! ret ) RETURN_NULL();

    GEOSNormalize_r(GEOS_G(handle), ret); /* exception should be gotten automatically */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::extractUniquePoints()
 */
#ifdef HAVE_GEOS_GEOM_EXTRACT_UNIQUE_POINTS
PHP_METHOD(Geometry, extractUniquePoints)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_extractUniquePoints_r(GEOS_G(handle), this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

/**
 * bool GEOSGeometry::disjoint(GEOSGeometry)
 */
PHP_METHOD(Geometry, disjoint)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDisjoint_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::touches(GEOSGeometry)
 */
PHP_METHOD(Geometry, touches)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSTouches_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::intersects(GEOSGeometry)
 */
PHP_METHOD(Geometry, intersects)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSIntersects_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::crosses(GEOSGeometry)
 */
PHP_METHOD(Geometry, crosses)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCrosses_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::within(GEOSGeometry)
 */
PHP_METHOD(Geometry, within)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSWithin_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::contains(GEOSGeometry)
 */
PHP_METHOD(Geometry, contains)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSContains_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::overlaps(GEOSGeometry)
 */
PHP_METHOD(Geometry, overlaps)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSOverlaps_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::covers(GEOSGeometry)
 */
#ifdef HAVE_GEOS_COVERS
PHP_METHOD(Geometry, covers)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCovers_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}
#endif

/**
 * bool GEOSGeometry::coveredBy(GEOSGeometry)
 */
#ifdef HAVE_GEOS_COVERED_BY
PHP_METHOD(Geometry, coveredBy)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCoveredBy_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}
#endif

/**
 * bool GEOSGeometry::equals(GEOSGeometry)
 */
PHP_METHOD(Geometry, equals)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSEquals_r(GEOS_G(handle), this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::equalsExact(GEOSGeometry)
 * bool GEOSGeometry::equalsExact(GEOSGeometry, double tolerance)
 */
PHP_METHOD(Geometry, equalsExact)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    double tolerance = 0;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|d",
        &zobj, &tolerance) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSEqualsExact_r(GEOS_G(handle), this, other, tolerance);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isEmpty()
 */
PHP_METHOD(Geometry, isEmpty)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisEmpty_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * array GEOSGeometry::checkValidity()
 */
#ifdef HAVE_GEOS_IS_VALID_DETAIL
PHP_METHOD(Geometry, checkValidity)
{
    GEOSGeometry *this;
    GEOSGeometry *location = NULL;
    int ret;
    char *reason = NULL;
    zend_bool retBool;
    char *reasonVal = NULL;
    zval *locationVal = NULL;
    zend_long flags = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
        &flags) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSisValidDetail_r(GEOS_G(handle), this, flags, &reason, &location);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    if ( reason ) {
        reasonVal = estrdup(reason);
        GEOSFree_r(GEOS_G(handle), reason);
    }

    if ( location ) {
        MAKE_STD_ZVAL(locationVal);
        object_init_ex(locationVal, Geometry_ce_ptr);
        setRelay(locationVal, location);
    }

    retBool = ret;

    /* return value is an array */
    array_init(return_value);
    add_assoc_bool(return_value, "valid", retBool);
    if ( reasonVal ) GEOS_PHP_ADD_ASSOC_ARRAY(return_value, "reason", reasonVal);
    if ( locationVal ) GEOS_PHP_ADD_ASSOC_ZVAL(return_value, "location", locationVal);

}
#endif

/**
 * bool GEOSGeometry::isSimple()
 */
PHP_METHOD(Geometry, isSimple)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisSimple_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isRing()
 */
PHP_METHOD(Geometry, isRing)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisRing_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::hasZ()
 */
PHP_METHOD(Geometry, hasZ)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSHasZ_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::hasM()
 */
PHP_METHOD(Geometry, hasM)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSHasM_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isClosed()
 */
#ifdef HAVE_GEOS_IS_CLOSED
PHP_METHOD(Geometry, isClosed)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisClosed_r(GEOS_G(handle), this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}
#endif

/**
 * string GEOSGeometry::typeName()
 */
PHP_METHOD(Geometry, typeName)
{
    GEOSGeometry *this;
    char *typ;
    char *typVal;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    /* TODO: define constant strings instead... */

    typ = GEOSGeomType_r(GEOS_G(handle), this);
    if ( ! typ ) RETURN_NULL(); /* should get an exception first */

    typVal = estrdup(typ);
    GEOSFree_r(GEOS_G(handle), typ);

    GEOS_PHP_RETURN_STRING(typVal);
}

/**
 * long GEOSGeometry::typeId()
 */
PHP_METHOD(Geometry, typeId)
{
    GEOSGeometry *this;
    long typ;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    /* TODO: define constant strings instead... */

    typ = GEOSGeomTypeId_r(GEOS_G(handle), this);
    if ( typ == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(typ);
}

/**
 * long GEOSGeometry::getSRID()
 */
PHP_METHOD(Geometry, getSRID)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetSRID_r(GEOS_G(handle), geom);

    RETURN_LONG(ret);
}

/**
 * void GEOSGeometry::setSRID(long)
 */
PHP_METHOD(Geometry, setSRID)
{
    GEOSGeometry *geom;
    zend_long srid;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &srid) == FAILURE) {
        RETURN_NULL();
    }

    GEOSSetSRID_r(GEOS_G(handle), geom, srid);
}

/**
 * long GEOSGeometry::numGeometries()
 */
PHP_METHOD(Geometry, numGeometries)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumGeometries_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * GEOSGeometry GEOSGeometry::geometryN()
 */
PHP_METHOD(Geometry, geometryN)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;
    zend_long num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGetNumGeometries_r(GEOS_G(handle), geom) ) RETURN_NULL();
    c = GEOSGetGeometryN_r(GEOS_G(handle), geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone_r(GEOS_G(handle), c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * long GEOSGeometry::numInteriorRings()
 */
PHP_METHOD(Geometry, numInteriorRings)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumInteriorRings_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::numPoints()
 */
#ifdef HAVE_GEOS_GEOM_GET_NUM_POINTS
PHP_METHOD(Geometry, numPoints)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetNumPoints_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}
#endif

/**
 * double GEOSGeometry::getX()
 */
#ifdef HAVE_GEOS_GEOM_GET_X
PHP_METHOD(Geometry, getX)
{
    GEOSGeometry *geom;
    int ret;
    double x;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetX_r(GEOS_G(handle), geom, &x);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(x);
}
#endif

/**
 * double GEOSGeometry::getY()
 */
#ifdef HAVE_GEOS_GEOM_GET_Y
PHP_METHOD(Geometry, getY)
{
    GEOSGeometry *geom;
    int ret;
    double y;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetY_r(GEOS_G(handle), geom, &y);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(y);
}
#endif

/**
 * double GEOSGeometry::getZ()
 *
 * Only valid for non-empty Point geometries; the underlying
 * GEOS API throws on other types.
 */
PHP_METHOD(Geometry, getZ)
{
    GEOSGeometry *geom;
    int ret;
    double z;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetZ_r(GEOS_G(handle), geom, &z);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(z);
}

/**
 * double GEOSGeometry::getM()
 *
 * Only valid for non-empty Point geometries; the underlying
 * GEOS API throws on other types.
 */
PHP_METHOD(Geometry, getM)
{
    GEOSGeometry *geom;
    int ret;
    double m;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetM_r(GEOS_G(handle), geom, &m);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(m);
}

/**
 * GEOSGeometry GEOSGeometry::interiorRingN()
 */
PHP_METHOD(Geometry, interiorRingN)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;
    zend_long num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGetNumInteriorRings_r(GEOS_G(handle), geom) ) RETURN_NULL();
    c = GEOSGetInteriorRingN_r(GEOS_G(handle), geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone_r(GEOS_G(handle), c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * GEOSGeometry GEOSGeometry::exteriorRing()
 */
PHP_METHOD(Geometry, exteriorRing)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGetExteriorRing_r(GEOS_G(handle), geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone_r(GEOS_G(handle), c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * long GEOSGeometry::numCoordinates()
 */
PHP_METHOD(Geometry, numCoordinates)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumCoordinates_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::dimension()
 * 0:puntual 1:lineal 2:areal
 */
PHP_METHOD(Geometry, dimension)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_getDimensions_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::coordinateDimension()
 */
#ifdef HAVE_GEOS_GEOM_GET_COORDINATE_DIMENSION
PHP_METHOD(Geometry, coordinateDimension)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_getCoordinateDimension_r(GEOS_G(handle), geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}
#endif

/**
 * GEOSGeometry GEOSGeometry::pointN()
 */
#ifdef HAVE_GEOS_GEOM_GET_POINT_N
PHP_METHOD(Geometry, pointN)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;
    zend_long num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGeomGetNumPoints_r(GEOS_G(handle), geom) ) RETURN_NULL();
    c = GEOSGeomGetPointN_r(GEOS_G(handle), geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}
#endif

/**
 * GEOSGeometry GEOSGeometry::startPoint()
 */
PHP_METHOD(Geometry, startPoint)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGeomGetStartPoint_r(GEOS_G(handle), geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}

/**
 * GEOSGeometry GEOSGeometry::endPoint()
 */
PHP_METHOD(Geometry, endPoint)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGeomGetEndPoint_r(GEOS_G(handle), geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}

/**
 * double GEOSGeometry::area()
 */
PHP_METHOD(Geometry, area)
{
    GEOSGeometry *geom;
    double area;
    int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSArea_r(GEOS_G(handle), geom, &area);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(area);
}

/**
 * double GEOSGeometry::length()
 */
PHP_METHOD(Geometry, length)
{
    GEOSGeometry *geom;
    double length;
    int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSLength_r(GEOS_G(handle), geom, &length);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(length);
}

/**
 * double GEOSGeometry::distance(GEOSGeometry)
 */
PHP_METHOD(Geometry, distance)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDistance_r(GEOS_G(handle), this, other, &dist);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(dist);
}

/**
 * double GEOSGeometry::hausdorffDistance(GEOSGeometry)
 */
PHP_METHOD(Geometry, hausdorffDistance)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSHausdorffDistance_r(GEOS_G(handle), this, other, &dist);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(dist);
}

#ifdef HAVE_GEOS_SNAP
PHP_METHOD(Geometry, snapTo)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    double tolerance;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od", &zobj,
            &tolerance) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSSnap_r(GEOS_G(handle), this, other, tolerance);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

#ifdef HAVE_GEOS_NODE
PHP_METHOD(Geometry, node)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSNode_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif



/* -- Items 3..11: scalar wrappers on GEOSGeometry -------------------- */

/**
 * GEOSGeometry::makeValid(array $params = [])
 *
 * Empty params -> GEOSMakeValid_r.
 * Otherwise, build a GEOSMakeValidParams via setMethod / setKeepCollapsed
 * and invoke GEOSMakeValidWithParams_r.
 *
 * Recognised keys:
 *   'method'         => "linework" or "structure" (string)
 *   'keep_collapsed' => bool
 */
PHP_METHOD(Geometry, makeValid)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    zval *params_val = NULL;
    HashTable *params;
    GEOS_PHP_ZVAL data;
    zend_string *key;
    zend_ulong index;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a",
            &params_val) == FAILURE) {
        RETURN_NULL();
    }

    if (!params_val || zend_hash_num_elements(HASH_OF(params_val)) == 0) {
        ret = GEOSMakeValid_r(GEOS_G(handle), this);
    } else {
        GEOSMakeValidParams *mvp = GEOSMakeValidParams_create_r(GEOS_G(handle));
        if (!mvp) RETURN_NULL();

        params = HASH_OF(params_val);
        zend_hash_internal_pointer_reset(params);
        while (GEOS_PHP_HASH_GET_CUR_KEY(params, &key, &index)
               == HASH_KEY_IS_STRING)
        {
            if (!strcmp(ZSTR_VAL(key), "method")) {
                zval tmp;
                int methodValue = GEOS_MAKE_VALID_LINEWORK;
                GEOS_PHP_HASH_GET_CUR_DATA(params, data);
#if PHP_VERSION_ID >= 70000
                tmp = *data;
#else
                tmp = **data;
#endif
                zval_copy_ctor(&tmp);
                convert_to_string(&tmp);
                if (!strcmp(Z_STRVAL(tmp), "structure")) {
                    methodValue = GEOS_MAKE_VALID_STRUCTURE;
                } else if (!strcmp(Z_STRVAL(tmp), "linework")) {
                    methodValue = GEOS_MAKE_VALID_LINEWORK;
                } else {
                    zval_dtor(&tmp);
                    GEOSMakeValidParams_destroy_r(GEOS_G(handle), mvp);
                    zend_throw_exception_ex(
                        zend_exception_get_default(TSRMLS_C), 1 TSRMLS_CC,
                        "makeValid 'method' must be 'linework' or 'structure'");
                    RETURN_NULL();
                }
                zval_dtor(&tmp);
                GEOSMakeValidParams_setMethod_r(GEOS_G(handle), mvp,
                    (enum GEOSMakeValidMethods)methodValue);
            } else if (!strcmp(ZSTR_VAL(key), "keep_collapsed")) {
                long bv;
                GEOS_PHP_HASH_GET_CUR_DATA(params, data);
                bv = getZvalAsLong(data);
                GEOSMakeValidParams_setKeepCollapsed_r(GEOS_G(handle), mvp,
                    bv ? 1 : 0);
            }
            zend_hash_move_forward(params);
        }

        ret = GEOSMakeValidWithParams_r(GEOS_G(handle), this, mvp);
        GEOSMakeValidParams_destroy_r(GEOS_G(handle), mvp);
    }

    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::concaveHull(float $ratio, bool $allowHoles = false)
 */
PHP_METHOD(Geometry, concaveHull)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double ratio;
    zend_bool allowHoles = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &ratio, &allowHoles) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSConcaveHull_r(GEOS_G(handle), this, ratio, allowHoles ? 1 : 0);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::concaveHullByLength(float $maxLength, bool $allowHoles = false)
 */
PHP_METHOD(Geometry, concaveHullByLength)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double maxLength;
    zend_bool allowHoles = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &maxLength, &allowHoles) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSConcaveHullByLength_r(GEOS_G(handle), this, maxLength,
        allowHoles ? 1 : 0);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::concaveHullOfPolygons(float $ratio,
 *   bool $isTight = true, bool $allowHoles = false)
 */
PHP_METHOD(Geometry, concaveHullOfPolygons)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double ratio;
    zend_bool isTight = 1;
    zend_bool allowHoles = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|bb",
            &ratio, &isTight, &allowHoles) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSConcaveHullOfPolygons_r(GEOS_G(handle), this, ratio,
        isTight ? 1 : 0, allowHoles ? 1 : 0);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::polygonHullSimplify(bool $isOuter, float $param,
 *   ?int $mode = null)
 */
PHP_METHOD(Geometry, polygonHullSimplify)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    zend_bool isOuter;
    double param;
    zval *modeZv = NULL;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "bd|z!",
            &isOuter, &param, &modeZv) == FAILURE) {
        RETURN_NULL();
    }

    if (modeZv && Z_TYPE_P(modeZv) != IS_NULL) {
        long mode;
        zval tmp;
        tmp = *modeZv;
        zval_copy_ctor(&tmp);
        convert_to_long(&tmp);
        mode = Z_LVAL(tmp);
        zval_dtor(&tmp);
        ret = GEOSPolygonHullSimplifyMode_r(GEOS_G(handle), this,
            isOuter ? 1 : 0, (unsigned int)mode, param);
    } else {
        ret = GEOSPolygonHullSimplify_r(GEOS_G(handle), this,
            isOuter ? 1 : 0, param);
    }
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/* -- Item 8: Hull / metric extras -------------------- */

PHP_METHOD(Geometry, minimumBoundingCircle)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    GEOSGeometry *center = NULL;
    double radius = 0.0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    /* GEOSMinimumBoundingCircle_r writes to radius and *center. We don't
     * expose them here (GEOSGeometry::minimumBoundingCircle returns the
     * circle geometry only); just destroy the center geom afterwards. */
    ret = GEOSMinimumBoundingCircle_r(GEOS_G(handle), this, &radius, &center);
    if (center) GEOSGeom_destroy_r(GEOS_G(handle), center);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, minimumRotatedRectangle)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSMinimumRotatedRectangle_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, minimumWidth)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSMinimumWidth_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, minimumClearance)
{
    GEOSGeometry *this;
    double d;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    rc = GEOSMinimumClearance_r(GEOS_G(handle), this, &d);
    if (rc != 0) RETURN_NULL();
    RETURN_DOUBLE(d);
}

PHP_METHOD(Geometry, minimumClearanceLine)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSMinimumClearanceLine_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, maximumInscribedCircle)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double tol;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &tol) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSMaximumInscribedCircle_r(GEOS_G(handle), this, tol);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::largestEmptyCircle(float $tol, ?GEOSGeometry $boundary = null)
 *
 * Order matches the natural required-then-optional pattern.
 */
PHP_METHOD(Geometry, largestEmptyCircle)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    GEOSGeometry *boundary = NULL;
    double tol;
    zval *zobj = NULL;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|o!",
            &tol, &zobj) == FAILURE) {
        RETURN_NULL();
    }

    if (zobj) boundary = getRelay(zobj, Geometry_ce_ptr);
    ret = GEOSLargestEmptyCircle_r(GEOS_G(handle), this, boundary, tol);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/* -- Item 9: Distance / metric extras -------------------- */

PHP_METHOD(Geometry, distanceWithin)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double maxDist;
    char ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od",
            &zobj, &maxDist) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDistanceWithin_r(GEOS_G(handle), this, other, maxDist);
    if (ret == 2) RETURN_NULL();
    retBool = ret;
    RETURN_BOOL(retBool);
}

PHP_METHOD(Geometry, distanceIndexed)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    rc = GEOSDistanceIndexed_r(GEOS_G(handle), this, other, &dist);
    if (rc != 1) RETURN_NULL();
    RETURN_DOUBLE(dist);
}

/**
 * GEOSGeometry::nearestPoints(GEOSGeometry $other) : GEOSCoordSeq
 *
 * GEOSNearestPoints_r returns a NEW coord-seq that the caller owns; we
 * wrap it directly via setRelay (no clone) per CoordSeq lifecycle rule 1.
 */
PHP_METHOD(Geometry, nearestPoints)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSCoordSequence *cs;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    cs = GEOSNearestPoints_r(GEOS_G(handle), this, other);
    if ( ! cs ) RETURN_NULL();

    object_init_ex(return_value, CoordSeq_ce_ptr);
    setRelay(return_value, cs);
}

PHP_METHOD(Geometry, frechetDistance)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    rc = GEOSFrechetDistance_r(GEOS_G(handle), this, other, &dist);
    if (rc != 1) RETURN_NULL();
    RETURN_DOUBLE(dist);
}

PHP_METHOD(Geometry, frechetDistanceDensify)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist, frac;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od",
            &zobj, &frac) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    rc = GEOSFrechetDistanceDensify_r(GEOS_G(handle), this, other, frac, &dist);
    if (rc != 1) RETURN_NULL();
    RETURN_DOUBLE(dist);
}

PHP_METHOD(Geometry, hausdorffDistanceDensify)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist, frac;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od",
            &zobj, &frac) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    rc = GEOSHausdorffDistanceDensify_r(GEOS_G(handle), this, other, frac, &dist);
    if (rc != 1) RETURN_NULL();
    RETURN_DOUBLE(dist);
}

/* -- Item 10: Linear referencing extras -------------------- */

PHP_METHOD(Geometry, lineSubstring)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double startFrac, endFrac;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd",
            &startFrac, &endFrac) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSLineSubstring_r(GEOS_G(handle), this, startFrac, endFrac);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, lineMergeDirected)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSLineMergeDirected_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/* -- Item 11: Construction / utility extras -------------------- */

PHP_METHOD(Geometry, reverse)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSReverse_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, densify)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double dist;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &dist) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSDensify_r(GEOS_G(handle), this, dist);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, removeRepeatedPoints)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double tol = 0.0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|d", &tol) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSRemoveRepeatedPoints_r(GEOS_G(handle), this, tol);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::orientPolygons(bool $exterior_cw = false)
 *
 * Mutates a CLONE of the source geometry; returns the (oriented) clone.
 * GEOSOrientPolygons_r operates in-place, so we clone first so the input
 * geometry remains immutable from a PHP-API perspective.
 */
PHP_METHOD(Geometry, orientPolygons)
{
    GEOSGeometry *this;
    GEOSGeometry *clone;
    zend_bool exterior_cw = 0;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b",
            &exterior_cw) == FAILURE) {
        RETURN_NULL();
    }

    clone = GEOSGeom_clone_r(GEOS_G(handle), this);
    if ( ! clone ) RETURN_NULL();

    rc = GEOSOrientPolygons_r(GEOS_G(handle), clone, exterior_cw ? 1 : 0);
    if (rc < 0) {
        GEOSGeom_destroy_r(GEOS_G(handle), clone);
        RETURN_NULL();
    }

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, clone);
}

PHP_METHOD(Geometry, equalsIdentical)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    char ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSEqualsIdentical_r(GEOS_G(handle), this, other);
    if (ret == 2) RETURN_NULL();
    retBool = ret;
    RETURN_BOOL(retBool);
}

PHP_METHOD(Geometry, getXMin)
{
    GEOSGeometry *this;
    double v;
    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    if (GEOSGeom_getXMin_r(GEOS_G(handle), this, &v) == 0) RETURN_NULL();
    RETURN_DOUBLE(v);
}

PHP_METHOD(Geometry, getXMax)
{
    GEOSGeometry *this;
    double v;
    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    if (GEOSGeom_getXMax_r(GEOS_G(handle), this, &v) == 0) RETURN_NULL();
    RETURN_DOUBLE(v);
}

PHP_METHOD(Geometry, getYMin)
{
    GEOSGeometry *this;
    double v;
    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    if (GEOSGeom_getYMin_r(GEOS_G(handle), this, &v) == 0) RETURN_NULL();
    RETURN_DOUBLE(v);
}

PHP_METHOD(Geometry, getYMax)
{
    GEOSGeometry *this;
    double v;
    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    if (GEOSGeom_getYMax_r(GEOS_G(handle), this, &v) == 0) RETURN_NULL();
    RETURN_DOUBLE(v);
}

PHP_METHOD(Geometry, getExtent)
{
    GEOSGeometry *this;
    double xmin, ymin, xmax, ymax;
    int rc;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    rc = GEOSGeom_getExtent_r(GEOS_G(handle), this,
        &xmin, &ymin, &xmax, &ymax);
    if (rc == 0) RETURN_NULL();

    array_init(return_value);
    add_assoc_double(return_value, "xmin", xmin);
    add_assoc_double(return_value, "ymin", ymin);
    add_assoc_double(return_value, "xmax", xmax);
    add_assoc_double(return_value, "ymax", ymax);
}

PHP_METHOD(Geometry, buildArea)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    ret = GEOSBuildArea_r(GEOS_G(handle), this);
    if ( ! ret ) RETURN_NULL();
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/* -- class GEOSCoordSeq -------------------- */

/*
 * Lifecycle / ownership invariants for GEOSCoordSeq:
 *
 *  1. A PHP GEOSCoordSeq object owns its underlying GEOSCoordSequence*. The
 *     dtor calls GEOSCoordSeq_destroy_r on the relay pointer (if non-NULL).
 *
 *  2. GEOS C API functions that build a geometry from a coord-seq
 *     (GEOSGeom_createPoint_r, _createLineString_r, _createLinearRing_r,
 *     curved-geometry constructors, etc.) TRANSFER ownership of the coord-seq
 *     into the new geometry. After such a handoff the PHP coord-seq object's
 *     relay must be set to NULL via setRelay(zv, NULL) so the dtor does not
 *     double-free. (Used by item 12/16 factory statics.)
 *
 *  3. GEOSGeom_getCoordSeq_r returns a NON-OWNING const pointer to a coord-seq
 *     internal to a geometry. To wrap that as a PHP GEOSCoordSeq object we
 *     must FIRST clone via GEOSCoordSeq_clone_r so the PHP object owns its
 *     own copy. See GEOSGeometry::getCoordSeq() below for an example.
 */

PHP_METHOD(CoordSeq, __construct);
PHP_METHOD(CoordSeq, getSize);
PHP_METHOD(CoordSeq, getDimensions);
PHP_METHOD(CoordSeq, hasZ);
PHP_METHOD(CoordSeq, hasM);
PHP_METHOD(CoordSeq, isCCW);
PHP_METHOD(CoordSeq, setX);
PHP_METHOD(CoordSeq, setY);
PHP_METHOD(CoordSeq, setZ);
PHP_METHOD(CoordSeq, setM);
PHP_METHOD(CoordSeq, getX);
PHP_METHOD(CoordSeq, getY);
PHP_METHOD(CoordSeq, getZ);
PHP_METHOD(CoordSeq, getM);
PHP_METHOD(CoordSeq, setXY);
PHP_METHOD(CoordSeq, setXYZ);
PHP_METHOD(CoordSeq, getXY);
PHP_METHOD(CoordSeq, getXYZ);
PHP_METHOD(CoordSeq, copyFromArrays);
PHP_METHOD(CoordSeq, copyToArrays);

static zend_function_entry CoordSeq_methods[] = {
    PHP_ME(CoordSeq, __construct, arginfo_CoordSeq_construct, 0)
    PHP_ME(CoordSeq, getSize, arginfo_CoordSeq_getSize, 0)
    PHP_ME(CoordSeq, getDimensions, arginfo_CoordSeq_getDimensions, 0)
    PHP_ME(CoordSeq, hasZ, arginfo_CoordSeq_hasZ, 0)
    PHP_ME(CoordSeq, hasM, arginfo_CoordSeq_hasM, 0)
    PHP_ME(CoordSeq, isCCW, arginfo_CoordSeq_isCCW, 0)
    PHP_ME(CoordSeq, setX, arginfo_CoordSeq_setX, 0)
    PHP_ME(CoordSeq, setY, arginfo_CoordSeq_setY, 0)
    PHP_ME(CoordSeq, setZ, arginfo_CoordSeq_setZ, 0)
    PHP_ME(CoordSeq, setM, arginfo_CoordSeq_setM, 0)
    PHP_ME(CoordSeq, getX, arginfo_CoordSeq_getX, 0)
    PHP_ME(CoordSeq, getY, arginfo_CoordSeq_getY, 0)
    PHP_ME(CoordSeq, getZ, arginfo_CoordSeq_getZ, 0)
    PHP_ME(CoordSeq, getM, arginfo_CoordSeq_getM, 0)
    PHP_ME(CoordSeq, setXY, arginfo_CoordSeq_setXY, 0)
    PHP_ME(CoordSeq, setXYZ, arginfo_CoordSeq_setXYZ, 0)
    PHP_ME(CoordSeq, getXY, arginfo_CoordSeq_getXY, 0)
    PHP_ME(CoordSeq, getXYZ, arginfo_CoordSeq_getXYZ, 0)
    PHP_ME(CoordSeq, copyFromArrays, arginfo_CoordSeq_copyFromArrays, 0)
    PHP_ME(CoordSeq, copyToArrays, arginfo_CoordSeq_copyToArrays, 0)
    {NULL, NULL, NULL}
};

/* CoordSeq_ce_ptr is forward-declared near Geometry_ce_ptr. */

static zend_object_handlers CoordSeq_object_handlers;

static void
CoordSeq_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSCoordSequence *seq = (GEOSCoordSequence*)obj->relay;
    if (seq) {
        GEOSCoordSeq_destroy_r(GEOS_G(handle), seq);
    }

#if PHP_VERSION_ID < 70000
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
CoordSeq_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, CoordSeq_dtor, &CoordSeq_object_handlers);
}

/* Helper: bounds-check an index against the coord-seq size. Returns
 * 0 on success, throws and returns 1 on failure. */
static int
CoordSeq_checkIndex(GEOSCoordSequence *seq, long idx)
{
    TSRMLS_FETCH();
    unsigned int size = 0;

    if (idx < 0) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC, "GEOSCoordSeq index %ld is negative", idx);
        return 1;
    }
    if (!GEOSCoordSeq_getSize_r(GEOS_G(handle), seq, &size)) {
        return 1; /* GEOS will have thrown */
    }
    if ((unsigned int)idx >= size) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq index %ld out of range [0..%u)", idx, size);
        return 1;
    }
    return 0;
}

PHP_METHOD(CoordSeq, __construct)
{
    GEOSCoordSequence *seq;
    zval *object = getThis();
    zend_long size;
    zend_long dim = 2;
    zend_bool hasM = 0;
    int hasZ;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|lb",
            &size, &dim, &hasM) == FAILURE) {
        RETURN_NULL();
    }

    if (size < 0) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC, "GEOSCoordSeq size cannot be negative");
        RETURN_NULL();
    }
    if (dim != 2 && dim != 3) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq dim must be 2 or 3 (got %ld)", (long)dim);
        RETURN_NULL();
    }

    hasZ = (dim == 3) ? 1 : 0;

    seq = GEOSCoordSeq_createWithDimensions_r(GEOS_G(handle),
            (unsigned int)size, hasZ, hasM ? 1 : 0);
    if ( ! seq ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSCoordSeq_createWithDimensions() failed (didn't initGEOS?)");
        RETURN_NULL();
    }

    setRelay(object, seq);
}

PHP_METHOD(CoordSeq, getSize)
{
    GEOSCoordSequence *seq;
    unsigned int size = 0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (!GEOSCoordSeq_getSize_r(GEOS_G(handle), seq, &size)) {
        RETURN_NULL();
    }

    RETURN_LONG((long)size);
}

PHP_METHOD(CoordSeq, getDimensions)
{
    GEOSCoordSequence *seq;
    unsigned int dims = 0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (!GEOSCoordSeq_getDimensions_r(GEOS_G(handle), seq, &dims)) {
        RETURN_NULL();
    }

    RETURN_LONG((long)dims);
}

PHP_METHOD(CoordSeq, hasZ)
{
    GEOSCoordSequence *seq;
    char ret;
    zend_bool retBool;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    ret = GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq);
    retBool = ret ? 1 : 0;
    RETURN_BOOL(retBool);
}

PHP_METHOD(CoordSeq, hasM)
{
    GEOSCoordSequence *seq;
    char ret;
    zend_bool retBool;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    ret = GEOSCoordSeq_hasM_r(GEOS_G(handle), seq);
    retBool = ret ? 1 : 0;
    RETURN_BOOL(retBool);
}

PHP_METHOD(CoordSeq, isCCW)
{
    GEOSCoordSequence *seq;
    char is_ccw = 0;
    zend_bool retBool;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (!GEOSCoordSeq_isCCW_r(GEOS_G(handle), seq, &is_ccw)) {
        RETURN_NULL();
    }

    retBool = is_ccw ? 1 : 0;
    RETURN_BOOL(retBool);
}

PHP_METHOD(CoordSeq, setX)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld",
            &idx, &val) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setX_r(GEOS_G(handle), seq, (unsigned int)idx, val)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, setY)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld",
            &idx, &val) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setY_r(GEOS_G(handle), seq, (unsigned int)idx, val)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, setZ)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld",
            &idx, &val) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::setZ called on a sequence without Z");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setZ_r(GEOS_G(handle), seq, (unsigned int)idx, val)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, setM)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld",
            &idx, &val) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasM_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::setM called on a sequence without M");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setM_r(GEOS_G(handle), seq, (unsigned int)idx, val)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, getX)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getX_r(GEOS_G(handle), seq, (unsigned int)idx, &val)) {
        RETURN_NULL();
    }

    RETURN_DOUBLE(val);
}

PHP_METHOD(CoordSeq, getY)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getY_r(GEOS_G(handle), seq, (unsigned int)idx, &val)) {
        RETURN_NULL();
    }

    RETURN_DOUBLE(val);
}

PHP_METHOD(CoordSeq, getZ)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::getZ called on a sequence without Z");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getZ_r(GEOS_G(handle), seq, (unsigned int)idx, &val)) {
        RETURN_NULL();
    }

    RETURN_DOUBLE(val);
}

PHP_METHOD(CoordSeq, getM)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double val = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasM_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::getM called on a sequence without M");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getM_r(GEOS_G(handle), seq, (unsigned int)idx, &val)) {
        RETURN_NULL();
    }

    RETURN_DOUBLE(val);
}

PHP_METHOD(CoordSeq, setXY)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double x, y;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ldd",
            &idx, &x, &y) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setXY_r(GEOS_G(handle), seq, (unsigned int)idx, x, y)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, setXYZ)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double x, y, z;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lddd",
            &idx, &x, &y, &z) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::setXYZ called on a sequence without Z");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_setXYZ_r(GEOS_G(handle), seq, (unsigned int)idx, x, y, z)) {
        RETURN_NULL();
    }
}

PHP_METHOD(CoordSeq, getXY)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double x = 0.0, y = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getXY_r(GEOS_G(handle), seq, (unsigned int)idx, &x, &y)) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_double(return_value, "x", x);
    add_assoc_double(return_value, "y", y);
}

PHP_METHOD(CoordSeq, getXYZ)
{
    GEOSCoordSequence *seq;
    zend_long idx;
    double x = 0.0, y = 0.0, z = 0.0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &idx) == FAILURE) {
        RETURN_NULL();
    }

    if (!GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq)) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::getXYZ called on a sequence without Z");
        RETURN_NULL();
    }
    if (CoordSeq_checkIndex(seq, idx)) RETURN_NULL();

    if (!GEOSCoordSeq_getXYZ_r(GEOS_G(handle), seq, (unsigned int)idx, &x, &y, &z)) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_double(return_value, "x", x);
    add_assoc_double(return_value, "y", y);
    add_assoc_double(return_value, "z", z);
}

/* Helper: copy a PHP numeric array into a freshly emalloc()'d double[].
 * Returns the pointer (caller must efree) and writes the size. Returns NULL
 * on failure (an exception will have been thrown). */
static double *
CoordSeq_zvalArrayToDoubles(zval *arr, unsigned int *out_size)
{
    TSRMLS_FETCH();
    HashTable *ht;
    double *buf;
    unsigned int n, i;
    GEOS_PHP_ZVAL data;

    ht = HASH_OF(arr);
    if (!ht) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC, "GEOSCoordSeq::copyFromArrays expected an array");
        return NULL;
    }
    n = zend_hash_num_elements(ht);
    *out_size = n;
    if (n == 0) return NULL; /* legitimate empty: caller treats as no-op */

    buf = (double*)emalloc(sizeof(double) * n);

    zend_hash_internal_pointer_reset(ht);
    for (i = 0; i < n; ++i) {
        if (!GEOS_PHP_HASH_GET_CUR_DATA(ht, data)) {
            efree(buf);
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                1 TSRMLS_CC,
                "GEOSCoordSeq::copyFromArrays: failed reading element %u",
                i);
            return NULL;
        }
        buf[i] = getZvalAsDouble(data);
        zend_hash_move_forward(ht);
    }

    return buf;
}

PHP_METHOD(CoordSeq, copyFromArrays)
{
    GEOSCoordSequence *seq;
    zval *xv = NULL, *yv = NULL, *zv = NULL, *mv = NULL;
    double *xa = NULL, *ya = NULL, *za = NULL, *ma = NULL;
    unsigned int xn = 0, yn = 0, zn = 0, mn = 0;
    GEOSCoordSequence *newseq;
    unsigned int seqsize = 0;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aa|a!a!",
            &xv, &yv, &zv, &mv) == FAILURE) {
        RETURN_NULL();
    }

    /* Convert input arrays */
    xa = CoordSeq_zvalArrayToDoubles(xv, &xn);
    if (!xa && xn > 0) RETURN_NULL();
    ya = CoordSeq_zvalArrayToDoubles(yv, &yn);
    if (!ya && yn > 0) { if (xa) efree(xa); RETURN_NULL(); }

    if (xn != yn) {
        if (xa) efree(xa);
        if (ya) efree(ya);
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::copyFromArrays: x/y array sizes differ (%u vs %u)",
            xn, yn);
        RETURN_NULL();
    }

    if (zv) {
        za = CoordSeq_zvalArrayToDoubles(zv, &zn);
        if (!za && zn > 0) {
            if (xa) efree(xa); if (ya) efree(ya);
            RETURN_NULL();
        }
        if (zn != xn) {
            if (xa) efree(xa); if (ya) efree(ya); if (za) efree(za);
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                1 TSRMLS_CC,
                "GEOSCoordSeq::copyFromArrays: z array size %u != x size %u",
                zn, xn);
            RETURN_NULL();
        }
    }

    if (mv) {
        ma = CoordSeq_zvalArrayToDoubles(mv, &mn);
        if (!ma && mn > 0) {
            if (xa) efree(xa); if (ya) efree(ya); if (za) efree(za);
            RETURN_NULL();
        }
        if (mn != xn) {
            if (xa) efree(xa); if (ya) efree(ya);
            if (za) efree(za); if (ma) efree(ma);
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                1 TSRMLS_CC,
                "GEOSCoordSeq::copyFromArrays: m array size %u != x size %u",
                mn, xn);
            RETURN_NULL();
        }
    }

    /* Verify the destination has matching size */
    if (!GEOSCoordSeq_getSize_r(GEOS_G(handle), seq, &seqsize)) {
        if (xa) efree(xa); if (ya) efree(ya);
        if (za) efree(za); if (ma) efree(ma);
        RETURN_NULL();
    }
    if (seqsize != xn) {
        if (xa) efree(xa); if (ya) efree(ya);
        if (za) efree(za); if (ma) efree(ma);
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSCoordSeq::copyFromArrays: input array size %u != coord-seq size %u",
            xn, seqsize);
        RETURN_NULL();
    }

    /* Build a new coord-seq from arrays, then swap into our relay
     * (destroy the old one). GEOSCoordSeq_copyFromArrays_r does not
     * accept an existing destination buffer.
     */
    newseq = GEOSCoordSeq_copyFromArrays_r(GEOS_G(handle),
            xa, ya, za, ma, xn);

    if (xa) efree(xa);
    if (ya) efree(ya);
    if (za) efree(za);
    if (ma) efree(ma);

    if (!newseq) RETURN_NULL();

    /* Replace existing coord-seq with the new one. */
    GEOSCoordSeq_destroy_r(GEOS_G(handle), seq);
    setRelay(getThis(), newseq);
}

PHP_METHOD(CoordSeq, copyToArrays)
{
    GEOSCoordSequence *seq;
    unsigned int size = 0, i;
    char hasZ, hasM;
    double *xa = NULL, *ya = NULL, *za = NULL, *ma = NULL;
    zval *xarr, *yarr, *zarr = NULL, *marr = NULL;

    seq = (GEOSCoordSequence*)getRelay(getThis(), CoordSeq_ce_ptr);

    if (!GEOSCoordSeq_getSize_r(GEOS_G(handle), seq, &size)) {
        RETURN_NULL();
    }
    hasZ = GEOSCoordSeq_hasZ_r(GEOS_G(handle), seq);
    hasM = GEOSCoordSeq_hasM_r(GEOS_G(handle), seq);

    /* GEOSCoordSeq_copyToArrays_r requires non-NULL buffers when the
     * sequence has the corresponding ordinate. Allocate accordingly. */
    if (size > 0) {
        xa = (double*)emalloc(sizeof(double) * size);
        ya = (double*)emalloc(sizeof(double) * size);
        if (hasZ) za = (double*)emalloc(sizeof(double) * size);
        if (hasM) ma = (double*)emalloc(sizeof(double) * size);
    }

    if (size > 0 && !GEOSCoordSeq_copyToArrays_r(GEOS_G(handle), seq,
                xa, ya, za, ma)) {
        if (xa) efree(xa); if (ya) efree(ya);
        if (za) efree(za); if (ma) efree(ma);
        RETURN_NULL();
    }

    array_init(return_value);

#if PHP_VERSION_ID >= 70000
    {
        zval xz, yz;
        array_init(&xz);
        array_init(&yz);
        for (i = 0; i < size; ++i) {
            add_next_index_double(&xz, xa[i]);
            add_next_index_double(&yz, ya[i]);
        }
        add_assoc_zval(return_value, "x", &xz);
        add_assoc_zval(return_value, "y", &yz);
        if (hasZ) {
            zval zz;
            array_init(&zz);
            for (i = 0; i < size; ++i) add_next_index_double(&zz, za[i]);
            add_assoc_zval(return_value, "z", &zz);
        }
        if (hasM) {
            zval mz;
            array_init(&mz);
            for (i = 0; i < size; ++i) add_next_index_double(&mz, ma[i]);
            add_assoc_zval(return_value, "m", &mz);
        }
    }
#else
    MAKE_STD_ZVAL(xarr); array_init(xarr);
    MAKE_STD_ZVAL(yarr); array_init(yarr);
    for (i = 0; i < size; ++i) {
        add_next_index_double(xarr, xa[i]);
        add_next_index_double(yarr, ya[i]);
    }
    add_assoc_zval(return_value, "x", xarr);
    add_assoc_zval(return_value, "y", yarr);
    if (hasZ) {
        MAKE_STD_ZVAL(zarr); array_init(zarr);
        for (i = 0; i < size; ++i) add_next_index_double(zarr, za[i]);
        add_assoc_zval(return_value, "z", zarr);
    }
    if (hasM) {
        MAKE_STD_ZVAL(marr); array_init(marr);
        for (i = 0; i < size; ++i) add_next_index_double(marr, ma[i]);
        add_assoc_zval(return_value, "m", marr);
    }
    /* silence unused-warning for the !=PHP7 branch. */
    (void)xarr; (void)yarr; (void)zarr; (void)marr;
#endif

    if (xa) efree(xa); if (ya) efree(ya);
    if (za) efree(za); if (ma) efree(ma);
}

/* GEOSGeometry::getCoordSeq()
 *
 * Returns a *cloned* GEOSCoordSeq representing the geometry's
 * coordinate sequence. Only valid for Point/LineString/LinearRing.
 */
PHP_METHOD(Geometry, getCoordSeq)
{
    GEOSGeometry *geom;
    const GEOSCoordSequence *cs;
    GEOSCoordSequence *clone;
    int typId;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    typId = GEOSGeomTypeId_r(GEOS_G(handle), geom);
    if (typId != GEOS_POINT && typId != GEOS_LINESTRING
            && typId != GEOS_LINEARRING) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
            1 TSRMLS_CC,
            "GEOSGeometry::getCoordSeq is only valid for Point, LineString, "
            "or LinearRing (got typeId=%d)", typId);
        RETURN_NULL();
    }

    cs = GEOSGeom_getCoordSeq_r(GEOS_G(handle), geom);
    if (!cs) RETURN_NULL(); /* exception will have been thrown */

    /* GEOSGeom_getCoordSeq_r returns a non-owning pointer to the
     * geometry's internal coord-seq. We must clone it before handing
     * it to a PHP object that owns its relay. */
    clone = GEOSCoordSeq_clone_r(GEOS_G(handle), cs);
    if (!clone) RETURN_NULL();

    object_init_ex(return_value, CoordSeq_ce_ptr);
    setRelay(return_value, clone);
}

/* -- class GEOSWKTReader -------------------- */

PHP_METHOD(WKTReader, __construct);
PHP_METHOD(WKTReader, read);
PHP_METHOD(WKTReader, setFixStructure);

static zend_function_entry WKTReader_methods[] = {
    PHP_ME(WKTReader, __construct, arginfo_WKTReader_construct, 0)
    PHP_ME(WKTReader, read, arginfo_WKTReader_read, 0)
    PHP_ME(WKTReader, setFixStructure, arginfo_WKTReader_setFixStructure, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKTReader_ce_ptr;

static zend_object_handlers WKTReader_object_handlers;

static void
WKTReader_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSWKTReader *reader = (GEOSWKTReader*)obj->relay;
    if (reader) {
        GEOSWKTReader_destroy_r(GEOS_G(handle), reader);
    }

#if PHP_VERSION_ID < 70000
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
WKTReader_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKTReader_dtor, &WKTReader_object_handlers);
}


PHP_METHOD(WKTReader, __construct)
{
    GEOSWKTReader* obj;
    zval *object = getThis();

    obj = GEOSWKTReader_create_r(GEOS_G(handle));
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKTReader_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKTReader, read)
{
    GEOSWKTReader *reader;
    GEOSGeometry *geom;
    zend_string *wkt;
#if PHP_VERSION_ID < 70000
    int wktlen;
#endif

    reader = (GEOSWKTReader*)getRelay(getThis(), WKTReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
#if PHP_VERSION_ID >= 70000
            "S", &wkt
#else
            "s", &wkt, &wktlen
#endif
       ) == FAILURE)
    {
        RETURN_NULL();
    }

    geom = GEOSWKTReader_read_r(GEOS_G(handle), reader, ZSTR_VAL(wkt));
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}

PHP_METHOD(WKTReader, setFixStructure)
{
    GEOSWKTReader *reader;
    zend_bool fix;

    reader = (GEOSWKTReader*)getRelay(getThis(), WKTReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &fix) == FAILURE) {
        RETURN_NULL();
    }

    GEOSWKTReader_setFixStructure_r(GEOS_G(handle), reader, fix ? 1 : 0);
}

/* -- class GEOSWKTWriter -------------------- */

PHP_METHOD(WKTWriter, __construct);
PHP_METHOD(WKTWriter, write);

#ifdef HAVE_GEOS_WKT_WRITER_SET_TRIM
PHP_METHOD(WKTWriter, setTrim);
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_ROUNDING_PRECISION
PHP_METHOD(WKTWriter, setRoundingPrecision);
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_OUTPUT_DIMENSION
PHP_METHOD(WKTWriter, setOutputDimension);
#endif

#ifdef HAVE_GEOS_WKT_WRITER_GET_OUTPUT_DIMENSION
PHP_METHOD(WKTWriter, getOutputDimension);
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_OLD_3D
PHP_METHOD(WKTWriter, setOld3D);
#endif

static zend_function_entry WKTWriter_methods[] = {
    PHP_ME(WKTWriter, __construct, arginfo_WKTWriter_construct, 0)
    PHP_ME(WKTWriter, write, arginfo_WKTWriter_write, 0)

#   ifdef HAVE_GEOS_WKT_WRITER_SET_TRIM
    PHP_ME(WKTWriter, setTrim, arginfo_WKTWriter_setTrim, 0)
#   endif

#   ifdef HAVE_GEOS_WKT_WRITER_SET_ROUNDING_PRECISION
    PHP_ME(WKTWriter, setRoundingPrecision, arginfo_WKTWriter_setRoundingPrecision, 0)
#   endif

#   ifdef HAVE_GEOS_WKT_WRITER_SET_OUTPUT_DIMENSION
    PHP_ME(WKTWriter, setOutputDimension, arginfo_WKTWriter_setOutputDimension, 0)
#   endif

#   ifdef HAVE_GEOS_WKT_WRITER_GET_OUTPUT_DIMENSION
    PHP_ME(WKTWriter, getOutputDimension, arginfo_WKTWriter_getOutputDimension, 0)
#   endif

#   ifdef HAVE_GEOS_WKT_WRITER_SET_OLD_3D
    PHP_ME(WKTWriter, setOld3D, arginfo_WKTWriter_setOld3D, 0)
#   endif

    {NULL, NULL, NULL}
};

static zend_class_entry *WKTWriter_ce_ptr;

static zend_object_handlers WKTWriter_object_handlers;

static void
WKTWriter_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSWKTWriter_destroy_r(GEOS_G(handle), (GEOSWKTWriter*)obj->relay);

#if PHP_VERSION_ID >= 70000
    //zend_object_std_dtor(&obj->std);
#else
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
WKTWriter_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKTWriter_dtor, &WKTWriter_object_handlers);
}

PHP_METHOD(WKTWriter, __construct)
{
    GEOSWKTWriter* obj;
    zval *object = getThis();

    obj = GEOSWKTWriter_create_r(GEOS_G(handle));
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKTWriter_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKTWriter, write)
{
    GEOSWKTWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char* wkt;
    char* retstr;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    wkt = GEOSWKTWriter_write_r(GEOS_G(handle), writer, geom);
    /* we'll probably get an exception if wkt is null */
    if ( ! wkt ) RETURN_NULL();

    retstr = estrdup(wkt);
    GEOSFree_r(GEOS_G(handle), wkt);

    GEOS_PHP_RETURN_STRING(retstr);
}

#ifdef HAVE_GEOS_WKT_WRITER_SET_TRIM
PHP_METHOD(WKTWriter, setTrim)
{
    GEOSWKTWriter *writer;
    zend_bool trimval;
    char trim;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &trimval)
        == FAILURE)
    {
        RETURN_NULL();
    }

    trim = trimval;
    GEOSWKTWriter_setTrim_r(GEOS_G(handle), writer, trim);
}
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_ROUNDING_PRECISION
PHP_METHOD(WKTWriter, setRoundingPrecision)
{
    GEOSWKTWriter *writer;
    zend_long prec;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prec)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKTWriter_setRoundingPrecision_r(GEOS_G(handle), writer, prec);
}
#endif

/**
 * void GEOSWKTWriter::setOutputDimension()
 */
#ifdef HAVE_GEOS_WKT_WRITER_SET_OUTPUT_DIMENSION
PHP_METHOD(WKTWriter, setOutputDimension)
{
    GEOSWKTWriter *writer;
    zend_long dim;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKTWriter_setOutputDimension_r(GEOS_G(handle), writer, dim);
}
#endif

/**
 * long GEOSWKTWriter::getOutputDimension()
 */
#ifdef HAVE_GEOS_WKT_WRITER_GET_OUTPUT_DIMENSION
PHP_METHOD(WKTWriter, getOutputDimension)
{
    GEOSWKTWriter *writer;
    long int ret;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    ret = GEOSWKTWriter_getOutputDimension_r(GEOS_G(handle), writer);

    RETURN_LONG(ret);
}
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_OLD_3D
PHP_METHOD(WKTWriter, setOld3D)
{
    GEOSWKTWriter *writer;
    zend_bool bval;
    int val;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &bval)
        == FAILURE)
    {
        RETURN_NULL();
    }

    val = bval;
    GEOSWKTWriter_setOld3D_r(GEOS_G(handle), writer, val);
}
#endif

/* -- class GEOSWKBWriter -------------------- */

PHP_METHOD(WKBWriter, __construct);
PHP_METHOD(WKBWriter, getOutputDimension);
PHP_METHOD(WKBWriter, setOutputDimension);
PHP_METHOD(WKBWriter, getByteOrder);
PHP_METHOD(WKBWriter, setByteOrder);
PHP_METHOD(WKBWriter, setIncludeSRID);
PHP_METHOD(WKBWriter, getIncludeSRID);
PHP_METHOD(WKBWriter, write);
PHP_METHOD(WKBWriter, writeHEX);
PHP_METHOD(WKBWriter, setFlavor);
PHP_METHOD(WKBWriter, getFlavor);

static zend_function_entry WKBWriter_methods[] = {
    PHP_ME(WKBWriter, __construct, arginfo_WKBWriter_construct, 0)
    PHP_ME(WKBWriter, getOutputDimension, arginfo_WKBWriter_getOutputDimension, 0)
    PHP_ME(WKBWriter, setOutputDimension, arginfo_WKBWriter_setOutputDimension, 0)
    PHP_ME(WKBWriter, getByteOrder, arginfo_WKBWriter_getByteOrder, 0)
    PHP_ME(WKBWriter, setByteOrder, arginfo_WKBWriter_setByteOrder, 0)
    PHP_ME(WKBWriter, getIncludeSRID, arginfo_WKBWriter_getIncludeSRID, 0)
    PHP_ME(WKBWriter, setIncludeSRID, arginfo_WKBWriter_setIncludeSRID, 0)
    PHP_ME(WKBWriter, write, arginfo_WKBWriter_write, 0)
    PHP_ME(WKBWriter, writeHEX, arginfo_WKBWriter_writeHEX, 0)
    PHP_ME(WKBWriter, setFlavor, arginfo_WKBWriter_setFlavor, 0)
    PHP_ME(WKBWriter, getFlavor, arginfo_WKBWriter_getFlavor, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKBWriter_ce_ptr;

static zend_object_handlers WKBWriter_object_handlers;

static void
WKBWriter_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSWKBWriter_destroy_r(GEOS_G(handle), (GEOSWKBWriter*)obj->relay);

#if PHP_VERSION_ID >= 70000
    //zend_object_std_dtor(&obj->std);
#else
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
WKBWriter_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKBWriter_dtor, &WKBWriter_object_handlers);
}

/**
 * GEOSWKBWriter w = new GEOSWKBWriter()
 */
PHP_METHOD(WKBWriter, __construct)
{
    GEOSWKBWriter* obj;
    zval *object = getThis();

    obj = GEOSWKBWriter_create_r(GEOS_G(handle));
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKBWriter_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

/**
 * long GEOSWKBWriter::getOutputDimension();
 */
PHP_METHOD(WKBWriter, getOutputDimension)
{
    GEOSWKBWriter *writer;
    long int ret;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getOutputDimension_r(GEOS_G(handle), writer);

    RETURN_LONG(ret);
}

/**
 * void GEOSWKBWriter::setOutputDimension(dims);
 */
PHP_METHOD(WKBWriter, setOutputDimension)
{
    GEOSWKBWriter *writer;
    zend_long dim;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKBWriter_setOutputDimension_r(GEOS_G(handle), writer, dim);

}

/**
 * string GEOSWKBWriter::write(GEOSGeometry)
 */
PHP_METHOD(WKBWriter, write)
{
    GEOSWKBWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char *ret;
    size_t retsize;
    char* retstr;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    ret = (char*)GEOSWKBWriter_write_r(GEOS_G(handle), writer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) RETURN_NULL();

    retstr = estrndup(ret, retsize);
    GEOSFree_r(GEOS_G(handle), ret);

    GEOS_PHP_RETURN_STRINGL(retstr, retsize);
}

/**
 * string GEOSWKBWriter::writeHEX(GEOSGeometry)
 */
PHP_METHOD(WKBWriter, writeHEX)
{
    GEOSWKBWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char *ret;
    size_t retsize; /* useless... */
    char* retstr;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    ret = (char*)GEOSWKBWriter_writeHEX_r(GEOS_G(handle), writer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) RETURN_NULL();

    retstr = estrndup(ret, retsize);
    GEOSFree_r(GEOS_G(handle), ret);

    GEOS_PHP_RETURN_STRING(retstr);
}

/**
 * long GEOSWKBWriter::getByteOrder();
 */
PHP_METHOD(WKBWriter, getByteOrder)
{
    GEOSWKBWriter *writer;
    long int ret;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getByteOrder_r(GEOS_G(handle), writer);

    RETURN_LONG(ret);
}

/**
 * void GEOSWKBWriter::setByteOrder(dims);
 */
PHP_METHOD(WKBWriter, setByteOrder)
{
    GEOSWKBWriter *writer;
    zend_long dim;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKBWriter_setByteOrder_r(GEOS_G(handle), writer, dim);

}

/**
 * bool GEOSWKBWriter::getIncludeSRID();
 */
PHP_METHOD(WKBWriter, getIncludeSRID)
{
    GEOSWKBWriter *writer;
    int ret;
    zend_bool retBool;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getIncludeSRID_r(GEOS_G(handle), writer);
    retBool = ret;

    RETURN_BOOL(retBool);
}

/**
 * void GEOSWKBWriter::setIncludeSRID(bool);
 */
PHP_METHOD(WKBWriter, setIncludeSRID)
{
    GEOSWKBWriter *writer;
    int inc;
    zend_bool incVal;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &incVal)
        == FAILURE)
    {
        RETURN_NULL();
    }

    inc = incVal;
    GEOSWKBWriter_setIncludeSRID_r(GEOS_G(handle), writer, inc);
}

/**
 * void GEOSWKBWriter::setFlavor(int flavor);
 */
PHP_METHOD(WKBWriter, setFlavor)
{
    GEOSWKBWriter *writer;
    zend_long flavor;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flavor) == FAILURE) {
        RETURN_NULL();
    }

    GEOSWKBWriter_setFlavor_r(GEOS_G(handle), writer, (int)flavor);
}

/**
 * int GEOSWKBWriter::getFlavor();
 */
PHP_METHOD(WKBWriter, getFlavor)
{
    GEOSWKBWriter *writer;
    int ret;
    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);
    ret = GEOSWKBWriter_getFlavor_r(GEOS_G(handle), writer);
    RETURN_LONG(ret);
}

/* -- class GEOSWKBReader -------------------- */

PHP_METHOD(WKBReader, __construct);
PHP_METHOD(WKBReader, read);
PHP_METHOD(WKBReader, readHEX);
PHP_METHOD(WKBReader, setFixStructure);

static zend_function_entry WKBReader_methods[] = {
    PHP_ME(WKBReader, __construct, arginfo_WKBReader_construct, 0)
    PHP_ME(WKBReader, read, arginfo_WKBReader_read, 0)
    PHP_ME(WKBReader, readHEX, arginfo_WKBReader_readHEX, 0)
    PHP_ME(WKBReader, setFixStructure, arginfo_WKBReader_setFixStructure, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKBReader_ce_ptr;

static zend_object_handlers WKBReader_object_handlers;

static void
WKBReader_dtor (GEOS_PHP_DTOR_OBJECT *object TSRMLS_DC)
{
#if PHP_VERSION_ID < 70000
    Proxy *obj = (Proxy *)object;
#else
    Proxy *obj = php_geos_fetch_object(object);
#endif

    GEOSWKBReader_destroy_r(GEOS_G(handle), (GEOSWKBReader*)obj->relay);

#if PHP_VERSION_ID >= 70000
    //zend_object_std_dtor(&obj->std);
#else
    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
#endif
}

static zend_object_value
WKBReader_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKBReader_dtor, &WKBReader_object_handlers);
}


PHP_METHOD(WKBReader, __construct)
{
    GEOSWKBReader* obj;
    zval *object = getThis();

    obj = GEOSWKBReader_create_r(GEOS_G(handle));
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKBReader_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKBReader, read)
{
    GEOSWKBReader *reader;
    GEOSGeometry *geom;
    zend_string* wkb;
    int wkblen;

    reader = (GEOSWKBReader*)getRelay(getThis(), WKBReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
#if PHP_VERSION_ID >= 70000
            "S", &wkb
#else
            "s", &wkb, &wkblen
#endif
       ) == FAILURE)
    {
        RETURN_NULL();
    }

#if PHP_VERSION_ID >= 70000
    wkblen = ZSTR_LEN(wkb);
#endif

    geom = GEOSWKBReader_read_r(GEOS_G(handle), reader, (unsigned char*)ZSTR_VAL(wkb), wkblen);
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}

PHP_METHOD(WKBReader, readHEX)
{
    GEOSWKBReader *reader;
    GEOSGeometry *geom;
    unsigned char* wkb;
#if PHP_VERSION_ID >= 70000
    size_t wkblen;
#else
    int wkblen;
#endif

    reader = (GEOSWKBReader*)getRelay(getThis(), WKBReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
        &wkb, &wkblen) == FAILURE)
    {
        RETURN_NULL();
    }

    geom = GEOSWKBReader_readHEX_r(GEOS_G(handle), reader, wkb, wkblen);
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}

PHP_METHOD(WKBReader, setFixStructure)
{
    GEOSWKBReader *reader;
    zend_bool fix;

    reader = (GEOSWKBReader*)getRelay(getThis(), WKBReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &fix) == FAILURE) {
        RETURN_NULL();
    }

    GEOSWKBReader_setFixStructure_r(GEOS_G(handle), reader, fix ? 1 : 0);
}


/* -- Free functions ------------------------- */

/**
 * string GEOSVersion()
 */
PHP_FUNCTION(GEOSVersion)
{
    char *str;

    str = estrdup(GEOSversion());
    GEOS_PHP_RETURN_STRING(str);
}

/**
 * array GEOSPolygonize(GEOSGeometry $geom)
 *
 * The returned array contains the following elements:
 *
 *  - 'rings'
 *      Type: array of GEOSGeometry
 *      Rings that can be formed by the costituent
 *      linework of geometry.
 *  - 'cut_edges' (optional)
 *      Type: array of GEOSGeometry
 *      Edges which are connected at both ends but
 *      which do not form part of polygon.
 *  - 'dangles'
 *      Type: array of GEOSGeometry
 *      Edges which have one or both ends which are
 *      not incident on another edge endpoint
 *  - 'invalid_rings'
 *      Type: array of GEOSGeometry
 *      Edges which form rings which are invalid
 *      (e.g. the component lines contain a self-intersection)
 *
 */
PHP_FUNCTION(GEOSPolygonize)
{
    GEOSGeometry *this;
    GEOSGeometry *rings;
    GEOSGeometry *cut_edges;
    GEOSGeometry *dangles;
    GEOSGeometry *invalid_rings;
    zval *array_elem;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    this = getRelay(zobj, Geometry_ce_ptr);

    rings = GEOSPolygonize_full_r(GEOS_G(handle), this, &cut_edges, &dangles, &invalid_rings);
    if ( ! rings ) RETURN_NULL(); /* should get an exception first */

    /* return value should be an array */
    array_init(return_value);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(rings, array_elem);
    GEOSGeom_destroy_r(GEOS_G(handle), rings);
    GEOS_PHP_ADD_ASSOC_ZVAL(return_value, "rings", array_elem);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(cut_edges, array_elem);
    GEOSGeom_destroy_r(GEOS_G(handle), cut_edges);
    GEOS_PHP_ADD_ASSOC_ZVAL(return_value, "cut_edges", array_elem);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(dangles, array_elem);
    GEOSGeom_destroy_r(GEOS_G(handle), dangles);
    GEOS_PHP_ADD_ASSOC_ZVAL(return_value, "dangles", array_elem);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(invalid_rings, array_elem);
    GEOSGeom_destroy_r(GEOS_G(handle), invalid_rings);
    GEOS_PHP_ADD_ASSOC_ZVAL(return_value, "invalid_rings", array_elem);

}

/**
 * array GEOSPolygonizeValid(GEOSGeometry $geom)
 *
 * Returns an array of valid polygons formed from the linework of the input.
 */
PHP_FUNCTION(GEOSPolygonizeValid)
{
    GEOSGeometry *this;
    GEOSGeometry *out;
    const GEOSGeometry *input[1];
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    this = getRelay(zobj, Geometry_ce_ptr);
    input[0] = this;

    out = GEOSPolygonize_valid_r(GEOS_G(handle), input, 1);
    if ( ! out ) RETURN_NULL();

    array_init(return_value);
    dumpGeometry(out, return_value);
    GEOSGeom_destroy_r(GEOS_G(handle), out);
}

/**
 * array GEOSPolygonizeCutEdges(GEOSGeometry $geom)
 *
 * Returns an array of "cut" edges (edges connected at both ends but not
 * forming part of any polygon).
 */
PHP_FUNCTION(GEOSPolygonizeCutEdges)
{
    GEOSGeometry *this;
    GEOSGeometry *out;
    const GEOSGeometry *input[1];
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    this = getRelay(zobj, Geometry_ce_ptr);
    input[0] = this;

    out = GEOSPolygonizer_getCutEdges_r(GEOS_G(handle), input, 1);
    if ( ! out ) RETURN_NULL();

    array_init(return_value);
    dumpGeometry(out, return_value);
    GEOSGeom_destroy_r(GEOS_G(handle), out);
}

/**
 * GEOSGeometry GEOSBuildArea(GEOSGeometry $geom)
 */
PHP_FUNCTION(GEOSBuildArea)
{
    GEOSGeometry *geom_in;
    GEOSGeometry *geom_out;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in = getRelay(zobj, Geometry_ce_ptr);

    geom_out = GEOSBuildArea_r(GEOS_G(handle), geom_in);
    if ( ! geom_out ) RETURN_NULL();

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom_out);
}

/**
 * GEOSGeometry GEOSDisjointSubsetUnion(GEOSGeometry $geom)
 */
PHP_FUNCTION(GEOSDisjointSubsetUnion)
{
    GEOSGeometry *geom_in;
    GEOSGeometry *geom_out;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in = getRelay(zobj, Geometry_ce_ptr);

    geom_out = GEOSDisjointSubsetUnion_r(GEOS_G(handle), geom_in);
    if ( ! geom_out ) RETURN_NULL();

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom_out);
}

/**
 * array GEOSLineMerge(GEOSGeometry $geom)
 */
PHP_FUNCTION(GEOSLineMerge)
{
    GEOSGeometry *geom_in;
    GEOSGeometry *geom_out;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in = getRelay(zobj, Geometry_ce_ptr);

    geom_out = GEOSLineMerge_r(GEOS_G(handle), geom_in);
    if ( ! geom_out ) RETURN_NULL(); /* should get an exception first */

    /* return value should be an array */
    array_init(return_value);
    dumpGeometry(geom_out, return_value);
    GEOSGeom_destroy_r(GEOS_G(handle), geom_out);
}

/**
 * GEOSGeometry GEOSSharedPaths(GEOSGeometry $geom1, GEOSGeometry *geom2)
 */
#ifdef HAVE_GEOS_SHARED_PATHS
PHP_FUNCTION(GEOSSharedPaths)
{
    GEOSGeometry *geom_in_1;
    GEOSGeometry *geom_in_2;
    GEOSGeometry *geom_out;
    zval *zobj1, *zobj2;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &zobj1, &zobj2)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in_1 = getRelay(zobj1, Geometry_ce_ptr);
    geom_in_2 = getRelay(zobj2, Geometry_ce_ptr);

    geom_out = GEOSSharedPaths_r(GEOS_G(handle), geom_in_1, geom_in_2);
    if ( ! geom_out ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom_out);
}
#endif

/**
 * GEOSGeometry::delaunayTriangulation([<tolerance>], [<onlyEdges>])
 *
 *  'tolerance'
 *       Type: double
 *       snapping tolerance to use for improved robustness
 *  'onlyEdges'
 *       Type: boolean
 *       if true will return a MULTILINESTRING, otherwise (the default)
 *       it will return a GEOMETRYCOLLECTION containing triangular POLYGONs.
 */
#ifdef HAVE_GEOS_DELAUNAY_TRIANGULATION
PHP_METHOD(Geometry, delaunayTriangulation)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double tolerance = 0.0;
    zend_bool edgeonly = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|db",
            &tolerance, &edgeonly) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSDelaunayTriangulation_r(GEOS_G(handle), this, tolerance, edgeonly ? 1 : 0);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

/**
 * GEOSGeometry::voronoiDiagram([<tolerance>], [<onlyEdges>], [<extent>])
 *
 *  'tolerance'
 *       Type: double
 *       snapping tolerance to use for improved robustness
 *  'onlyEdges'
 *       Type: boolean
 *       if true will return a MULTILINESTRING, otherwise (the default)
 *       it will return a GEOMETRYCOLLECTION containing POLYGONs.
 *  'extent'
 *       Type: geometry
 *       Clip returned diagram by the extent of the given geometry
 */
#ifdef HAVE_GEOS_VORONOI_DIAGRAM
PHP_METHOD(Geometry, voronoiDiagram)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    zval *zobj = 0;
    GEOSGeometry *env = 0;
    double tolerance = 0.0;
    zend_bool edgeonly = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|dbo",
            &tolerance, &edgeonly, &zobj) == FAILURE) {
        RETURN_NULL();
    }

    if ( zobj ) env = getRelay(zobj, Geometry_ce_ptr);
    ret = GEOSVoronoiDiagram_r(GEOS_G(handle), this, env, tolerance, edgeonly ? 1 : 0);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}
#endif

/**
 * bool GEOSRelateMatch(string matrix, string pattern)
 */
#ifdef HAVE_GEOS_RELATE_PATTERN_MATCH
PHP_FUNCTION(GEOSRelateMatch)
{
    char* mat = NULL;
    char* pat = NULL;
#if PHP_VERSION_ID >= 70000
    size_t matlen;
    size_t patlen;
#else
    int matlen;
    int patlen;
#endif
    int ret;
    zend_bool retBool;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
        &mat, &matlen, &pat, &patlen) == FAILURE)
    {
        RETURN_NULL();
    }

    ret = GEOSRelatePatternMatch_r(GEOS_G(handle), mat, pat);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}
#endif

/* ------ Initialization / Deinitialization / Meta ------------------ */

/* per-module initialization */
PHP_MINIT_FUNCTION(geos)
{
    zend_class_entry ce;

    /* WKTReader */
    INIT_CLASS_ENTRY(ce, "GEOSWKTReader", WKTReader_methods);
    WKTReader_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKTReader_ce_ptr->create_object = WKTReader_create_obj;
    memcpy(&WKTReader_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKTReader_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 70000
    WKTReader_object_handlers.offset = XtOffsetOf(Proxy, std);
    WKTReader_object_handlers.free_obj = WKTReader_dtor;
#endif

    /* WKTWriter */
    INIT_CLASS_ENTRY(ce, "GEOSWKTWriter", WKTWriter_methods);
    WKTWriter_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKTWriter_ce_ptr->create_object = WKTWriter_create_obj;
    memcpy(&WKTWriter_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKTWriter_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 70000
    WKTWriter_object_handlers.offset = XtOffsetOf(Proxy, std);
    WKTWriter_object_handlers.free_obj = WKTWriter_dtor;
#endif

    /* Geometry */
    INIT_CLASS_ENTRY(ce, "GEOSGeometry", Geometry_methods);
    Geometry_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    Geometry_ce_ptr->create_object = Geometry_create_obj;
    memcpy(&Geometry_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    Geometry_object_handlers.clone_obj = NULL;
    /* Geometry serialization */
    Geometry_ce_ptr->serialize = Geometry_serialize;
    Geometry_ce_ptr->unserialize = Geometry_deserialize;
#if PHP_VERSION_ID >= 70000
    Geometry_object_handlers.offset = XtOffsetOf(Proxy, std);
    Geometry_object_handlers.free_obj = Geometry_dtor;
#endif

    /* WKBWriter */
    INIT_CLASS_ENTRY(ce, "GEOSWKBWriter", WKBWriter_methods);
    WKBWriter_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKBWriter_ce_ptr->create_object = WKBWriter_create_obj;
    memcpy(&WKBWriter_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKBWriter_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 70000
    WKBWriter_object_handlers.offset = XtOffsetOf(Proxy, std);
    WKBWriter_object_handlers.free_obj = WKBWriter_dtor;
#endif

    /* WKBReader */
    INIT_CLASS_ENTRY(ce, "GEOSWKBReader", WKBReader_methods);
    WKBReader_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKBReader_ce_ptr->create_object = WKBReader_create_obj;
    memcpy(&WKBReader_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKBReader_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 70000
    WKBReader_object_handlers.offset = XtOffsetOf(Proxy, std);
    WKBReader_object_handlers.free_obj = WKBReader_dtor;
#endif

    /* CoordSeq */
    INIT_CLASS_ENTRY(ce, "GEOSCoordSeq", CoordSeq_methods);
    CoordSeq_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    CoordSeq_ce_ptr->create_object = CoordSeq_create_obj;
    memcpy(&CoordSeq_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    CoordSeq_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 70000
    CoordSeq_object_handlers.offset = XtOffsetOf(Proxy, std);
    CoordSeq_object_handlers.free_obj = CoordSeq_dtor;
#endif


    /* Constants */
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_ROUND",  GEOSBUF_CAP_ROUND,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_FLAT",   GEOSBUF_CAP_FLAT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_SQUARE", GEOSBUF_CAP_SQUARE,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_ROUND", GEOSBUF_JOIN_ROUND,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_MITRE", GEOSBUF_JOIN_MITRE,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_BEVEL", GEOSBUF_JOIN_BEVEL,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOS_POINT", GEOS_POINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_LINESTRING", GEOS_LINESTRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_LINEARRING", GEOS_LINEARRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_POLYGON", GEOS_POLYGON,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTIPOINT", GEOS_MULTIPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTILINESTRING", GEOS_MULTILINESTRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTIPOLYGON", GEOS_MULTIPOLYGON,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_GEOMETRYCOLLECTION", GEOS_GEOMETRYCOLLECTION,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE",
        GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE,
        CONST_CS|CONST_PERSISTENT);

#   ifdef HAVE_GEOS_PREC_NO_TOPO
    REGISTER_LONG_CONSTANT("GEOS_PREC_NO_TOPO", GEOS_PREC_NO_TOPO,
        CONST_CS|CONST_PERSISTENT);
#   endif

#   ifdef HAVE_GEOS_PREC_KEEP_COLLAPSED
    REGISTER_LONG_CONSTANT("GEOS_PREC_KEEP_COLLAPSED", GEOS_PREC_KEEP_COLLAPSED,
        CONST_CS|CONST_PERSISTENT);
#   endif

    /* Item 3: MakeValid methods */
    REGISTER_LONG_CONSTANT("GEOS_MAKE_VALID_LINEWORK", GEOS_MAKE_VALID_LINEWORK,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MAKE_VALID_STRUCTURE", GEOS_MAKE_VALID_STRUCTURE,
        CONST_CS|CONST_PERSISTENT);

    /* Item 4: PolygonHullSimplify modes — names per implementation plan,
     * values from GEOSPolygonHullParameterModes in geos_c.h. */
    REGISTER_LONG_CONSTANT("GEOS_HULL_PARAM_VERTEX_NUM_FRACTION",
        GEOSHULL_PARAM_VERTEX_RATIO, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_HULL_PARAM_AREA_DELTA_RATIO",
        GEOSHULL_PARAM_AREA_RATIO, CONST_CS|CONST_PERSISTENT);

    /* Item 7: WKB flavors */
    REGISTER_LONG_CONSTANT("GEOSWKB_EXTENDED", GEOS_WKB_EXTENDED,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSWKB_ISO", GEOS_WKB_ISO,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MOD2", GEOSRELATE_BNR_MOD2,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_OGC", GEOSRELATE_BNR_OGC,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_ENDPOINT", GEOSRELATE_BNR_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MULTIVALENT_ENDPOINT",
        GEOSRELATE_BNR_MULTIVALENT_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MONOVALENT_ENDPOINT",
        GEOSRELATE_BNR_MONOVALENT_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);

    return SUCCESS;
}

/* per-module shutdown */
PHP_MSHUTDOWN_FUNCTION(geos)
{
    delGeometrySerializer();
    delGeometryDeserializer();
    return SUCCESS;
}

/* per-request initialization */
PHP_RINIT_FUNCTION(geos)
{
    GEOS_G(handle) = initGEOS_r(noticeHandler, errorHandler);
    return SUCCESS;
}

/* pre-request destruction */
PHP_RSHUTDOWN_FUNCTION(geos)
{
    finishGEOS_r(GEOS_G(handle));
    return SUCCESS;
}

/* global initialization */
PHP_GINIT_FUNCTION(geos)
{
    geos_globals->handle = NULL;
}

/* module info */
PHP_MINFO_FUNCTION(geos)
{
    php_info_print_table_start();
    php_info_print_table_row(2,
        "GEOS - Geometry Engine Open Source", "enabled");
    php_info_print_table_row(2,
        "Version", PHP_GEOS_VERSION);
    php_info_print_table_row(2,
        "GEOS Version", GEOSversion());
    php_info_print_table_end();
}
