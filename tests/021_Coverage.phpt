--TEST--
GEOSGeometry coverage processing (Item 13)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class CoverageTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }
    private function writer() {
        $w = new GEOSWKTWriter();
        if (method_exists($w, 'setTrim')) $w->setTrim(true);
        return $w;
    }

    /* Two adjacent unit squares forming a valid 2x1 coverage. */
    private function adjacentPair()
    {
        return $this->reader()->read(
            'GEOMETRYCOLLECTION('
            . 'POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)),'
            . 'POLYGON((1 0, 2 0, 2 1, 1 1, 1 0))'
            . ')');
    }

    /* Two squares with a small gap in between: not a valid coverage. */
    private function gappedPair()
    {
        return $this->reader()->read(
            'GEOMETRYCOLLECTION('
            . 'POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)),'
            . 'POLYGON((1.1 0, 2.1 0, 2.1 1, 1.1 1, 1.1 0))'
            . ')');
    }

    public function testOverlapMergeConstantsRegistered()
    {
        $this->assertTrue(defined('GEOS_OVERLAP_MERGE_LONGEST_BORDER'));
        $this->assertTrue(defined('GEOS_OVERLAP_MERGE_MAX_AREA'));
        $this->assertTrue(defined('GEOS_OVERLAP_MERGE_MIN_AREA'));
        $this->assertTrue(defined('GEOS_OVERLAP_MERGE_MIN_INDEX'));
        /* C-enum names also exposed */
        $this->assertTrue(defined('GEOS_MERGE_LONGEST_BORDER'));
        $this->assertTrue(defined('GEOS_MERGE_MAX_AREA'));
        $this->assertTrue(defined('GEOS_MERGE_MIN_AREA'));
        $this->assertTrue(defined('GEOS_MERGE_MIN_INDEX'));
        /* Aliases share enum values */
        $this->assertEquals(GEOS_MERGE_LONGEST_BORDER,
                            GEOS_OVERLAP_MERGE_LONGEST_BORDER);
        $this->assertEquals(GEOS_MERGE_MIN_INDEX,
                            GEOS_OVERLAP_MERGE_MIN_INDEX);
    }

    public function testCoverageUnion()
    {
        $g = $this->adjacentPair();
        $u = $g->coverageUnion();
        $this->assertNotNull($u);
        /* Union of two adjacent unit squares = one 2x1 polygon, area 2 */
        $this->assertEquals(2.0, $u->area());
    }

    public function testCoverageIsValid_validCoverage()
    {
        $g = $this->adjacentPair();
        $r = $g->coverageIsValid();
        $this->assertTrue(is_array($r));
        $this->assertTrue($r['valid']);
        /* GEOSCoverageIsValid always populates invalidEdges (per upstream
         * docs): each polygon gets a MULTILINESTRING of error edges, or an
         * EMPTY where it's a valid participant. So 'invalid' is a
         * GEOSGeometry whose members are all empty for a valid coverage. */
        $this->assertNotNull($r['invalid']);
        $this->assertTrue($r['invalid'] instanceof GEOSGeometry);
        $this->assertTrue($r['invalid']->isEmpty());
    }

    public function testCoverageIsValid_invalidCoverage()
    {
        $g = $this->gappedPair();
        /* gapWidth of 0.5 (> the 0.1 gap) -> coverage detected as invalid */
        $r = $g->coverageIsValid(0.5);
        $this->assertTrue(is_array($r));
        $this->assertFalse($r['valid']);
        $this->assertNotNull($r['invalid']);
        $this->assertTrue($r['invalid'] instanceof GEOSGeometry);
        /* When invalid, the geometry has at least one non-empty member. */
        $this->assertFalse($r['invalid']->isEmpty());
    }

    public function testCoverageSimplifyVW()
    {
        /* A coverage with extra collinear vertices on the shared edge. */
        $g = $this->reader()->read(
            'GEOMETRYCOLLECTION('
            . 'POLYGON((0 0, 0.5 0, 1 0, 1 0.5, 1 1, 0.5 1, 0 1, 0 0.5, 0 0)),'
            . 'POLYGON((1 0, 2 0, 2 1, 1 1, 1 0.5, 1 0))'
            . ')');
        $orig = $g->numCoordinates();
        $s = $g->coverageSimplifyVW(1.0);
        $this->assertNotNull($s);
        $this->assertTrue($s->numCoordinates() <= $orig);
    }

    public function testCoverageClean_emptyParams()
    {
        $g = $this->adjacentPair();
        $c = $g->coverageClean(array());
        $this->assertNotNull($c);
        $this->assertTrue($c instanceof GEOSGeometry);
    }

    public function testCoverageClean_noArgs()
    {
        $g = $this->adjacentPair();
        $c = $g->coverageClean();
        $this->assertNotNull($c);
    }

    public function testCoverageClean_withSnapDistance()
    {
        $g = $this->gappedPair();
        $c = $g->coverageClean(array('snap_distance' => 0.5));
        $this->assertNotNull($c);
        $this->assertTrue($c instanceof GEOSGeometry);
    }

    public function testCoverageClean_withOverlapStrategy()
    {
        $g = $this->adjacentPair();
        $c = $g->coverageClean(array(
            'snap_distance' => 0.0,
            'gap_max_width' => 0.0,
            'overlap_strategy' => GEOS_OVERLAP_MERGE_MIN_INDEX,
        ));
        $this->assertNotNull($c);
    }
}

CoverageTest::run();

?>
--EXPECT--
CoverageTest->testOverlapMergeConstantsRegistered	OK
CoverageTest->testCoverageUnion	OK
CoverageTest->testCoverageIsValid_validCoverage	OK
CoverageTest->testCoverageIsValid_invalidCoverage	OK
CoverageTest->testCoverageSimplifyVW	OK
CoverageTest->testCoverageClean_emptyParams	OK
CoverageTest->testCoverageClean_noArgs	OK
CoverageTest->testCoverageClean_withSnapDistance	OK
CoverageTest->testCoverageClean_withOverlapStrategy	OK
