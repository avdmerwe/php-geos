/* commonly used ones */

ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geom, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_num, 0, 0, 1)
    ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

/* function arginfo */

#define arginfo_GEOSVersion              arginfo_void

#define arginfo_GEOSPolygonize           arginfo_geom
#define arginfo_GEOSLineMerge            arginfo_geom

#ifdef HAVE_GEOS_SHARED_PATHS
ZEND_BEGIN_ARG_INFO_EX(arginfo_GEOSSharedPaths, 0, 0, 2)
    ZEND_ARG_INFO(0, geom1)
    ZEND_ARG_INFO(0, geom2)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GEOS_RELATE_PATTERN_MATCH
ZEND_BEGIN_ARG_INFO_EX(arginfo_GEOSRelateMatch, 0, 0, 2)
    ZEND_ARG_INFO(0, maxtric)
    ZEND_ARG_INFO(0, pattern)
ZEND_END_ARG_INFO()
#endif

/* GEOSGeometry methods arginfo */

#define arginfo_Geometry_construct       arginfo_void

#if PHP_VERSION_ID >= 80200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Geometry_toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO();
#else
#define arginfo_Geometry_toString        arginfo_void
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_project, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, normalized)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_interpolate, 0, 0, 1)
    ZEND_ARG_INFO(0, dist)
    ZEND_ARG_INFO(0, normalized)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_buffer, 0, 0, 1)
    ZEND_ARG_INFO(0, dist)
    ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

#ifdef HAVE_GEOS_OFFSET_CURVE
#define arginfo_Geometry_offsetCurve     arginfo_Geometry_buffer
#endif

#define arginfo_Geometry_envelope        arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_intersection, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, gridSize)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_convexHull      arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_difference, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, gridSize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_symDifference, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, gridSize)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_boundary        arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_union, 0, 0, 0)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, gridSize)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_pointOnSurface  arginfo_void

#define arginfo_Geometry_centroid        arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_relate, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, pattern)
ZEND_END_ARG_INFO()

#ifdef HAVE_GEOS_RELATE_BOUNDARY_NODE_RULE
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_relateBoundaryNodeRule, 0, 0, 2)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, rule)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_simplify, 0, 0, 1)
    ZEND_ARG_INFO(0, tolerance)
    ZEND_ARG_INFO(0, preserveTopology)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_normalize       arginfo_void

#ifdef HAVE_GEOS_GEOM_SET_PRECISION
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_setPrecision, 0, 0, 1)
    ZEND_ARG_INFO(0, gridsize)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GEOS_GEOM_GET_PRECISION
#define arginfo_Geometry_getPrecision    arginfo_void
#endif

#ifdef HAVE_GEOS_GEOM_EXTRACT_UNIQUE_POINTS
#define arginfo_Geometry_extractUniquePoints arginfo_void
#endif

#define arginfo_Geometry_disjoint        arginfo_geom

#define arginfo_Geometry_touches         arginfo_geom

#define arginfo_Geometry_intersects      arginfo_geom

#define arginfo_Geometry_crosses         arginfo_geom

#define arginfo_Geometry_within          arginfo_geom

#define arginfo_Geometry_contains        arginfo_geom

#define arginfo_Geometry_overlaps        arginfo_geom

#ifdef HAVE_GEOS_COVERS
#define arginfo_Geometry_covers          arginfo_geom
#endif

#ifdef HAVE_GEOS_COVERED_BY
#define arginfo_Geometry_coveredBy       arginfo_geom
#endif

#define arginfo_Geometry_equals          arginfo_geom

#define arginfo_Geometry_equalsExact     arginfo_geom

#define arginfo_Geometry_isEmpty         arginfo_void

#ifdef HAVE_GEOS_IS_VALID_DETAIL
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_checkValidity, 0, 0, 0)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
#endif

#define arginfo_Geometry_isSimple        arginfo_void

#define arginfo_Geometry_isRing          arginfo_void

#define arginfo_Geometry_hasZ            arginfo_void

#define arginfo_Geometry_hasM            arginfo_void

#define arginfo_Geometry_getZ            arginfo_void

#define arginfo_Geometry_getM            arginfo_void

#define arginfo_Geometry_getCoordSeq     arginfo_void


#ifdef HAVE_GEOS_IS_CLOSED
#define arginfo_Geometry_isClosed        arginfo_void
#endif

#define arginfo_Geometry_typeName        arginfo_void

#define arginfo_Geometry_typeId          arginfo_void

#define arginfo_Geometry_getSRID         arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_setSRID, 0, 0, 0)
    ZEND_ARG_INFO(0, srid)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_numGeometries   arginfo_void

#define arginfo_Geometry_geometryN       arginfo_num

#define arginfo_Geometry_numInteriorRings arginfo_void

