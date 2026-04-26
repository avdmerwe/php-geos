--TEST--
Hull / metric extras — bounding circle, rotated rectangle, inscribed circle, etc.
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class HullExtrasTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    public function testMinimumBoundingCircle()
    {
        /* 1x1 square — bounding circle radius is sqrt(2)/2 ≈ 0.707, so
         * area ≈ pi/2 ≈ 1.5708. */
        $g = $this->reader()->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $c = $g->minimumBoundingCircle();
        $this->assertNotNull($c);
        $a = $c->area();
        $this->assertTrue($a > 1.5 && $a < 1.6);
    }

    public function testMinimumRotatedRectangleTilted()
    {
        $g = $this->reader()->read(
            'POLYGON((0 0, 4 1, 5 5, 1 4, 0 0))');
        $r = $g->minimumRotatedRectangle();
        $this->assertNotNull($r);
        $this->assertTrue($r->area() > 0);
    }

    public function testMinimumWidth()
    {
        $g = $this->reader()->read('POLYGON((0 0, 4 0, 4 1, 0 1, 0 0))');
        $w = $g->minimumWidth();
        $this->assertNotNull($w);
        /* Should be a LINESTRING */
        $this->assertEquals('LineString', $w->typeName());
    }

    public function testMinimumClearance()
    {
        $g = $this->reader()->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $c = $g->minimumClearance();
        $this->assertTrue(is_float($c));
        $this->assertTrue($c > 0);
    }

    public function testMinimumClearanceLine()
    {
        $g = $this->reader()->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $l = $g->minimumClearanceLine();
        $this->assertNotNull($l);
    }

    public function testMaximumInscribedCircle()
    {
        /* 10x10 square — max inscribed circle has radius 5. */
        $g = $this->reader()->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        $c = $g->maximumInscribedCircle(0.5);
        $this->assertNotNull($c);
        /* radius == length of returned LINESTRING */
        $this->assertTrue($c->length() > 4 && $c->length() < 6);
    }

    public function testMaximumInscribedCircleRequiresArg()
    {
        $g = $this->reader()->read('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))');
        try {
            $g->maximumInscribedCircle();
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }

    public function testLargestEmptyCircleNoBoundary()
    {
        /* Two distant points — the largest empty circle can sit between
         * them.  GEOS clips to the envelope by default. */
        $g = $this->reader()->read('MULTIPOINT(0 0, 10 0)');
        $c = $g->largestEmptyCircle(0.5);
        $this->assertNotNull($c);
    }

    public function testLargestEmptyCircleWithBoundary()
    {
        $g = $this->reader()->read('MULTIPOINT(0 0, 10 0)');
        $bnd = $this->reader()->read(
            'POLYGON((-1 -5, 11 -5, 11 5, -1 5, -1 -5))');
        $c = $g->largestEmptyCircle(0.5, $bnd);
        $this->assertNotNull($c);
    }
}

HullExtrasTest::run();

?>
--EXPECT--
HullExtrasTest->testMinimumBoundingCircle	OK
HullExtrasTest->testMinimumRotatedRectangleTilted	OK
HullExtrasTest->testMinimumWidth	OK
HullExtrasTest->testMinimumClearance	OK
HullExtrasTest->testMinimumClearanceLine	OK
HullExtrasTest->testMaximumInscribedCircle	OK
HullExtrasTest->testMaximumInscribedCircleRequiresArg	OK
HullExtrasTest->testLargestEmptyCircleNoBoundary	OK
HullExtrasTest->testLargestEmptyCircleWithBoundary	OK
