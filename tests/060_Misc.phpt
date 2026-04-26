--TEST--
P2 fillers — hilbertCode / orientationIndex / segmentIntersection / gridIntersectionFractions (Item 20)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class MiscTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    private function box($xmin, $ymin, $xmax, $ymax)
    {
        return $this->reader()->read(
            "POLYGON(($xmin $ymin, $xmax $ymin, "
            . "$xmax $ymax, $xmin $ymax, $xmin $ymin))");
    }

    public function testHilbertCodeReturnsInt()
    {
        $extent = $this->box(0, 0, 100, 100);
        $point = $this->reader()->read("POINT(50 50)");

        $code = $point->hilbertCode($extent, 5);
        $this->assertTrue(is_int($code));
        $this->assertTrue($code >= 0);
    }

    public function testHilbertCodeDifferentPointsDifferentCodes()
    {
        $extent = $this->box(0, 0, 100, 100);
        $a = $this->reader()->read("POINT(5 5)");
        $b = $this->reader()->read("POINT(95 95)");

        $codeA = $a->hilbertCode($extent, 5);
        $codeB = $b->hilbertCode($extent, 5);
        $this->assertTrue($codeA !== $codeB);
    }

    public function testHilbertCodeBadLevelThrows()
    {
        $extent = $this->box(0, 0, 100, 100);
        $point = $this->reader()->read("POINT(50 50)");
        try {
            $point->hilbertCode($extent, 17);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains("level", $e->getMessage());
        }
    }

    public function testOrientationIndexCollinear()
    {
        /* (0,0)→(2,2): point (1,1) is exactly on the line. */
        $idx = GEOSOrientationIndex(0, 0, 2, 2, 1, 1);
        $this->assertEquals(0, $idx);
    }

    public function testOrientationIndexCCW()
    {
        /* (0,0)→(1,0): point (0,1) is to the left → CCW (+1). */
        $idx = GEOSOrientationIndex(0, 0, 1, 0, 0, 1);
        $this->assertEquals(1, $idx);
    }

    public function testOrientationIndexCW()
    {
        /* (0,0)→(1,0): point (0,-1) is to the right → CW (-1). */
        $idx = GEOSOrientationIndex(0, 0, 1, 0, 0, -1);
        $this->assertEquals(-1, $idx);
    }

    public function testSegmentIntersectionCrossing()
    {
        /* (0,0)→(2,2) crossed by (0,2)→(2,0) → meets at (1,1). */
        $r = GEOSSegmentIntersection(0, 0, 2, 2, 0, 2, 2, 0);
        $this->assertTrue(is_array($r));
        $this->assertEquals(2, count($r));
        $this->assertEquals(1.0, $r[0]);
        $this->assertEquals(1.0, $r[1]);
    }

    public function testSegmentIntersectionParallelReturnsNull()
    {
        /* Two parallel horizontal segments — no intersection. */
        $r = GEOSSegmentIntersection(0, 0, 1, 0, 0, 1, 1, 1);
        $this->assertNull($r);
    }

    public function testGridIntersectionFractionsBasic()
    {
        /* A polygon covering [0,10]x[0,10] inside a 10x10 grid bounded by
         * [0,10]x[0,10] should yield ~1.0 in every cell. We do not assert
         * on exact float equality; just that the array is the right shape
         * and values are in [0,1]. */
        $g = $this->box(0, 0, 10, 10);
        $arr = $g->gridIntersectionFractions(0, 0, 10, 10, 5, 5);
        $this->assertTrue(is_array($arr));
        $this->assertEquals(25, count($arr));
        foreach ($arr as $v) {
            if ($v < 0.0 || $v > 1.0001) {
                throw new Exception("fraction out of [0,1]: $v");
            }
        }
    }

    public function testGridIntersectionFractionsBadDims()
    {
        $g = $this->box(0, 0, 10, 10);
        try {
            $g->gridIntersectionFractions(0, 0, 10, 10, 0, 5);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains("nx and ny must be > 0", $e->getMessage());
        }
    }
}

MiscTest::run();

?>
--EXPECT--
MiscTest->testHilbertCodeReturnsInt	OK
MiscTest->testHilbertCodeDifferentPointsDifferentCodes	OK
MiscTest->testHilbertCodeBadLevelThrows	OK
MiscTest->testOrientationIndexCollinear	OK
MiscTest->testOrientationIndexCCW	OK
MiscTest->testOrientationIndexCW	OK
MiscTest->testSegmentIntersectionCrossing	OK
MiscTest->testSegmentIntersectionParallelReturnsNull	OK
MiscTest->testGridIntersectionFractionsBasic	OK
MiscTest->testGridIntersectionFractionsBadDims	OK
