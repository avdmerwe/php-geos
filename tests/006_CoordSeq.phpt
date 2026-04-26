--TEST--
GEOSCoordSeq tests
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class CoordSeqTest extends GEOSTest
{
    public function testConstruct2D()
    {
        $cs = new GEOSCoordSeq(3);
        $this->assertEquals(3, $cs->getSize());
        $this->assertEquals(2, $cs->getDimensions());
        $this->assertFalse($cs->hasZ());
        $this->assertFalse($cs->hasM());
    }

    public function testConstruct3D()
    {
        $cs = new GEOSCoordSeq(2, 3);
        $this->assertEquals(2, $cs->getSize());
        $this->assertEquals(3, $cs->getDimensions());
        $this->assertTrue($cs->hasZ());
        $this->assertFalse($cs->hasM());
    }

    public function testConstruct2DM()
    {
        $cs = new GEOSCoordSeq(2, 2, true);
        $this->assertEquals(2, $cs->getSize());
        /* getDimensions returns total dims incl. M => XYM = 3 */
        $this->assertEquals(3, $cs->getDimensions());
        $this->assertFalse($cs->hasZ());
        $this->assertTrue($cs->hasM());
    }

    public function testConstruct3DM()
    {
        $cs = new GEOSCoordSeq(2, 3, true);
        $this->assertEquals(2, $cs->getSize());
        $this->assertEquals(4, $cs->getDimensions());
        $this->assertTrue($cs->hasZ());
        $this->assertTrue($cs->hasM());
    }

    public function testConstructInvalidDim()
    {
        try {
            $cs = new GEOSCoordSeq(2, 4);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('dim must be 2 or 3', $e->getMessage());
        }
    }

    public function testScalarSetGet2D()
    {
        $cs = new GEOSCoordSeq(2);
        $cs->setX(0, 1.5);
        $cs->setY(0, 2.5);
        $cs->setX(1, 3.0);
        $cs->setY(1, 4.0);
        $this->assertEquals(1.5, $cs->getX(0));
        $this->assertEquals(2.5, $cs->getY(0));
        $this->assertEquals(3.0, $cs->getX(1));
        $this->assertEquals(4.0, $cs->getY(1));
    }

    public function testScalarSetGet3D()
    {
        $cs = new GEOSCoordSeq(1, 3);
        $cs->setX(0, 7);
        $cs->setY(0, 8);
        $cs->setZ(0, 9);
        $this->assertEquals(7, $cs->getX(0));
        $this->assertEquals(8, $cs->getY(0));
        $this->assertEquals(9, $cs->getZ(0));
    }

    public function testScalarSetGetM()
    {
        $cs = new GEOSCoordSeq(1, 2, true);
        $cs->setX(0, 1);
        $cs->setY(0, 2);
        $cs->setM(0, 99);
        $this->assertEquals(1, $cs->getX(0));
        $this->assertEquals(2, $cs->getY(0));
        $this->assertEquals(99, $cs->getM(0));
    }

    public function testSetXYAndGetXY()
    {
        $cs = new GEOSCoordSeq(1);
        $cs->setXY(0, 11.0, 22.0);
        $r = $cs->getXY(0);
        $this->assertEquals(11.0, $r['x']);
        $this->assertEquals(22.0, $r['y']);
    }

    public function testSetXYZAndGetXYZ()
    {
        $cs = new GEOSCoordSeq(1, 3);
        $cs->setXYZ(0, 1.5, 2.5, 3.5);
        $r = $cs->getXYZ(0);
        $this->assertEquals(1.5, $r['x']);
        $this->assertEquals(2.5, $r['y']);
        $this->assertEquals(3.5, $r['z']);
    }

    public function testSetZOn2DThrows()
    {
        $cs = new GEOSCoordSeq(1);
        try {
            $cs->setZ(0, 1.0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('without Z', $e->getMessage());
        }
    }

    public function testGetZOn2DThrows()
    {
        $cs = new GEOSCoordSeq(1);
        $cs->setX(0, 0); $cs->setY(0, 0);
        try {
            $cs->getZ(0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('without Z', $e->getMessage());
        }
    }

    public function testSetMOnNonMThrows()
    {
        $cs = new GEOSCoordSeq(1, 3);
        try {
            $cs->setM(0, 1.0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('without M', $e->getMessage());
        }
    }

    public function testGetMOnNonMThrows()
    {
        $cs = new GEOSCoordSeq(1, 3);
        $cs->setX(0, 0); $cs->setY(0, 0); $cs->setZ(0, 0);
        try {
            $cs->getM(0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('without M', $e->getMessage());
        }
    }

    public function testGetXYZOn2DThrows()
    {
        $cs = new GEOSCoordSeq(1);
        try {
            $cs->getXYZ(0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('without Z', $e->getMessage());
        }
    }

    public function testCopyFromArrays2D()
    {
        $cs = new GEOSCoordSeq(3);
        $cs->copyFromArrays(array(1, 2, 3), array(10, 20, 30));
        $a = $cs->copyToArrays();
        $this->assertEquals(array(1, 2, 3), $a['x']);
        $this->assertEquals(array(10, 20, 30), $a['y']);
        /* z and m absent */
        $this->assertFalse(isset($a['z']));
        $this->assertFalse(isset($a['m']));
    }

    public function testCopyFromArrays3D()
    {
        $cs = new GEOSCoordSeq(3, 3);
        $cs->copyFromArrays(
            array(1.0, 2.0, 3.0),
            array(10.0, 20.0, 30.0),
            array(100.0, 200.0, 300.0)
        );
        $a = $cs->copyToArrays();
        $this->assertEquals(array(1.0, 2.0, 3.0), $a['x']);
        $this->assertEquals(array(10.0, 20.0, 30.0), $a['y']);
        $this->assertEquals(array(100.0, 200.0, 300.0), $a['z']);
        $this->assertFalse(isset($a['m']));
    }

    public function testCopyFromArrays2DM()
    {
        $cs = new GEOSCoordSeq(2, 2, true);
        $cs->copyFromArrays(
            array(1.0, 2.0),
            array(3.0, 4.0),
            null,
            array(99.0, 88.0)
        );
        $a = $cs->copyToArrays();
        $this->assertEquals(array(1.0, 2.0), $a['x']);
        $this->assertEquals(array(3.0, 4.0), $a['y']);
        $this->assertFalse(isset($a['z']));
        $this->assertEquals(array(99.0, 88.0), $a['m']);
    }

    public function testCopyFromArrays3DM()
    {
        $cs = new GEOSCoordSeq(2, 3, true);
        $cs->copyFromArrays(
            array(1, 2),
            array(3, 4),
            array(5, 6),
            array(7, 8)
        );
        $a = $cs->copyToArrays();
        $this->assertEquals(array(1, 2), $a['x']);
        $this->assertEquals(array(3, 4), $a['y']);
        $this->assertEquals(array(5, 6), $a['z']);
        $this->assertEquals(array(7, 8), $a['m']);
    }

    public function testCopyFromArraysMismatchedSize()
    {
        $cs = new GEOSCoordSeq(3);
        try {
            $cs->copyFromArrays(array(1, 2), array(3, 4));
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('coord-seq size', $e->getMessage());
        }
    }

    public function testIndexOutOfRange()
    {
        $cs = new GEOSCoordSeq(2);
        try {
            $cs->setX(5, 1.0);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('out of range', $e->getMessage());
        }
    }

    public function testGetCoordSeqOnPoint()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('POINT(1 2)');
        $cs = $g->getCoordSeq();
        $this->assertEquals(1, $cs->getSize());
        $this->assertEquals(1.0, $cs->getX(0));
        $this->assertEquals(2.0, $cs->getY(0));
    }

    public function testGetCoordSeqOnLineString()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('LINESTRING(0 0, 1 1, 2 2)');
        $cs = $g->getCoordSeq();
        $this->assertEquals(3, $cs->getSize());
        $this->assertEquals(2.0, $cs->getX(2));
        $this->assertEquals(2.0, $cs->getY(2));
    }

    public function testGetCoordSeqOnLinearRing()
    {
        $reader = new GEOSWKTReader();
        /* extract the exterior ring of a polygon */
        $poly = $reader->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $ring = $poly->exteriorRing();
        $cs = $ring->getCoordSeq();
        $this->assertEquals(5, $cs->getSize());
    }

    public function testGetCoordSeqOnPolygonThrows()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        try {
            $g->getCoordSeq();
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('only valid for', $e->getMessage());
        }
    }

    public function testGetCoordSeqOnMultiPointThrows()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('MULTIPOINT((0 0), (1 1))');
        try {
            $g->getCoordSeq();
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('only valid for', $e->getMessage());
        }
    }

    public function testGetCoordSeqIsClone()
    {
        /* Verify the clone semantics: mutating the returned coord-seq
         * should NOT affect the source geometry. */
        $reader = new GEOSWKTReader();
        $writer = new GEOSWKTWriter();
        if (method_exists($writer, 'setTrim')) $writer->setTrim(true);

        $g = $reader->read('POINT(1 2)');
        $cs = $g->getCoordSeq();
        $cs->setX(0, 999);
        $this->assertEquals('POINT (1 2)', $writer->write($g));
    }

    public function testGeometryHasM()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('POINT(1 2)');
        $this->assertFalse($g->hasM());
        $this->assertFalse($g->hasZ());

        $g3 = $reader->read('POINT Z (1 2 3)');
        $this->assertFalse($g3->hasM());
        $this->assertTrue($g3->hasZ());

        $gm = $reader->read('POINT M (1 2 99)');
        $this->assertTrue($gm->hasM());
        $this->assertFalse($gm->hasZ());

        $gzm = $reader->read('POINT ZM (1 2 3 99)');
        $this->assertTrue($gzm->hasM());
        $this->assertTrue($gzm->hasZ());
    }

    public function testGeometryGetZ()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('POINT Z (10 20 30)');
        $this->assertEquals(30.0, $g->getZ());
    }

    public function testGeometryGetM()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('POINT M (10 20 99)');
        $this->assertEquals(99.0, $g->getM());
    }

    public function testGeometryGetZOnNonPointThrows()
    {
        $reader = new GEOSWKTReader();
        $g = $reader->read('LINESTRING(0 0, 1 1)');
        try {
            $g->getZ();
            $this->assertTrue(false);
        } catch (Exception $e) {
            /* GEOS C library throws -- error contents depend on version */
            $this->assertNotNull($e);
        }
    }
}

