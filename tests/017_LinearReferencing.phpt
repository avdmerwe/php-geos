--TEST--
Linear referencing extras — lineSubstring + lineMergeDirected
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class LinearReferencingTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }
    private function writer() {
        $w = new GEOSWKTWriter();
        if (method_exists($w, 'setTrim')) $w->setTrim(true);
        return $w;
    }

    public function testLineSubstringMidHalf()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $sub = $g->lineSubstring(0.25, 0.75);
        $this->assertNotNull($sub);
        /* substring of half the length of a length-10 line is length 5 */
        $this->assertEquals(5.0, $sub->length());
    }

    public function testLineSubstringFullRange()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 10 0)');
        $sub = $g->lineSubstring(0.0, 1.0);
        $this->assertEquals(10.0, $sub->length());
    }

    public function testLineSubstringRequiresArgs()
    {
        $g = $this->reader()->read('LINESTRING(0 0, 10 0)');
        try {
            $g->lineSubstring(0.0);
            $this->assertTrue(false);
        } catch (Throwable $e) {
            $this->assertNotNull($e);
        }
    }

    public function testLineMergeDirected()
    {
        /* Two segments in a consistent direction should merge. */
        $g = $this->reader()->read(
            'MULTILINESTRING((0 0, 5 0), (5 0, 10 0))');
        $merged = $g->lineMergeDirected();
        $this->assertNotNull($merged);
        $this->assertEquals('LineString', $merged->typeName());
    }

    public function testLineMergeDirectedNoMerge()
    {
        /* Two segments going AWAY from a common point — directed merge
         * should NOT collapse them. */
        $g = $this->reader()->read(
            'MULTILINESTRING((5 0, 0 0), (5 0, 10 0))');
        $merged = $g->lineMergeDirected();
        $this->assertNotNull($merged);
    }
}

LinearReferencingTest::run();

?>
--EXPECT--
LinearReferencingTest->testLineSubstringMidHalf	OK
LinearReferencingTest->testLineSubstringFullRange	OK
LinearReferencingTest->testLineSubstringRequiresArgs	OK
LinearReferencingTest->testLineMergeDirected	OK
LinearReferencingTest->testLineMergeDirectedNoMerge	OK