#ifdef HAVE_GEOS_GEOM_GET_NUM_POINTS
#define arginfo_Geometry_numPoints       arginfo_void
#endif

#ifdef HAVE_GEOS_GEOM_GET_X
#define arginfo_Geometry_getX            arginfo_void
#endif

#ifdef HAVE_GEOS_GEOM_GET_Y
#define arginfo_Geometry_getY            arginfo_void
#endif

#define arginfo_Geometry_interiorRingN   arginfo_num

#define arginfo_Geometry_exteriorRing    arginfo_void

#define arginfo_Geometry_numCoordinates  arginfo_void

#define arginfo_Geometry_dimension       arginfo_void

#ifdef HAVE_GEOS_GEOM_GET_COORDINATE_DIMENSION
#define arginfo_Geometry_coordinateDimension arginfo_void
#endif

#ifdef HAVE_GEOS_GEOM_GET_POINT_N
#define arginfo_Geometry_pointN          arginfo_num
#endif

#ifdef HAVE_GEOS_GEOM_GET_START_POINT
#define arginfo_Geometry_startPoint      arginfo_void
#endif

#ifdef HAVE_GEOS_GEOM_GET_END_POINT
#define arginfo_Geometry_endPoint        arginfo_void
#endif

#define arginfo_Geometry_area            arginfo_void

#define arginfo_Geometry_length          arginfo_void

#define arginfo_Geometry_distance        arginfo_geom

#define arginfo_Geometry_hausdorffDistance arginfo_geom

#ifdef HAVE_GEOS_SNAP
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_snapTo, 0, 0, 2)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, tolerance)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GEOS_NODE
#define arginfo_Geometry_node            arginfo_void
#endif

#ifdef HAVE_GEOS_DELAUNAY_TRIANGULATION
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_delaunayTriangulation, 0, 0, 0)
    ZEND_ARG_INFO(0, tolerance)
    ZEND_ARG_INFO(0, edgeonly)
    ZEND_ARG_INFO(0, constrained)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GEOS_VORONOI_DIAGRAM
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_voronoiDiagram, 0, 0, 0)
    ZEND_ARG_INFO(0, tolerance)
    ZEND_ARG_INFO(0, edgeonly)
    ZEND_ARG_INFO(0, extent)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GEOS_CLIP_BY_RECT
ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_clipByRect, 0, 0, 4)
    ZEND_ARG_INFO(0, xmin)
    ZEND_ARG_INFO(0, ymin)
    ZEND_ARG_INFO(0, xmax)
    ZEND_ARG_INFO(0, ymax)
ZEND_END_ARG_INFO()
#endif

/* GEOSWKBReader methods arginfo */

#define arginfo_WKBReader_construct      arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_WKBReader_read, 0, 0, 1)
    ZEND_ARG_INFO(0, wkb)
ZEND_END_ARG_INFO()

#define arginfo_WKBReader_readHEX        arginfo_WKBReader_read

/* GEOSWKTReader methods arginfo */

#define arginfo_WKTReader_construct      arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_WKTReader_read, 0, 0, 1)
    ZEND_ARG_INFO(0, wkt)
ZEND_END_ARG_INFO()

/* GEOSWKTWriter methods arginfo */

#define arginfo_WKTWriter_construct      arginfo_void

#define arginfo_WKTWriter_write          arginfo_geom

#ifdef HAVE_GEOS_WKT_WRITER_SET_TRIM
#define arginfo_WKTWriter_setTrim        arginfo_num
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_ROUNDING_PRECISION
#define arginfo_WKTWriter_setRoundingPrecision arginfo_num
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_OUTPUT_DIMENSION
#define arginfo_WKTWriter_setOutputDimension   arginfo_num
#endif

#ifdef HAVE_GEOS_WKT_WRITER_GET_OUTPUT_DIMENSION
#define arginfo_WKTWriter_getOutputDimension arginfo_void
#endif

#ifdef HAVE_GEOS_WKT_WRITER_SET_OLD_3D
ZEND_BEGIN_ARG_INFO_EX(arginfo_WKTWriter_setOld3D, 0, 0, 1)
    ZEND_ARG_INFO(0, bval)
ZEND_END_ARG_INFO()
#endif

/* GEOSWKTWriter methods arginfo */

#define arginfo_WKBWriter_construct          arginfo_void

#define arginfo_WKBWriter_getOutputDimension arginfo_void

#define arginfo_WKBWriter_setOutputDimension arginfo_num

#define arginfo_WKBWriter_getByteOrder       arginfo_void

#define arginfo_WKBWriter_setByteOrder       arginfo_num

#define arginfo_WKBWriter_setIncludeSRID     arginfo_num

#define arginfo_WKBWriter_getIncludeSRID     arginfo_void

#define arginfo_WKBWriter_write              arginfo_geom

#define arginfo_WKBWriter_writeHEX           arginfo_geom

