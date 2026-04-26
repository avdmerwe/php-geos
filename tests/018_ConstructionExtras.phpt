--TEST--
Construction / utility extras + standalone polygonize/buildArea/disjointSubsetUnion
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class ConstructionExtrasTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }
    private function writer() {
        $w = new GEOSWKTWriter();
        if (method_exists($w, 'setTrim')) $w->setTrim(true);
        return $w;
    }

    public function testReverseLineString()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 1 1, 2 2)');
        $r = $g->reverse();
        $this->assertNotNull($r);
        /* First point of reversed line equals last point of original. */
        $this->assertEquals(2.0, $r->getCoordSeq()->getX(0));
        $this->assertEquals(2.0, $r->getCoordSeq()->getY(0));
    }

    public function testDensifySpacing()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $d = $g->densify(2.0);
        /* Original had 2 vertices; densified at 2.0 over length 10 has at
         * least 6 vertices. */
        $this->assertNotNull($d);
        $this->assertTrue($d->numCoordinates() >= 6);
    }

    public function testDensifyRequiresArg()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 10 0)');
        try {
            $g->densify();
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }

    public function testRemoveRepeatedPoints()
    {
        $g = $this->reader()->read(
            'LINESTRING(0 0, 0 0, 1 1, 1 1, 2 2)');
        $r = $g->removeRepeatedPoints();
        $this->assertEquals(3, $r->numCoordinates());
    }

    public function testRemoveRepeatedPointsWithTolerance()
    {
        $g = $this->reader()->read(
            'LINESTRING(0 0, 0.001 0.001, 1 1, 1.0001 1.0001, 2 2)');
        $r = $g->removeRepeatedPoints(0.01);
        $this->assertTrue($r->numCoordinates() < 5);
    }

    public function testOrientPolygonsCCW()
    {
        /* Default exterior_cw=false => exterior CCW. */
        $g = $this->reader()->read(
            'POLYGON((0 0, 0 10, 10 10, 10 0, 0 0))'); // CW
        $r = $g->orientPolygons();
        $this->assertNotNull($r);
        /* Original unchanged */
        $cs = $g->exteriorRing()->getCoordSeq();
        $this->assertFalse($cs->isCCW());
    }

    public function testOrientPolygonsCW()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))'); // CCW
        $r = $g->orientPolygons(true);
        $cs = $r->exteriorRing()->getCoordSeq();
        $this->assertFalse($cs->isCCW());
    }

    public function testEqualsIdenticalPositive()
    {
        $a = $this->reader()->read('POINT(1 2)');
        $b = $this->reader()->read('POINT(1 2)');
        $this->assertTrue($a->equalsIdentical($b));
    }

    public function testEqualsIdenticalNegative()
    {
        $a = $this->reader()->read('POINT(1 2)');
        $b = $this->reader()->read('POINT(1 3)');
        $this->assertFalse($a->equalsIdentical($b));
    }

    public function testGetXMinMaxYMinMax()
    {
        $g = $this->reader()->read(
            'POLYGON((1 2, 5 2, 5 8, 1 8, 1 2))');
        $this->assertEquals(1.0, $g->getXMin());
        $this->assertEquals(5.0, $g->getXMax());
        $this->assertEquals(2.0, $g->getYMin());
        $this->assertEquals(8.0, $g->getYMax());
    }

    public function testGetExtent()
    {
        $g = $this->reader()->read(
            'POLYGON((1 2, 5 2, 5 8, 1 8, 1 2))');
        $e = $g->getExtent();
        $this->assertTrue(is_array($e));
        $this->assertEquals(1.0, $e['xmin']);
        $this->assertEquals(2.0, $e['ymin']);
        $this->assertEquals(5.0, $e['xmax']);
        $this->assertEquals(8.0, $e['ymax']);
    }

    public function testBuildAreaMethod()
    {
        $g = $this->reader()->read(
            'GEOMETRYCOLLECTION('.
            'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0))');
        $a = $g->buildArea();
        $this->assertNotNull($a);
        $this->assertTrue($a->area() > 0);
    }

    public function testStandaloneBuildArea()
    {
        $g = $this->reader()->read(
            'GEOMETRYCOLLECTION('.
            'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0))');
        $a = GEOSBuildArea($g);
        $this->assertNotNull($a);
        $this->assertTrue($a->area() > 0);
    }

    public function testStandalonePolygonizeValid()
    {
        $g = $this->reader()->read(
            'MULTILINESTRING((0 0, 1 0, 1 1, 0 1, 0 0))');
        $arr = GEOSPolygonizeValid($g);
        $this->assertTrue(is_array($arr));
    }

    public function testStandalonePolygonizeCutEdges()
    {
        $g = $this->reader()->read(
            'MULTILINESTRING((0 0, 1 0, 1 1, 0 1, 0 0),'.
            '(2 2, 3 3))');
        $arr = GEOSPolygonizeCutEdges($g);
        $this->assertTrue(is_array($arr));
    }

    public function testStandaloneDisjointSubsetUnion()
    {
        $g = $this->reader()->read(
            'MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),'.
            '((10 10, 11 10, 11 11, 10 11, 10 10)))');
        $u = GEOSDisjointSubsetUnion($g);
        $this->assertNotNull($u);
    }
}

ConstructionExtrasTest::run();

?>
--EXPECT--
ConstructionExtrasTest->testReverseLineString	OK
ConstructionExtrasTest->testDensifySpacing	OK
ConstructionExtrasTest->testDensifyRequiresArg	OK
ConstructionExtrasTest->testRemoveRepeatedPoints	OK
ConstructionExtrasTest->testRemoveRepeatedPointsWithTolerance	OK
ConstructionExtrasTest->testOrientPolygonsCCW	OK
ConstructionExtrasTest->testOrientPolygonsCW	OK
ConstructionExtrasTest->testEqualsIdenticalPositive	OK
ConstructionExtrasTest->testEqualsIdenticalNegative	OK
ConstructionExtrasTest->testGetXMinMaxYMinMax	OK
ConstructionExtrasTest->testGetExtent	OK
ConstructionExtrasTest->testBuildAreaMethod	OK
ConstructionExtrasTest->testStandaloneBuildArea	OK
ConstructionExtrasTest->testStandalonePolygonizeValid	OK
ConstructionExtrasTest->testStandalonePolygonizeCutEdges	OK
ConstructionExtrasTest->testStandaloneDisjointSubsetUnion	OK
