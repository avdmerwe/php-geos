--TEST--
Precision-grid overlays — intersection/difference/symDifference/union with optional gridSize
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class PrecOverlaysTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    public function testIntersectionNoGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->intersection($b);
        $this->assertNotNull($r);
        $this->assertTrue($r->area() > 0);
    }

    public function testIntersectionWithGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->intersection($b, 1.0);
        $this->assertNotNull($r);
    }

    public function testDifferenceNoGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->difference($b);
        $this->assertNotNull($r);
    }

    public function testDifferenceWithGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->difference($b, 1.0);
        $this->assertNotNull($r);
    }

    public function testSymDifferenceNoGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->symDifference($b);
        $this->assertNotNull($r);
    }

    public function testSymDifferenceWithGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->symDifference($b, 1.0);
        $this->assertNotNull($r);
    }

    public function testUnaryUnionNoArgs()
    {
        $g = $this->reader()->read(
            'MULTIPOLYGON(((0 0, 2 0, 2 2, 0 2, 0 0)),'.
            '((1 1, 3 1, 3 3, 1 3, 1 1)))');
        $r = $g->union();
        $this->assertNotNull($r);
    }

    public function testUnaryUnionWithGrid()
    {
        $g = $this->reader()->read(
            'MULTIPOLYGON(((0 0, 2 0, 2 2, 0 2, 0 0)),'.
            '((1 1, 3 1, 3 3, 1 3, 1 1)))');
        $r = $g->union(null, 1.0);
        $this->assertNotNull($r);
    }

    public function testBinaryUnionNoGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->union($b);
        $this->assertNotNull($r);
    }

    public function testBinaryUnionWithGrid()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        $b = $this->reader()->read('POLYGON((1 1, 3 1, 3 3, 1 3, 1 1))');
        $r = $a->union($b, 1.0);
        $this->assertNotNull($r);
    }

    public function testIntersectionRequiresOther()
    {
        $a = $this->reader()->read('POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))');
        try {
            $a->intersection();
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }
}

PrecOverlaysTest::run();

?>
--EXPECT--
PrecOverlaysTest->testIntersectionNoGrid	OK
PrecOverlaysTest->testIntersectionWithGrid	OK
PrecOverlaysTest->testDifferenceNoGrid	OK
PrecOverlaysTest->testDifferenceWithGrid	OK
PrecOverlaysTest->testSymDifferenceNoGrid	OK
PrecOverlaysTest->testSymDifferenceWithGrid	OK
PrecOverlaysTest->testUnaryUnionNoArgs	OK
PrecOverlaysTest->testUnaryUnionWithGrid	OK
PrecOverlaysTest->testBinaryUnionNoGrid	OK
PrecOverlaysTest->testBinaryUnionWithGrid	OK
PrecOverlaysTest->testIntersectionRequiresOther	OK
