--TEST--
GEOSGeometry concave hull family
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class ConcaveHullTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    public function testConstantsRegistered()
    {
        $this->assertTrue(defined('GEOS_HULL_PARAM_VERTEX_NUM_FRACTION'));
        $this->assertTrue(defined('GEOS_HULL_PARAM_AREA_DELTA_RATIO'));
    }

    public function testConcaveHullMultipoint()
    {
        $g = $this->reader()->read(
            'MULTIPOINT(0 0, 1 0, 2 0, 0 1, 1 1, 2 1, 0 2, 1 2, 2 2)');
        $h = $g->concaveHull(0.5);
        $this->assertNotNull($h);
        $this->assertTrue($h->area() > 0);
    }

    public function testConcaveHullAllowHoles()
    {
        $g = $this->reader()->read(
            'MULTIPOINT(0 0, 1 0, 2 0, 0 1, 1 1, 2 1, 0 2, 1 2, 2 2)');
        $h = $g->concaveHull(0.5, true);
        $this->assertNotNull($h);
    }

    public function testConcaveHullByLength()
    {
        $g = $this->reader()->read('MULTIPOINT(0 0, 10 0, 5 8, 5 4)');
        $h = $g->concaveHullByLength(20.0);
        $this->assertNotNull($h);
    }

    public function testConcaveHullByLengthAllowHoles()
    {
        $g = $this->reader()->read('MULTIPOINT(0 0, 10 0, 5 8, 5 4)');
        $h = $g->concaveHullByLength(20.0, true);
        $this->assertNotNull($h);
    }

    public function testConcaveHullOfPolygons()
    {
        /* Two adjacent unit squares — concave hull of polygons should
         * yield a sensible polygon. */
        $g = $this->reader()->read(
            'MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),'.
            '((2 0, 3 0, 3 1, 2 1, 2 0)))');
        $h = $g->concaveHullOfPolygons(0.7);
        $this->assertNotNull($h);
    }

    public function testConcaveHullOfPolygonsLoose()
    {
        $g = $this->reader()->read(
            'MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),'.
            '((2 0, 3 0, 3 1, 2 1, 2 0)))');
        $h = $g->concaveHullOfPolygons(0.7, false, true);
        $this->assertNotNull($h);
    }

    public function testPolygonHullSimplifyDefaultMode()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 4 0, 4.1 1, 4 2, 4 4, 0 4, 0 0))');
        $h = $g->polygonHullSimplify(true, 0.5);
        $this->assertNotNull($h);
    }

    public function testPolygonHullSimplifyVertexFraction()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 4 0, 4.1 1, 4 2, 4 4, 0 4, 0 0))');
        $h = $g->polygonHullSimplify(true, 0.5,
            GEOS_HULL_PARAM_VERTEX_NUM_FRACTION);
        $this->assertNotNull($h);
    }

    public function testPolygonHullSimplifyAreaDelta()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 4 0, 4.1 1, 4 2, 4 4, 0 4, 0 0))');
        $h = $g->polygonHullSimplify(true, 0.1,
            GEOS_HULL_PARAM_AREA_DELTA_RATIO);
        $this->assertNotNull($h);
    }

    public function testPolygonHullSimplifyInner()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 4 0, 4.1 1, 4 2, 4 4, 0 4, 0 0))');
        $h = $g->polygonHullSimplify(false, 0.5);
        $this->assertNotNull($h);
    }
}

ConcaveHullTest::run();

?>
--EXPECT--
ConcaveHullTest->testConstantsRegistered	OK
ConcaveHullTest->testConcaveHullMultipoint	OK
ConcaveHullTest->testConcaveHullAllowHoles	OK
ConcaveHullTest->testConcaveHullByLength	OK
ConcaveHullTest->testConcaveHullByLengthAllowHoles	OK
ConcaveHullTest->testConcaveHullOfPolygons	OK
ConcaveHullTest->testConcaveHullOfPolygonsLoose	OK
ConcaveHullTest->testPolygonHullSimplifyDefaultMode	OK
ConcaveHullTest->testPolygonHullSimplifyVertexFraction	OK
ConcaveHullTest->testPolygonHullSimplifyAreaDelta	OK
ConcaveHullTest->testPolygonHullSimplifyInner	OK
