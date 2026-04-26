--TEST--
Geometry construction factories — createPoint / LineString / Polygon / Collection / Rectangle
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class FactoriesTest extends GEOSTest
{
    public function testCreatePointFromXY()
    {
        $p = GEOSGeometry::createPointFromXY(1.5, 2.5);
        $this->assertEquals('Point', $p->typeName());
        $writer = new GEOSWKTWriter();
        $writer->setTrim(true);
        $this->assertEquals('POINT (1.5 2.5)', $writer->write($p));
    }

    public function testCreateEmptyPoint()
    {
        $p = GEOSGeometry::createEmptyPoint();
        $this->assertEquals('Point', $p->typeName());
        $this->assertTrue($p->isEmpty());
    }

    public function testCreatePoint()
    {
        $cs = new GEOSCoordSeq(1);
        $cs->setXY(0, 4.0, 5.0);
        $p = GEOSGeometry::createPoint($cs);
        $this->assertEquals('Point', $p->typeName());

        /* Ownership transferred — reusing the consumed CoordSeq must
         * raise the "already consumed" exception. */
        try {
            GEOSGeometry::createPoint($cs);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('already consumed', $e->getMessage());
        }
    }

    public function testCreateLineString()
    {
        $cs = new GEOSCoordSeq(4);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 1);
        $cs->setXY(2, 2, 2);
        $cs->setXY(3, 3, 3);
        $g = GEOSGeometry::createLineString($cs);
        $this->assertEquals('LineString', $g->typeName());
        $this->assertEquals(4, $g->numCoordinates());
    }

    public function testCreateEmptyLineString()
    {
        $g = GEOSGeometry::createEmptyLineString();
        $this->assertEquals('LineString', $g->typeName());
        $this->assertTrue($g->isEmpty());
    }

    public function testCreateLinearRing()
    {
        /* A closed ring: start == end, at least 4 points. */
        $cs = new GEOSCoordSeq(4);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 0);
        $cs->setXY(2, 1, 1);
        $cs->setXY(3, 0, 0);
        $g = GEOSGeometry::createLinearRing($cs);
        $this->assertEquals('LinearRing', $g->typeName());
    }

    public function testCreatePolygonWithHole()
    {
        /* Outer 10x10 square. */
        $shellCs = new GEOSCoordSeq(5);
        $shellCs->setXY(0, 0,  0);
        $shellCs->setXY(1, 10, 0);
        $shellCs->setXY(2, 10, 10);
        $shellCs->setXY(3, 0,  10);
        $shellCs->setXY(4, 0,  0);
        $shell = GEOSGeometry::createLinearRing($shellCs);

        /* Inner hole. */
        $holeCs = new GEOSCoordSeq(5);
        $holeCs->setXY(0, 2, 2);
        $holeCs->setXY(1, 4, 2);
        $holeCs->setXY(2, 4, 4);
        $holeCs->setXY(3, 2, 4);
        $holeCs->setXY(4, 2, 2);
        $hole = GEOSGeometry::createLinearRing($holeCs);

        $poly = GEOSGeometry::createPolygon($shell, array($hole));
        $this->assertEquals('Polygon', $poly->typeName());
        $this->assertEquals(1, $poly->numInteriorRings());
    }

    public function testCreatePolygonNoHoles()
    {
        $cs = new GEOSCoordSeq(5);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 0);
        $cs->setXY(2, 1, 1);
        $cs->setXY(3, 0, 1);
        $cs->setXY(4, 0, 0);
        $shell = GEOSGeometry::createLinearRing($cs);

        $poly = GEOSGeometry::createPolygon($shell);
        $this->assertEquals('Polygon', $poly->typeName());
        $this->assertEquals(0, $poly->numInteriorRings());
    }

    public function testCreateEmptyPolygon()
    {
        $g = GEOSGeometry::createEmptyPolygon();
        $this->assertEquals('Polygon', $g->typeName());
        $this->assertTrue($g->isEmpty());
    }

    public function testCreateCollectionMultiPoint()
    {
        $p1 = GEOSGeometry::createPointFromXY(1, 1);
        $p2 = GEOSGeometry::createPointFromXY(2, 2);
        $mp = GEOSGeometry::createCollection(GEOS_MULTIPOINT, array($p1, $p2));
        $this->assertEquals('MultiPoint', $mp->typeName());
        $this->assertEquals(2, $mp->numGeometries());
    }

    public function testCreateCollectionMultiLineString()
    {
        $cs1 = new GEOSCoordSeq(2);
        $cs1->setXY(0, 0, 0); $cs1->setXY(1, 1, 1);
        $l1 = GEOSGeometry::createLineString($cs1);
        $cs2 = new GEOSCoordSeq(2);
        $cs2->setXY(0, 2, 2); $cs2->setXY(1, 3, 3);
        $l2 = GEOSGeometry::createLineString($cs2);

        $ml = GEOSGeometry::createCollection(
            GEOS_MULTILINESTRING, array($l1, $l2)
        );
        $this->assertEquals('MultiLineString', $ml->typeName());
        $this->assertEquals(2, $ml->numGeometries());
    }

    public function testCreateGeometryCollection()
    {
        $p = GEOSGeometry::createPointFromXY(1, 1);
        $cs = new GEOSCoordSeq(2);
        $cs->setXY(0, 0, 0); $cs->setXY(1, 5, 5);
        $l = GEOSGeometry::createLineString($cs);
        $gc = GEOSGeometry::createCollection(
            GEOS_GEOMETRYCOLLECTION, array($p, $l)
        );
        $this->assertEquals('GeometryCollection', $gc->typeName());
        $this->assertEquals(2, $gc->numGeometries());
    }

    public function testCreateCollectionInvalidType()
    {
        $p = GEOSGeometry::createPointFromXY(1, 1);
        try {
            /* GEOS_POINT (==0) is not a valid collection type. */
            GEOSGeometry::createCollection(GEOS_POINT, array($p));
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('invalid type', $e->getMessage());
        }
    }

    public function testCreateEmptyCollection()
    {
        foreach (array(
            GEOS_MULTIPOINT      => 'MultiPoint',
            GEOS_MULTILINESTRING => 'MultiLineString',
            GEOS_MULTIPOLYGON    => 'MultiPolygon',
            GEOS_GEOMETRYCOLLECTION => 'GeometryCollection',
        ) as $tid => $name) {
            $g = GEOSGeometry::createEmptyCollection($tid);
            $this->assertEquals($name, $g->typeName());
            $this->assertTrue($g->isEmpty());
        }
    }

    public function testCreateRectangle()
    {
        $r = GEOSGeometry::createRectangle(0, 0, 10, 10);
        $this->assertEquals('Polygon', $r->typeName());
        $this->assertEquals(0, $r->numInteriorRings());
        $this->assertEquals(100.0, $r->area());
    }

    /* Verifies that after createCollection consumes its input geometries,
     * passing one of them again to a factory raises cleanly rather than
     * silently double-freeing or producing garbage. */
    public function testCollectionOwnershipTransfer()
    {
        $p1 = GEOSGeometry::createPointFromXY(1, 1);
        $p2 = GEOSGeometry::createPointFromXY(2, 2);
        $mp = GEOSGeometry::createCollection(
            GEOS_MULTIPOINT, array($p1, $p2)
        );
        $this->assertEquals(2, $mp->numGeometries());

        /* Both p1 and p2 should now be unusable: their relays were NULLed
         * after handoff. Trying to feed one into another factory raises. */
        try {
            GEOSGeometry::createCollection(GEOS_MULTIPOINT, array($p1));
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('already been consumed', $e->getMessage());
        }
    }
}

FactoriesTest::run();

?>
--EXPECT--
FactoriesTest->testCreatePointFromXY	OK
FactoriesTest->testCreateEmptyPoint	OK
FactoriesTest->testCreatePoint	OK
FactoriesTest->testCreateLineString	OK
FactoriesTest->testCreateEmptyLineString	OK
FactoriesTest->testCreateLinearRing	OK
FactoriesTest->testCreatePolygonWithHole	OK
FactoriesTest->testCreatePolygonNoHoles	OK
FactoriesTest->testCreateEmptyPolygon	OK
FactoriesTest->testCreateCollectionMultiPoint	OK
FactoriesTest->testCreateCollectionMultiLineString	OK
FactoriesTest->testCreateGeometryCollection	OK
FactoriesTest->testCreateCollectionInvalidType	OK
FactoriesTest->testCreateEmptyCollection	OK
FactoriesTest->testCreateRectangle	OK
FactoriesTest->testCollectionOwnershipTransfer	OK