/* GEOSCoordSeq methods arginfo */

ZEND_BEGIN_ARG_INFO_EX(arginfo_CoordSeq_construct, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
    ZEND_ARG_INFO(0, dim)
    ZEND_ARG_INFO(0, hasM)
ZEND_END_ARG_INFO()

#define arginfo_CoordSeq_getSize         arginfo_void

#define arginfo_CoordSeq_getDimensions   arginfo_void

#define arginfo_CoordSeq_hasZ            arginfo_void

#define arginfo_CoordSeq_hasM            arginfo_void

#define arginfo_CoordSeq_isCCW           arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_CoordSeq_setOrdinate, 0, 0, 2)
    ZEND_ARG_INFO(0, idx)
    ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

#define arginfo_CoordSeq_setX            arginfo_CoordSeq_setOrdinate
#define arginfo_CoordSeq_setY            arginfo_CoordSeq_setOrdinate
#define arginfo_CoordSeq_setZ            arginfo_CoordSeq_setOrdinate
#define arginfo_CoordSeq_setM            arginfo_CoordSeq_setOrdinate

#define arginfo_CoordSeq_getX            arginfo_num
#define arginfo_CoordSeq_getY            arginfo_num
#define arginfo_CoordSeq_getZ            arginfo_num
#define arginfo_CoordSeq_getM            arginfo_num

ZEND_BEGIN_ARG_INFO_EX(arginfo_CoordSeq_setXY, 0, 0, 3)
    ZEND_ARG_INFO(0, idx)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_CoordSeq_setXYZ, 0, 0, 4)
    ZEND_ARG_INFO(0, idx)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
    ZEND_ARG_INFO(0, z)
ZEND_END_ARG_INFO()

#define arginfo_CoordSeq_getXY           arginfo_num
#define arginfo_CoordSeq_getXYZ          arginfo_num

ZEND_BEGIN_ARG_INFO_EX(arginfo_CoordSeq_copyFromArrays, 0, 0, 2)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
    ZEND_ARG_INFO(0, z)
    ZEND_ARG_INFO(0, m)
ZEND_END_ARG_INFO()

#define arginfo_CoordSeq_copyToArrays    arginfo_void


/* -- Item 3: MakeValid family -------------------- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_makeValid, 0, 0, 0)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()


/* -- Item 4: Concave hull family ----------------- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_concaveHull, 0, 0, 1)
    ZEND_ARG_INFO(0, ratio)
    ZEND_ARG_INFO(0, allowHoles)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_concaveHullByLength, 0, 0, 1)
    ZEND_ARG_INFO(0, maxLength)
    ZEND_ARG_INFO(0, allowHoles)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_concaveHullOfPolygons, 0, 0, 1)
    ZEND_ARG_INFO(0, ratio)
    ZEND_ARG_INFO(0, isTight)
    ZEND_ARG_INFO(0, allowHoles)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_polygonHullSimplify, 0, 0, 2)
    ZEND_ARG_INFO(0, isOuter)
    ZEND_ARG_INFO(0, param)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()


/* -- Item 7: WKB flavor + reader fix-structure --- */

#define arginfo_WKBWriter_setFlavor          arginfo_num
#define arginfo_WKBWriter_getFlavor          arginfo_void
#define arginfo_WKTReader_setFixStructure    arginfo_num
#define arginfo_WKBReader_setFixStructure    arginfo_num


/* -- Item 8: Hull / metric extras ---------------- */

#define arginfo_Geometry_minimumBoundingCircle    arginfo_void
#define arginfo_Geometry_minimumRotatedRectangle  arginfo_void
#define arginfo_Geometry_minimumWidth             arginfo_void
#define arginfo_Geometry_minimumClearance         arginfo_void
#define arginfo_Geometry_minimumClearanceLine     arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_maximumInscribedCircle, 0, 0, 1)
    ZEND_ARG_INFO(0, tol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_largestEmptyCircle, 0, 0, 1)
    ZEND_ARG_INFO(0, tol)
    ZEND_ARG_INFO(0, boundary)
ZEND_END_ARG_INFO()


/* -- Item 9: Distance / metric extras ------------ */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_distanceWithin, 0, 0, 2)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, maxDist)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_distanceIndexed     arginfo_geom
#define arginfo_Geometry_nearestPoints       arginfo_geom
#define arginfo_Geometry_frechetDistance     arginfo_geom

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_frechetDistanceDensify, 0, 0, 2)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, frac)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_hausdorffDistanceDensify, 0, 0, 2)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, frac)
ZEND_END_ARG_INFO()


/* -- Item 10: Linear referencing extras ---------- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_lineSubstring, 0, 0, 2)
    ZEND_ARG_INFO(0, startFrac)
    ZEND_ARG_INFO(0, endFrac)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_lineMergeDirected   arginfo_void


/* -- Item 11: Construction / utility extras ------ */

