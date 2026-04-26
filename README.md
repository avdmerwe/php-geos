PHP module for GEOS (extended fork)
===================================

This is the **extended fork** of `php-geos` maintained at
[github.com/avdmerwe/php-geos](https://github.com/avdmerwe/php-geos),
targeting **libgeos 3.14.x** (minimum 3.11.0) and exposing the broader
GEOS C API. It is a drop-in replacement for the upstream `1.0.0` PECL
release: every existing method keeps the same signature and behaviour,
and roughly 80 new methods plus six new classes are added.

The bindings link to the GEOS C API for stability.

Upstream lives at
[git.osgeo.org/gitea/geos/php-geos](https://git.osgeo.org/gitea/geos/php-geos);
this fork is rebased on the post-1.0.0 git HEAD (commit `dfe1ab1`,
2024-01-29).

# Supported GEOS versions

  - **Minimum:** GEOS 3.11.0.
  - **Target:** GEOS 3.14.x (tested on 3.14.1).
  - The build aborts at `./configure` time if the installed
    `geos-config --version` is below 3.11.

# Class / feature surface

| Class                     | Highlights                                                                                                  |
|---------------------------|-------------------------------------------------------------------------------------------------------------|
| `GEOSGeometry`            | Existing 1.0.0 surface plus MakeValid (linework + structure), concave hulls, precision-grid overlays, hull/metric extras (bounding circle, rotated rectangle, inscribed circle, etc.), distance extras (`distanceWithin`, `distanceIndexed`, `nearestPoints`, Frechet/Hausdorff variants), linear-referencing extras (`lineSubstring`, `lineMergeDirected`), construction utilities (`reverse`, `densify`, `removeRepeatedPoints`, `orientPolygons`, `equalsIdentical`, `getExtent`, `buildArea`), curved geometry constructors (CircularString, CompoundCurve, CurvePolygon), coverage processing, clustering, constrained Delaunay, geometry construction factories, coordinate-transform callbacks, Hilbert/orientation helpers. |
| `GEOSCoordSeq`            | Foundation sequence with full Z/M support: `getSize`, `getDimensions`, `setX/Y/Z/M`, `getX/Y/Z/M`, `setXY`, `setXYZ`, `getXY`, `getXYZ`, `copyFromArrays`, `copyToArrays`, `isCCW`. |
| `GEOSWKTReader`/`Writer`  | WKT I/O. Reader gains `setFixStructure(bool)`. Writer behaves per GEOS 3.14 defaults (trim on, accepts XYZM). |
| `GEOSWKBReader`/`Writer`  | WKB I/O. Reader gains `setFixStructure(bool)`. Writer gains `setFlavor(int)` / `getFlavor()` toggling between `GEOSWKB_EXTENDED` (default) and `GEOSWKB_ISO`. |
| `GEOSGeoJSONReader`/`Writer` | RFC-7946 geometry-shaped JSON I/O. Writer supports indent and `setOutputDimension`. |
| `GEOSPreparedGeometry`    | Cached predicate engine: `contains`, `containsProperly`, `containsXY`, `coveredBy`, `covers`, `crosses`, `disjoint`, `intersects`, `intersectsXY`, `overlaps`, `touches`, `within`, plus `distance`, `distanceWithin`, `nearestPoints`. |
| `GEOSSTRtree`             | Spatial R-tree index: `insert`, `remove`, `query`, `iterate`, `nearest`. Supports both collect-mode (returns array) and callback-mode iteration. PHP exceptions thrown from a callback propagate cleanly. |
| `GEOSClusterResult`       | Cluster output of `GEOSGeometry::clusterDBSCAN` / `clusterByDistance` / `clusterByIntersection`: `getNumClusters`, `getClusterSize`, `getClustersForInputs`, `getInputsForCluster`. |

A few new top-level functions are also provided: `GEOSPolygonizeValid`,
`GEOSPolygonizeCutEdges`, `GEOSBuildArea`, `GEOSDisjointSubsetUnion`,
`GEOSOrientationIndex`, `GEOSSegmentIntersection`.

The full per-feature breakdown lives in `docs/04-implementation-plan.md`
of the parent repository.

# Building

## Requirements

You need:

  - PHP development files
    (often found in packages named something like `php8.1-dev`).
  - GEOS development files >= 3.11.0
    (`libgeos-dev`; the `geos-config` binary must be on `$PATH` or
    pointed at via `--with-geos-config`).

## Procedure

    git clone https://github.com/avdmerwe/php-geos.git
    cd php-geos
    ./autogen.sh
    phpize
    ./configure --enable-geos
    make            # generates modules/geos.so

# Testing

    make test

The test suite is `.phpt`-based and runs through PHP's `run-tests.php`.
There are 25 test files exercising every new entry point plus the
existing baseline. All 25 pass against libgeos 3.14.1.

# Installing

As root (or owner with write access to the directory returned by
`php-config --extension-dir`), run:

    make install

Then enable the extension by adding `extension=geos.so` to your
`php.ini`. On Debian/Ubuntu, the `.deb` produced by
`dpkg-buildpackage` (see `debian/`) handles this for you and drops a
`/etc/php/<ver>/mods-available/geos.ini` snippet.

# Acknowledgements

  - Original `php-geos` codebase by J Smith, Sandro Santilli and
    contributors at the GEOS project
    ([git.osgeo.org/gitea/geos/php-geos](https://git.osgeo.org/gitea/geos/php-geos)).
  - Debian/Ubuntu packaging by Bas Couwenberg, Athos Ribeiro and
    Renan Rodrigo.
  - Extended-fork bindings (1.1.0) by Abraham van der Merwe
    `<abz@frogfoot.com>`.

# License

Mixed (LGPL-2 / MIT) — see `COPYING`, `LGPL-2`, and `MIT-LICENSE`.

# Contributing

Patches and pull requests are welcome at
[github.com/avdmerwe/php-geos](https://github.com/avdmerwe/php-geos).
Please ensure `make test` is green and that any new method ships with
a `.phpt` covering at least one happy path and one edge case.
