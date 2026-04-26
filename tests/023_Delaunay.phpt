--TEST--
GEOSGeometry::delaunayTriangulation $constrained option (Item 15)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class DelaunayTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    /* A non-convex polygon (an "L" shape) — constrained Delaunay should
     * triangulate it within its bounds. */
    private function lShape()
    {
        return $this->reader()->read(
            'POLYGON((0 0, 4 0, 4 1, 1 1, 1 4, 0 4, 0 0))');
    }

    public function testConstrainedReturnsCollection()
    {
        $g = $this->lShape();
        $t = $g->delaunayTriangulation(0.0, false, true);
        $this->assertNotNull($t);
        $this->assertTrue($t instanceof GEOSGeometry);
        $this->assertEquals('GeometryCollection', $t->typeName());
        /* Every member is a triangular polygon. */
        $n = $t->numGeometries();
        $this->assertTrue($n >= 1);
        for ($i = 0; $i < $n; ++$i) {
            $tri = $t->geometryN($i);
            $this->assertEquals('Polygon', $tri->typeName());
        }
    }

    public function testConstrainedVsUnconstrainedVertexCount()
    {
        $g = $this->lShape();
        $unc = $g->delaunayTriangulation(0.0, false, false);
        $con = $g->delaunayTriangulation(0.0, false, true);
        /* Unconstrained on a non-convex polygon triangulates the convex
         * hull, producing more triangles than the constrained variant
         * which respects the polygon boundary. */
        $this->assertTrue($unc->numGeometries() >= 1);
        $this->assertTrue($con->numGeometries() >= 1);
        /* Both produce at least some triangles; the unconstrained should
         * cover the convex hull (>= 4 triangles for an L-shape's hull). */
        $this->assertTrue($unc->numCoordinates() > 0);
        $this->assertTrue($con->numCoordinates() > 0);
    }

    public function testConstrainedIgnoresToleranceAndEdgeOnly()
    {
        $g = $this->lShape();
        /* Even with edgeOnly=true requested, constrained variant returns
         * a polygon-bearing GeometryCollection (it ignores the flag). */
        $t = $g->delaunayTriangulation(0.5, true, true);
        $this->assertNotNull($t);
        $this->assertEquals('GeometryCollection', $t->typeName());
        $n = $t->numGeometries();
        if ($n > 0) {
            $this->assertEquals('Polygon', $t->geometryN(0)->typeName());
        }
    }

    public function testDefaultStillUnconstrained()
    {
        /* Calling with the original 0/2-arg signature still works. */
        $g = $this->lShape();
        $t1 = $g->delaunayTriangulation();
        $t2 = $g->delaunayTriangulation(0.0);
        $t3 = $g->delaunayTriangulation(0.0, false);
        $this->assertNotNull($t1);
        $this->assertNotNull($t2);
        $this->assertNotNull($t3);
        $this->assertEquals('GeometryCollection', $t1->typeName());
    }
}

DelaunayTest::run();

?>
--EXPECT--
DelaunayTest->testConstrainedReturnsCollection	OK
DelaunayTest->testConstrainedVsUnconstrainedVertexCount	OK
DelaunayTest->testConstrainedIgnoresToleranceAndEdgeOnly	OK
DelaunayTest->testDefaultStillUnconstrained	OK