#define arginfo_Geometry_reverse             arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_densify, 0, 0, 1)
    ZEND_ARG_INFO(0, dist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_removeRepeatedPoints, 0, 0, 0)
    ZEND_ARG_INFO(0, tol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_orientPolygons, 0, 0, 0)
    ZEND_ARG_INFO(0, exterior_cw)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_equalsIdentical     arginfo_geom
#define arginfo_Geometry_getXMin             arginfo_void
#define arginfo_Geometry_getXMax             arginfo_void
#define arginfo_Geometry_getYMin             arginfo_void
#define arginfo_Geometry_getYMax             arginfo_void
#define arginfo_Geometry_getExtent           arginfo_void
#define arginfo_Geometry_buildArea           arginfo_void

#define arginfo_GEOSPolygonizeValid          arginfo_geom
#define arginfo_GEOSPolygonizeCutEdges       arginfo_geom
#define arginfo_GEOSBuildArea                arginfo_geom
#define arginfo_GEOSDisjointSubsetUnion      arginfo_geom


/* -- Item 6: GeoJSON I/O ------------------------- */

#define arginfo_GeoJSONReader_construct      arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_GeoJSONReader_read, 0, 0, 1)
    ZEND_ARG_INFO(0, json)
ZEND_END_ARG_INFO()

#define arginfo_GeoJSONWriter_construct      arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_GeoJSONWriter_write, 0, 0, 1)
    ZEND_ARG_INFO(0, geom)
    ZEND_ARG_INFO(0, indent)
ZEND_END_ARG_INFO()

#define arginfo_GeoJSONWriter_setOutputDimension   arginfo_num
#define arginfo_GeoJSONWriter_getOutputDimension   arginfo_void


/* -- Item 12: Curved geometry factory statics ---- */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createCircularString, 0, 0, 1)
    ZEND_ARG_INFO(0, coordseq)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_createEmptyCircularString  arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createCompoundCurve, 0, 0, 1)
    ZEND_ARG_INFO(0, components)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_createEmptyCompoundCurve   arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createCurvePolygon, 0, 0, 1)
    ZEND_ARG_INFO(0, shell)
    ZEND_ARG_INFO(0, holes)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_createEmptyCurvePolygon    arginfo_void


/* -- Item 16: Geometry construction factories ---- */

#define arginfo_Geometry_createPoint         arginfo_Geometry_createCircularString

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createPointFromXY, 0, 0, 2)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_createEmptyPoint    arginfo_void
#define arginfo_Geometry_createLineString    arginfo_Geometry_createCircularString
#define arginfo_Geometry_createEmptyLineString arginfo_void
#define arginfo_Geometry_createLinearRing    arginfo_Geometry_createCircularString

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createPolygon, 0, 0, 1)
    ZEND_ARG_INFO(0, shell)
    ZEND_ARG_INFO(0, holes)
ZEND_END_ARG_INFO()

#define arginfo_Geometry_createEmptyPolygon  arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createCollection, 0, 0, 2)
    ZEND_ARG_INFO(0, type)
    ZEND_ARG_INFO(0, geoms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createEmptyCollection, 0, 0, 1)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_createRectangle, 0, 0, 4)
    ZEND_ARG_INFO(0, xmin)
    ZEND_ARG_INFO(0, ymin)
    ZEND_ARG_INFO(0, xmax)
    ZEND_ARG_INFO(0, ymax)
ZEND_END_ARG_INFO()


/* -- Item 13: Coverage processing ---------------- */

#define arginfo_Geometry_coverageUnion       arginfo_void

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_coverageIsValid, 0, 0, 0)
    ZEND_ARG_INFO(0, gapWidth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_coverageSimplifyVW, 0, 0, 1)
    ZEND_ARG_INFO(0, tolerance)
    ZEND_ARG_INFO(0, simplifyBoundary)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_coverageClean, 0, 0, 0)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()


/* -- Item 14: Clustering family ------------------ */

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_clusterDBSCAN, 0, 0, 2)
    ZEND_ARG_INFO(0, eps)
    ZEND_ARG_INFO(0, minPoints)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_clusterByDistance, 0, 0, 1)
    ZEND_ARG_INFO(0, distance)
    ZEND_ARG_INFO(0, useEnvelope)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Geometry_clusterByIntersection, 0, 0, 0)
    ZEND_ARG_INFO(0, useEnvelope)
ZEND_END_ARG_INFO()

/* GEOSClusterResult methods arginfo */

#define arginfo_ClusterResult_construct             arginfo_void
#define arginfo_ClusterResult_getNumClusters        arginfo_void
#define arginfo_ClusterResult_getClusterSize        arginfo_num
#define arginfo_ClusterResult_getClustersForInputs  arginfo_void
#define arginfo_ClusterResult_getInputsForCluster   arginfo_num

