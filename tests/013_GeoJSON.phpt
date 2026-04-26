--TEST--
GEOSGeoJSONReader / GEOSGeoJSONWriter — GeoJSON I/O
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class GeoJSONTest extends GEOSTest
{
    public function testReadPoint()
    {
        $reader = new GEOSGeoJSONReader();
        $g = $reader->read('{"type":"Point","coordinates":[1,2]}');
        $this->assertEquals('Point', $g->typeName());
        $writer = new GEOSWKTWriter();
        $writer->setTrim(true);
        $this->assertEquals('POINT (1 2)', $writer->write($g));
    }

    public function testRoundTripPoint()
    {
        $reader = new GEOSGeoJSONReader();
        $writer = new GEOSGeoJSONWriter();
        $g = $reader->read('{"type":"Point","coordinates":[1,2]}');
        $json = $writer->write($g);
        /* Re-read the writer's output and verify equivalence */
        $g2 = $reader->read($json);
        $this->assertEquals('Point', $g2->typeName());
    }

    public function testRoundTripLineString()
    {
        $reader = new GEOSGeoJSONReader();
        $writer = new GEOSGeoJSONWriter();
        $g = $reader->read(
            '{"type":"LineString","coordinates":[[0,0],[1,1],[2,2]]}'
        );
        $this->assertEquals('LineString', $g->typeName());
        $json = $writer->write($g);
        $g2 = $reader->read($json);
        $this->assertEquals('LineString', $g2->typeName());
        $this->assertEquals(3, $g2->numCoordinates());
    }

    public function testRoundTripPolygon()
    {
        $reader = new GEOSGeoJSONReader();
        $writer = new GEOSGeoJSONWriter();
        $g = $reader->read(
            '{"type":"Polygon","coordinates":[[[0,0],[1,0],[1,1],[0,1],[0,0]]]}'
        );
        $this->assertEquals('Polygon', $g->typeName());
        $json = $writer->write($g);
        $g2 = $reader->read($json);
        $this->assertEquals('Polygon', $g2->typeName());
        $this->assertEquals(0, $g2->numInteriorRings());
    }

    public function test3DPointDimensionToggle()
    {
        $wktReader = new GEOSWKTReader();
        $writer = new GEOSGeoJSONWriter();
        $g = $wktReader->read('POINT Z (1 2 3)');

        /* Default dimension is whatever GEOS picks (3 in 3.14). */
        $this->assertTrue($writer->getOutputDimension() >= 2);

        $writer->setOutputDimension(3);
        $this->assertEquals(3, $writer->getOutputDimension());
        $json = $writer->write($g);
        /* The Z value should appear in the coordinates array. */
        $this->assertContains('3.0', $json);

        $writer->setOutputDimension(2);
        $this->assertEquals(2, $writer->getOutputDimension());
        $json2d = $writer->write($g);
        /* In 2D output the third ordinate is dropped. */
        $this->assertContains('[1.0,2.0]', $json2d);
    }

    public function testIndentToggle()
    {
        $reader = new GEOSGeoJSONReader();
        $writer = new GEOSGeoJSONWriter();
        $g = $reader->read('{"type":"Point","coordinates":[1,2]}');

        /* Default is compact (indent = -1). */
        $compact = $writer->write($g);
        /* Compact form has no newlines. */
        $this->assertEquals(false, strpos($compact, "\n"));

        $pretty = $writer->write($g, 2);
        /* Pretty-printed has newlines. */
        $this->assertTrue(strpos($pretty, "\n") !== false);
    }

    public function testInvalidJsonThrows()
    {
        $reader = new GEOSGeoJSONReader();
        try {
            $reader->read('not valid json at all');
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertNotNull($e);
        }
    }

    /* GEOS 3.14 GeoJSONReader is permissive — it extracts a geometry from
     * a Feature or wraps a FeatureCollection's geometries in a
     * GeometryCollection rather than throwing. We pin that behaviour here
     * so callers can rely on it. (Strict GeoJSON parsers would reject
     * FeatureCollection in a "geometry" reader; GEOS does not.) */
    public function testFeatureCollectionReturnsGeometryCollection()
    {
        $reader = new GEOSGeoJSONReader();
        $r = $reader->read(
            '{"type":"FeatureCollection","features":['
            . '{"type":"Feature","geometry":{"type":"Point","coordinates":[1,2]},"properties":{}},'
            . '{"type":"Feature","geometry":{"type":"Point","coordinates":[3,4]},"properties":{}}'
            . ']}'
        );
        $this->assertEquals('GeometryCollection', $r->typeName());
        $this->assertEquals(2, $r->numGeometries());
    }

    public function testFeatureReturnsInnerGeometry()
    {
        $reader = new GEOSGeoJSONReader();
        $r = $reader->read(
            '{"type":"Feature","geometry":{"type":"Point","coordinates":[5,6]},"properties":{}}'
        );
        $this->assertEquals('Point', $r->typeName());
    }
}

GeoJSONTest::run();

?>
--EXPECT--
GeoJSONTest->testReadPoint	OK
GeoJSONTest->testRoundTripPoint	OK
GeoJSONTest->testRoundTripLineString	OK
GeoJSONTest->testRoundTripPolygon	OK
GeoJSONTest->test3DPointDimensionToggle	OK
GeoJSONTest->testIndentToggle	OK
GeoJSONTest->testInvalidJsonThrows	OK
GeoJSONTest->testFeatureCollectionReturnsGeometryCollection	OK
GeoJSONTest->testFeatureReturnsInnerGeometry	OK
