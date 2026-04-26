--TEST--
Curved geometry constructors — CircularString / CompoundCurve / CurvePolygon
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class CurvedGeomsTest extends GEOSTest
{
    public function testTypeIdConstants()
    {
        /* Pin the curved-geom type-id constants to the values libgeos
         * exposes via enum GEOSGeomTypes (8..12 in 3.14). */
        $this->assertEquals(8,  GEOS_CIRCULARSTRING);
        $this->assertEquals(9,  GEOS_COMPOUNDCURVE);
        $this->assertEquals(10, GEOS_CURVEPOLYGON);
        $this->assertEquals(11, GEOS_MULTICURVE);
        $this->assertEquals(12, GEOS_MULTISURFACE);
    }

    public function testCreateCircularString()
    {
        $cs = new GEOSCoordSeq(3);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 1);
        $cs->setXY(2, 2, 0);

        $g = GEOSGeometry::createCircularString($cs);
        $this->assertEquals('CircularString', $g->typeName());
        $this->assertEquals(GEOS_CIRCULARSTRING, $g->typeId());
    }

    public function testCreateEmptyCircularString()
    {
        $g = GEOSGeometry::createEmptyCircularString();
        $this->assertEquals('CircularString', $g->typeName());
        $this->assertEquals(GEOS_CIRCULARSTRING, $g->typeId());
        $this->assertTrue($g->isEmpty());
    }

    public function testCreateCompoundCurve()
    {
        $cs1 = new GEOSCoordSeq(3);
        $cs1->setXY(0, 0, 0);
        $cs1->setXY(1, 1, 1);
        $cs1->setXY(2, 2, 0);
        $arc = GEOSGeometry::createCircularString($cs1);

        $cs2 = new GEOSCoordSeq(2);
        $cs2->setXY(0, 2, 0);
        $cs2->setXY(1, 3, 0);
        $line = GEOSGeometry::createLineString($cs2);

        $cc = GEOSGeometry::createCompoundCurve(array($arc, $line));
        $this->assertEquals('CompoundCurve', $cc->typeName());
        $this->assertEquals(GEOS_COMPOUNDCURVE, $cc->typeId());
    }

    public function testCreateEmptyCompoundCurve()
    {
        $g = GEOSGeometry::createEmptyCompoundCurve();
        $this->assertEquals('CompoundCurve', $g->typeName());
        $this->assertTrue($g->isEmpty());
    }

    public function testCreateCurvePolygon()
    {
        /* A closed circular ring (start == end). */
        $cs = new GEOSCoordSeq(5);
        $cs->setXY(0, 0,  0);
        $cs->setXY(1, 1,  1);
        $cs->setXY(2, 2,  0);
        $cs->setXY(3, 1, -1);
        $cs->setXY(4, 0,  0);
        $ring = GEOSGeometry::createCircularString($cs);

        $cp = GEOSGeometry::createCurvePolygon($ring);
        $this->assertEquals('CurvePolygon', $cp->typeName());
        $this->assertEquals(GEOS_CURVEPOLYGON, $cp->typeId());
    }

    public function testCreateEmptyCurvePolygon()
    {
        $g = GEOSGeometry::createEmptyCurvePolygon();
        $this->assertEquals('CurvePolygon', $g->typeName());
        $this->assertTrue($g->isEmpty());
    }

    /* Ownership transfer: after passing a CoordSeq into createCircularString,
     * the same CoordSeq cannot be used as input again. We verify that by
     * trying to reuse it in a second factory call — the call raises a
     * "GEOSCoordSeq already consumed" exception. (Calling getSize on the
     * consumed CoordSeq would trigger a fatal E_ERROR from the relay
     * accessor, so we don't test that path; the cleanly-thrown exception
     * from a re-handoff is the contract we expose to PHP code.) */
    public function testCoordSeqOwnershipTransfer()
    {
        $cs = new GEOSCoordSeq(3);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 1);
        $cs->setXY(2, 2, 0);
        $g = GEOSGeometry::createCircularString($cs);
        $this->assertEquals('CircularString', $g->typeName());

        try {
            GEOSGeometry::createCircularString($cs);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('already consumed', $e->getMessage());
        }
    }

    public function testWKTRoundTripCircularString()
    {
        $cs = new GEOSCoordSeq(3);
        $cs->setXY(0, 0, 0);
        $cs->setXY(1, 1, 1);
        $cs->setXY(2, 2, 0);
        $g = GEOSGeometry::createCircularString($cs);

        $writer = new GEOSWKTWriter();
        $writer->setTrim(true);
        $wkt = $writer->write($g);
        $this->assertContains('CIRCULARSTRING', $wkt);

        /* Round-trip via the WKT reader. */
        $reader = new GEOSWKTReader();
        $g2 = $reader->read($wkt);
        $this->assertEquals('CircularString', $g2->typeName());
    }

    public function testWKTRoundTripCompoundCurve()
    {
        $cs1 = new GEOSCoordSeq(3);
        $cs1->setXY(0, 0, 0); $cs1->setXY(1, 1, 1); $cs1->setXY(2, 2, 0);
        $arc = GEOSGeometry::createCircularString($cs1);

        $cs2 = new GEOSCoordSeq(2);
        $cs2->setXY(0, 2, 0); $cs2->setXY(1, 3, 0);
        $line = GEOSGeometry::createLineString($cs2);

        $cc = GEOSGeometry::createCompoundCurve(array($arc, $line));
        $writer = new GEOSWKTWriter();
        $writer->setTrim(true);
        $wkt = $writer->write($cc);
        $this->assertContains('COMPOUNDCURVE', $wkt);

        $reader = new GEOSWKTReader();
        $cc2 = $reader->read($wkt);
        $this->assertEquals('CompoundCurve', $cc2->typeName());
    }

    /* Curved-geometry overlay/buffer ops are still NOT supported in 3.14:
     * GEOSIntersection/Buffer raise UnsupportedOperationException internally.
     * Document that contract here so a future libgeos that lifts the limit
     * trips this test (and prompts an explicit re-evaluation). */
    public function testIntersectionOnCurvedThrows()
    {
        $cs = new GEOSCoordSeq(3);
        $cs->setXY(0, 0, 0); $cs->setXY(1, 1, 1); $cs->setXY(2, 2, 0);
        $a = GEOSGeometry::createCircularString($cs);

        $cs2 = new GEOSCoordSeq(3);
        $cs2->setXY(0, 0, 0); $cs2->setXY(1, 1, 1); $cs2->setXY(2, 2, 0);
        $b = GEOSGeometry::createCircularString($cs2);

        try {
            $a->intersection($b);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('Curved geometry', $e->getMessage());
        }
    }
}

CurvedGeomsTest::run();

?>
--EXPECT--
CurvedGeomsTest->testTypeIdConstants	OK
CurvedGeomsTest->testCreateCircularString	OK
CurvedGeomsTest->testCreateEmptyCircularString	OK
CurvedGeomsTest->testCreateCompoundCurve	OK
CurvedGeomsTest->testCreateEmptyCompoundCurve	OK
CurvedGeomsTest->testCreateCurvePolygon	OK
CurvedGeomsTest->testCreateEmptyCurvePolygon	OK
CurvedGeomsTest->testCoordSeqOwnershipTransfer	OK
CurvedGeomsTest->testWKTRoundTripCircularString	OK
CurvedGeomsTest->testWKTRoundTripCompoundCurve	OK
CurvedGeomsTest->testIntersectionOnCurvedThrows	OK
