--TEST--
GEOSGeometry coordinate-transform callbacks (Item 19)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class TransformTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }
    private function writer() {
        $w = new GEOSWKTWriter();
        $w->setRoundingPrecision(2);
        $w->setTrim(true);
        return $w;
    }

    public function testTransformXYTranslatePoint()
    {
        $p = $this->reader()->read("POINT(1 2)");
        $shifted = $p->transformXY(function ($x, $y) {
            return [$x + 10, $y + 20];
        });
        $this->assertEquals("POINT (11 22)", $this->writer()->write($shifted));
    }

    public function testTransformXYScaleLineString()
    {
        $line = $this->reader()->read("LINESTRING(0 0, 1 1, 2 0)");
        $scaled = $line->transformXY(function ($x, $y) {
            return [$x * 2, $y * 3];
        });
        $this->assertEquals("LINESTRING (0 0, 2 3, 4 0)",
            $this->writer()->write($scaled));
    }

    public function testTransformXYZ()
    {
        $p = $this->reader()->read("POINT Z (1 2 3)");
        $shifted = $p->transformXYZ(function ($x, $y, $z) {
            return [$x + 1, $y + 1, $z * 2];
        });

        $w = new GEOSWKTWriter();
        $w->setRoundingPrecision(2);
        $w->setTrim(true);
        $w->setOutputDimension(3);
        $this->assertEquals("POINT Z (2 3 6)", $w->write($shifted));
    }

    public function testTransformXYReturnsNewGeometry()
    {
        $p = $this->reader()->read("POINT(0 0)");
        $shifted = $p->transformXY(function ($x, $y) {
            return [$x + 1, $y + 1];
        });
        /* Original is untouched (immutability of geometries). */
        $this->assertEquals("POINT (0 0)", $this->writer()->write($p));
        $this->assertEquals("POINT (1 1)", $this->writer()->write($shifted));
    }

    public function testTransformXYThrowingCallbackPropagates()
    {
        $p = $this->reader()->read("POINT(1 2)");
        try {
            $p->transformXY(function ($x, $y) {
                throw new Exception("boom");
            });
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertEquals("boom", $e->getMessage());
        }
    }

    public function testTransformXYNonArrayReturnThrows()
    {
        $p = $this->reader()->read("POINT(1 2)");
        try {
            $p->transformXY(function ($x, $y) { return 42; });
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains("must return an array", $e->getMessage());
        }
    }

    public function testTransformXYWrongLengthReturnThrows()
    {
        $p = $this->reader()->read("POINT(1 2)");
        try {
            $p->transformXY(function ($x, $y) { return [$x]; });
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains("wrong length", $e->getMessage());
        }
    }

    public function testTransformXYNotCallableThrows()
    {
        $p = $this->reader()->read("POINT(1 2)");
        try {
            $p->transformXY("definitely_not_a_function_xyzzy");
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains("not a valid callable", $e->getMessage());
        }
    }
}

TransformTest::run();

?>
--EXPECT--
TransformTest->testTransformXYTranslatePoint	OK
TransformTest->testTransformXYScaleLineString	OK
TransformTest->testTransformXYZ	OK
TransformTest->testTransformXYReturnsNewGeometry	OK
TransformTest->testTransformXYThrowingCallbackPropagates	OK
TransformTest->testTransformXYNonArrayReturnThrows	OK
TransformTest->testTransformXYWrongLengthReturnThrows	OK
TransformTest->testTransformXYNotCallableThrows	OK