CoordSeqTest::run();

?>
--EXPECT--
CoordSeqTest->testConstruct2D	OK
CoordSeqTest->testConstruct3D	OK
CoordSeqTest->testConstruct2DM	OK
CoordSeqTest->testConstruct3DM	OK
CoordSeqTest->testConstructInvalidDim	OK
CoordSeqTest->testScalarSetGet2D	OK
CoordSeqTest->testScalarSetGet3D	OK
CoordSeqTest->testScalarSetGetM	OK
CoordSeqTest->testSetXYAndGetXY	OK
CoordSeqTest->testSetXYZAndGetXYZ	OK
CoordSeqTest->testSetZOn2DThrows	OK
CoordSeqTest->testGetZOn2DThrows	OK
CoordSeqTest->testSetMOnNonMThrows	OK
CoordSeqTest->testGetMOnNonMThrows	OK
CoordSeqTest->testGetXYZOn2DThrows	OK
CoordSeqTest->testCopyFromArrays2D	OK
CoordSeqTest->testCopyFromArrays3D	OK
CoordSeqTest->testCopyFromArrays2DM	OK
CoordSeqTest->testCopyFromArrays3DM	OK
CoordSeqTest->testCopyFromArraysMismatchedSize	OK
CoordSeqTest->testIndexOutOfRange	OK
CoordSeqTest->testGetCoordSeqOnPoint	OK
CoordSeqTest->testGetCoordSeqOnLineString	OK
CoordSeqTest->testGetCoordSeqOnLinearRing	OK
CoordSeqTest->testGetCoordSeqOnPolygonThrows	OK
CoordSeqTest->testGetCoordSeqOnMultiPointThrows	OK
CoordSeqTest->testGetCoordSeqIsClone	OK
CoordSeqTest->testGeometryHasM	OK
CoordSeqTest->testGeometryGetZ	OK
CoordSeqTest->testGeometryGetM	OK
CoordSeqTest->testGeometryGetZOnNonPointThrows	OK
