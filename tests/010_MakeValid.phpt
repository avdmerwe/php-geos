--TEST--
GEOSGeometry::makeValid + GEOS_MAKE_VALID_* constants
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class MakeValidTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }
    private function writer() {
        $w = new GEOSWKTWriter();
        if (method_exists($w, 'setTrim')) $w->setTrim(true);
        return $w;
    }

    public function testConstantsRegistered()
    {
        $this->assertTrue(defined('GEOS_MAKE_VALID_LINEWORK'));
        $this->assertTrue(defined('GEOS_MAKE_VALID_STRUCTURE'));
        $this->assertEquals(0, GEOS_MAKE_VALID_LINEWORK);
        $this->assertEquals(1, GEOS_MAKE_VALID_STRUCTURE);
    }

    public function testMakeValidNoArgs()
    {
        /* Bowtie self-intersecting polygon */
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        $v = $g->makeValid();
        $this->assertNotNull($v);
        /* Result is valid */
        $this->assertEquals(0, $v->checkValidity()['valid'] ? 0 : 1);
    }

    public function testMakeValidEmptyParams()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        $v = $g->makeValid(array());
        $this->assertNotNull($v);
    }

    public function testMakeValidLineworkMethod()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        $v = $g->makeValid(array('method' => 'linework'));
        $this->assertNotNull($v);
    }

    public function testMakeValidStructureMethod()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        $v = $g->makeValid(array('method' => 'structure',
                                  'keep_collapsed' => true));
        $this->assertNotNull($v);
    }

    public function testMakeValidKeepCollapsedFalse()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        $v = $g->makeValid(array('method' => 'structure',
                                  'keep_collapsed' => false));
        $this->assertNotNull($v);
    }

    public function testMakeValidBadMethodThrows()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 1, 0 1, 1 0, 0 0))');
        try {
            $g->makeValid(array('method' => 'frobnicate'));
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('linework', $e->getMessage());
        }
    }

    public function testMakeValidOnAlreadyValid()
    {
        $g = $this->reader()->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $v = $g->makeValid();
        $this->assertNotNull($v);
    }
}

MakeValidTest::run();

?>
--EXPECT--
MakeValidTest->testConstantsRegistered	OK
MakeValidTest->testMakeValidNoArgs	OK
MakeValidTest->testMakeValidEmptyParams	OK
MakeValidTest->testMakeValidLineworkMethod	OK
MakeValidTest->testMakeValidStructureMethod	OK
MakeValidTest->testMakeValidKeepCollapsedFalse	OK
MakeValidTest->testMakeValidBadMethodThrows	OK
MakeValidTest->testMakeValidOnAlreadyValid	OK
