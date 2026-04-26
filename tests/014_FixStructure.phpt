--TEST--
WKB flavor + reader fix-structure
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class FixStructureTest extends GEOSTest
{
    public function testWKBConstantsRegistered()
    {
        $this->assertTrue(defined('GEOSWKB_EXTENDED'));
        $this->assertTrue(defined('GEOSWKB_ISO'));
        $this->assertEquals(1, GEOSWKB_EXTENDED);
        $this->assertEquals(2, GEOSWKB_ISO);
    }

    public function testWKBWriterDefaultFlavorIsExtended()
    {
        $w = new GEOSWKBWriter();
        $this->assertEquals(GEOSWKB_EXTENDED, $w->getFlavor());
    }

    public function testWKBWriterSetFlavorIso()
    {
        $w = new GEOSWKBWriter();
        $w->setFlavor(GEOSWKB_ISO);
        $this->assertEquals(GEOSWKB_ISO, $w->getFlavor());
    }

    public function testWKBWriterSetFlavorExtended()
    {
        $w = new GEOSWKBWriter();
        $w->setFlavor(GEOSWKB_ISO);
        $w->setFlavor(GEOSWKB_EXTENDED);
        $this->assertEquals(GEOSWKB_EXTENDED, $w->getFlavor());
    }

    public function testFlavorByteLengthsForXYZPoint()
    {
        /* Z-aware POINT.  EXTENDED and ISO flavors encode XYZ differently
         * but produce identical byte lengths for the same geometry; we
         * exercise both paths and verify both produce non-empty output that
         * round-trips to the same WKT. */
        $rdr = new GEOSWKTReader();
        $g = $rdr->read('POINT Z (1 2 3)');

        $we = new GEOSWKBWriter();
        $we->setOutputDimension(3);
        $we->setFlavor(GEOSWKB_EXTENDED);
        $bExt = $we->write($g);

        $wi = new GEOSWKBWriter();
        $wi->setOutputDimension(3);
        $wi->setFlavor(GEOSWKB_ISO);
        $bIso = $wi->write($g);

        $this->assertTrue(strlen($bExt) > 0);
        $this->assertTrue(strlen($bIso) > 0);

        /* Both can be read back. */
        $rdrW = new GEOSWKBReader();
        $g2 = $rdrW->read($bExt);
        $g3 = $rdrW->read($bIso);
        $this->assertNotNull($g2);
        $this->assertNotNull($g3);
        $this->assertTrue($g2->equals($g3));
    }

    public function testWKTReaderSetFixStructureUnclosedRing()
    {
        /* Without setFixStructure, GEOS 3.14 still accepts unclosed rings
         * and silently closes them in newer versions.  We just exercise
         * the setter and confirm the reader still produces a valid
         * polygon afterwards. */
        $rdr = new GEOSWKTReader();
        $rdr->setFixStructure(true);
        /* unclosed ring */
        $g = $rdr->read('POLYGON((0 0, 1 0, 1 1, 0 1))');
        $this->assertNotNull($g);
        $this->assertTrue($g->area() > 0);
    }

    public function testWKTReaderSetFixStructureFalse()
    {
        $rdr = new GEOSWKTReader();
        $rdr->setFixStructure(false);
        $g = $rdr->read('POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))');
        $this->assertNotNull($g);
    }

    public function testWKBReaderSetFixStructure()
    {
        /* Just exercise the setter — verifying it parses something
         * round-trip without errors is sufficient. */
        $reader = new GEOSWKBReader();
        $reader->setFixStructure(false);
        $reader->setFixStructure(true);

        /* And verify normal reading still works after toggling. */
        $rdr = new GEOSWKTReader();
        $g = $rdr->read('POINT(1 2)');
        $w = new GEOSWKBWriter();
        $bytes = $w->write($g);
        $g2 = $reader->read($bytes);
        $this->assertNotNull($g2);
    }
}

FixStructureTest::run();

?>
--EXPECT--
FixStructureTest->testWKBConstantsRegistered	OK
FixStructureTest->testWKBWriterDefaultFlavorIsExtended	OK
FixStructureTest->testWKBWriterSetFlavorIso	OK
FixStructureTest->testWKBWriterSetFlavorExtended	OK
FixStructureTest->testFlavorByteLengthsForXYZPoint	OK
FixStructureTest->testWKTReaderSetFixStructureUnclosedRing	OK
FixStructureTest->testWKTReaderSetFixStructureFalse	OK
FixStructureTest->testWKBReaderSetFixStructure	OK
