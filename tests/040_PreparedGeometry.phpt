--TEST--
GEOSPreparedGeometry (Item 18)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class PreparedGeometryTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    private function box($xmin, $ymin, $xmax, $ymax)
    {
        return $this->reader()->read(
            "POLYGON(($xmin $ymin, $xmax $ymin, "
            . "$xmax $ymax, $xmin $ymax, $xmin $ymin))");
    }

    public function testClassExists()
    {
        $this->assertTrue(class_exists('GEOSPreparedGeometry'));
    }

    public function testPrepareFactoryReturnsPreparedGeometry()
    {
        $g = $this->box(0, 0, 10, 10);
        $pg = $g->prepare();
        $this->assertTrue($pg instanceof GEOSPreparedGeometry);
    }

    public function testConstructLocked()
    {
        try {
            $pg = new GEOSPreparedGeometry();
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('cannot be constructed directly',
                $e->getMessage());
        }
    }

    public function testContains()
    {
        $outer = $this->box(0, 0, 10, 10)->prepare();
        $inner = $this->box(2, 2, 5, 5);
        $disjoint = $this->box(20, 20, 25, 25);

        $this->assertTrue($outer->contains($inner));
        $this->assertFalse($outer->contains($disjoint));
    }

    public function testContainsProperly()
    {
        $outer = $this->box(0, 0, 10, 10)->prepare();
        /* A box sharing the boundary is contained, but NOT properly. */
        $boundary = $this->box(0, 0, 5, 5);
        $strictlyInner = $this->box(2, 2, 5, 5);

        $this->assertFalse($outer->containsProperly($boundary));
        $this->assertTrue($outer->containsProperly($strictlyInner));
    }

    public function testContainsXY()
    {
        $pg = $this->box(0, 0, 10, 10)->prepare();
        $this->assertTrue($pg->containsXY(5.0, 5.0));
        $this->assertFalse($pg->containsXY(20.0, 20.0));
    }

    public function testCoveredBy()
    {
        $small = $this->box(2, 2, 5, 5)->prepare();
        $big = $this->box(0, 0, 10, 10);
        $other = $this->box(20, 20, 25, 25);

        $this->assertTrue($small->coveredBy($big));
        $this->assertFalse($small->coveredBy($other));
    }

    public function testCovers()
    {
        $big = $this->box(0, 0, 10, 10)->prepare();
        $small = $this->box(2, 2, 5, 5);
        $disjoint = $this->box(20, 20, 25, 25);

        $this->assertTrue($big->covers($small));
        $this->assertFalse($big->covers($disjoint));
    }

    public function testCrosses()
    {
        $line = $this->reader()->read("LINESTRING(0 0, 10 10)")->prepare();
        $crossing = $this->reader()->read("LINESTRING(0 10, 10 0)");
        $parallel = $this->reader()->read("LINESTRING(0 1, 10 11)");

        $this->assertTrue($line->crosses($crossing));
        $this->assertFalse($line->crosses($parallel));
    }

    public function testDisjoint()
    {
        $a = $this->box(0, 0, 5, 5)->prepare();
        $far = $this->box(20, 20, 25, 25);
        $touching = $this->box(2, 2, 4, 4);

        $this->assertTrue($a->disjoint($far));
        $this->assertFalse($a->disjoint($touching));
    }

    public function testIntersects()
    {
        $a = $this->box(0, 0, 5, 5)->prepare();
        $b = $this->box(3, 3, 8, 8);
        $far = $this->box(20, 20, 25, 25);

        $this->assertTrue($a->intersects($b));
        $this->assertFalse($a->intersects($far));
    }

    public function testIntersectsXY()
    {
        $pg = $this->box(0, 0, 10, 10)->prepare();
        $this->assertTrue($pg->intersectsXY(5.0, 5.0));
        $this->assertFalse($pg->intersectsXY(50.0, 50.0));
    }

    public function testOverlaps()
    {
        $a = $this->box(0, 0, 5, 5)->prepare();
        $b = $this->box(3, 3, 8, 8);
        $contained = $this->box(1, 1, 2, 2);

        $this->assertTrue($a->overlaps($b));
        /* Containment is not overlap (per OGC SFS). */
        $this->assertFalse($a->overlaps($contained));
    }

    public function testTouches()
    {
        $a = $this->box(0, 0, 5, 5)->prepare();
        $touching = $this->box(5, 0, 10, 5);
        $far = $this->box(20, 20, 25, 25);

        $this->assertTrue($a->touches($touching));
        $this->assertFalse($a->touches($far));
    }

    public function testWithin()
    {
        $small = $this->box(2, 2, 4, 4)->prepare();
        $big = $this->box(0, 0, 10, 10);
        $far = $this->box(20, 20, 25, 25);

        $this->assertTrue($small->within($big));
        $this->assertFalse($small->within($far));
    }

    public function testRelateReturnsImString()
    {
        $a = $this->box(0, 0, 10, 10)->prepare();
        $b = $this->box(2, 2, 5, 5);

        $im = $a->relate($b);
        $this->assertTrue(is_string($im));
        $this->assertEquals(9, strlen($im));
    }

    public function testRelatePattern()
    {
        $a = $this->box(0, 0, 10, 10)->prepare();
        $b = $this->box(2, 2, 5, 5);

        /* "T*****FF*" is the OGC pattern for "contains". */
        $this->assertTrue($a->relatePattern($b, "T*****FF*"));

        $disjoint = $this->box(20, 20, 25, 25);
        /* Two disjoint boxes should NOT match the contains pattern. */
        $this->assertFalse($a->relatePattern($disjoint, "T*****FF*"));
    }

    public function testDistance()
    {
        $a = $this->reader()->read("POINT(0 0)")->prepare();
        $b = $this->reader()->read("POINT(3 4)");

        $d = $a->distance($b);
        /* Pythagorean distance — exact for these inputs. */
        $this->assertEquals(5.0, $d);
    }

    public function testDistanceWithin()
    {
        $a = $this->reader()->read("POINT(0 0)")->prepare();
        $b = $this->reader()->read("POINT(3 4)");

        $this->assertTrue($a->distanceWithin($b, 5.0));
        $this->assertTrue($a->distanceWithin($b, 6.0));
        $this->assertFalse($a->distanceWithin($b, 4.0));
    }

    public function testNearestPoints()
    {
        $a = $this->box(0, 0, 5, 5)->prepare();
        $b = $this->box(10, 10, 12, 12);

        $cs = $a->nearestPoints($b);
        $this->assertTrue($cs instanceof GEOSCoordSeq);
        $this->assertEquals(2, $cs->getSize());
    }

    /**
     * Lifecycle: prepare from a temp source geometry, drop the source
     * variable, verify the prepared geom still works (refcount kept it alive).
     */
    public function testLifecycleRefcount()
    {
        $source = $this->box(0, 0, 10, 10);
        $pg = $source->prepare();

        unset($source);
        if (function_exists('gc_collect_cycles')) {
            gc_collect_cycles();
        }

        /* Source should still be alive courtesy of the prepared-geom relay. */
        $probe = $this->box(2, 2, 5, 5);
        $this->assertTrue($pg->contains($probe));
    }
}

PreparedGeometryTest::run();

?>
--EXPECT--
PreparedGeometryTest->testClassExists	OK
PreparedGeometryTest->testPrepareFactoryReturnsPreparedGeometry	OK
PreparedGeometryTest->testConstructLocked	OK
PreparedGeometryTest->testContains	OK
PreparedGeometryTest->testContainsProperly	OK
PreparedGeometryTest->testContainsXY	OK
PreparedGeometryTest->testCoveredBy	OK
PreparedGeometryTest->testCovers	OK
PreparedGeometryTest->testCrosses	OK
PreparedGeometryTest->testDisjoint	OK
PreparedGeometryTest->testIntersects	OK
PreparedGeometryTest->testIntersectsXY	OK
PreparedGeometryTest->testOverlaps	OK
PreparedGeometryTest->testTouches	OK
PreparedGeometryTest->testWithin	OK
PreparedGeometryTest->testRelateReturnsImString	OK
PreparedGeometryTest->testRelatePattern	OK
PreparedGeometryTest->testDistance	OK
PreparedGeometryTest->testDistanceWithin	OK
PreparedGeometryTest->testNearestPoints	OK
PreparedGeometryTest->testLifecycleRefcount	OK
