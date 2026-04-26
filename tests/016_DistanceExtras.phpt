--TEST--
Distance / metric extras — distanceWithin, nearestPoints, frechetDistance, etc.
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class DistanceExtrasTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    public function testDistanceWithinTrue()
    {
        $a = $this->reader()->read('POINT(0 0)');
        $b = $this->reader()->read('POINT(3 4)');
        $this->assertTrue($a->distanceWithin($b, 6.0));
    }

    public function testDistanceWithinFalse()
    {
        $a = $this->reader()->read('POINT(0 0)');
        $b = $this->reader()->read('POINT(3 4)');
        $this->assertFalse($a->distanceWithin($b, 4.0));
    }

    public function testDistanceWithinRequiresMaxDist()
    {
        $a = $this->reader()->read('POINT(0 0)');
        $b = $this->reader()->read('POINT(3 4)');
        try {
            $a->distanceWithin($b);
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }

    public function testDistanceIndexed()
    {
        $a = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $b = $this->reader()->read('LINESTRING(5 3, 5 7)');
        $d = $a->distanceIndexed($b);
        $this->assertEquals(3.0, $d);
    }

    public function testNearestPointsReturnsCoordSeq()
    {
        $a = $this->reader()->read('POINT(0 0)');
        $b = $this->reader()->read('POINT(3 4)');
        $cs = $a->nearestPoints($b);
        $this->assertNotNull($cs);
        $this->assertTrue($cs instanceof GEOSCoordSeq);
        $this->assertEquals(2, $cs->getSize());
        /* First point on $a, second on $b */
        $this->assertEquals(0.0, $cs->getX(0));
        $this->assertEquals(0.0, $cs->getY(0));
        $this->assertEquals(3.0, $cs->getX(1));
        $this->assertEquals(4.0, $cs->getY(1));
    }

    public function testNearestPointsRequiresOther()
    {
        $a = $this->reader()->read('POINT(0 0)');
        try {
            $a->nearestPoints();
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }

    public function testFrechetDistance()
    {
        $a = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $b = $this->reader()->read('LINESTRING(0 1, 10 1)');
        $d = $a->frechetDistance($b);
        $this->assertTrue(is_float($d));
        $this->assertTrue($d >= 1.0);
    }

    public function testFrechetDistanceDensify()
    {
        $a = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $b = $this->reader()->read('LINESTRING(0 1, 10 1)');
        $d = $a->frechetDistanceDensify($b, 0.1);
        $this->assertTrue(is_float($d));
        $this->assertTrue($d > 0);
    }

    public function testHausdorffDistanceDensify()
    {
        $a = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $b = $this->reader()->read('LINESTRING(0 1, 10 1)');
        $d = $a->hausdorffDistanceDensify($b, 0.1);
        $this->assertTrue(is_float($d));
        $this->assertTrue($d > 0);
    }

    public function testFrechetDistanceRequiresOther()
    {
        $a = $this->reader()->read('LINESTRING(0 0, 1 0)');
        try {
            $a->frechetDistance();
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }
}

DistanceExtrasTest::run();

?>
--EXPECT--
DistanceExtrasTest->testDistanceWithinTrue	OK
DistanceExtrasTest->testDistanceWithinFalse	OK
DistanceExtrasTest->testDistanceWithinRequiresMaxDist	OK
DistanceExtrasTest->testDistanceIndexed	OK
DistanceExtrasTest->testNearestPointsReturnsCoordSeq	OK
DistanceExtrasTest->testNearestPointsRequiresOther	OK
DistanceExtrasTest->testFrechetDistance	OK
DistanceExtrasTest->testFrechetDistanceDensify	OK
DistanceExtrasTest->testHausdorffDistanceDensify	OK
DistanceExtrasTest->testFrechetDistanceRequiresOther	OK
