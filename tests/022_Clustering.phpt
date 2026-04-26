--TEST--
GEOSGeometry clustering family + GEOSClusterResult (Item 14)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class ClusteringTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    /* Six points in two distinct groups. */
    private function pointGroups()
    {
        return $this->reader()->read(
            'MULTIPOINT('
            . '(0 0),(0 0.1),(0.1 0),'      /* tight cluster A */
            . '(10 10),(10 10.1),(10.1 10)' /* tight cluster B */
            . ')');
    }

    public function testClusterResultClassExists()
    {
        $this->assertTrue(class_exists('GEOSClusterResult'));
    }

    public function testDirectConstructionThrows()
    {
        try {
            $r = new GEOSClusterResult();
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('cannot be constructed', $e->getMessage());
        }
    }

    public function testClusterDBSCAN()
    {
        $g = $this->pointGroups();
        $r = $g->clusterDBSCAN(1.0, 2);
        $this->assertNotNull($r);
        $this->assertTrue($r instanceof GEOSClusterResult);
        /* Two clusters expected. */
        $this->assertEquals(2, $r->getNumClusters());
        /* Each cluster has 3 inputs. */
        $this->assertEquals(3, $r->getClusterSize(0));
        $this->assertEquals(3, $r->getClusterSize(1));
    }

    public function testClusterDBSCAN_unclusteredOutlier()
    {
        $g = $this->reader()->read(
            'MULTIPOINT((0 0),(0 0.1),(0.1 0),(100 100))');
        $r = $g->clusterDBSCAN(1.0, 2);
        $this->assertEquals(1, $r->getNumClusters());
        $forInputs = $r->getClustersForInputs();
        $this->assertEquals(4, count($forInputs));
        /* Outlier at index 3 is unclustered (-1) */
        $this->assertEquals(-1, $forInputs[3]);
    }

    public function testGetClustersForInputs()
    {
        $g = $this->pointGroups();
        $r = $g->clusterDBSCAN(1.0, 2);
        $forInputs = $r->getClustersForInputs();
        $this->assertTrue(is_array($forInputs));
        /* One entry per input geometry (6). */
        $this->assertEquals(6, count($forInputs));
        /* All 6 inputs are clustered (no -1). */
        foreach ($forInputs as $cidx) {
            $this->assertTrue($cidx === 0 || $cidx === 1);
        }
    }

    public function testGetInputsForCluster()
    {
        $g = $this->pointGroups();
        $r = $g->clusterDBSCAN(1.0, 2);
        $inputs0 = $r->getInputsForCluster(0);
        $inputs1 = $r->getInputsForCluster(1);
        $this->assertTrue(is_array($inputs0));
        $this->assertTrue(is_array($inputs1));
        $this->assertEquals(3, count($inputs0));
        $this->assertEquals(3, count($inputs1));
        /* The two clusters together should cover indices 0..5 exactly. */
        $all = array_merge($inputs0, $inputs1);
        sort($all);
        $this->assertEquals(array(0, 1, 2, 3, 4, 5), $all);
    }

    public function testGetInputsForClusterOutOfRange()
    {
        $g = $this->pointGroups();
        $r = $g->clusterDBSCAN(1.0, 2);
        try {
            $r->getInputsForCluster(99);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('out of range', $e->getMessage());
        }
    }

    public function testClusterByDistance_geometry()
    {
        $g = $this->pointGroups();
        $r = $g->clusterByDistance(1.0, false);
        $this->assertNotNull($r);
        $this->assertEquals(2, $r->getNumClusters());
    }

    public function testClusterByDistance_envelope()
    {
        $g = $this->pointGroups();
        $r = $g->clusterByDistance(1.0, true);
        $this->assertNotNull($r);
        $this->assertEquals(2, $r->getNumClusters());
    }

    public function testClusterByIntersection_geometry()
    {
        /* Three overlapping squares + one disjoint square => 2 clusters. */
        $g = $this->reader()->read(
            'GEOMETRYCOLLECTION('
            . 'POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)),'
            . 'POLYGON((0.5 0, 1.5 0, 1.5 1, 0.5 1, 0.5 0)),'
            . 'POLYGON((1.2 0, 2.2 0, 2.2 1, 1.2 1, 1.2 0)),'
            . 'POLYGON((10 10, 11 10, 11 11, 10 11, 10 10))'
            . ')');
        $r = $g->clusterByIntersection(false);
        $this->assertNotNull($r);
        /* 3 overlap into 1 cluster, 1 alone -> 2 clusters */
        $this->assertEquals(2, $r->getNumClusters());
    }

    public function testClusterByIntersection_envelope()
    {
        $g = $this->reader()->read(
            'GEOMETRYCOLLECTION('
            . 'POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)),'
            . 'POLYGON((0.5 0, 1.5 0, 1.5 1, 0.5 1, 0.5 0)),'
            . 'POLYGON((10 10, 11 10, 11 11, 10 11, 10 10))'
            . ')');
        $r = $g->clusterByIntersection(true);
        $this->assertNotNull($r);
        $this->assertEquals(2, $r->getNumClusters());
    }
}

ClusteringTest::run();

?>
--EXPECT--
ClusteringTest->testClusterResultClassExists	OK
ClusteringTest->testDirectConstructionThrows	OK
ClusteringTest->testClusterDBSCAN	OK
ClusteringTest->testClusterDBSCAN_unclusteredOutlier	OK
ClusteringTest->testGetClustersForInputs	OK
ClusteringTest->testGetInputsForCluster	OK
ClusteringTest->testGetInputsForClusterOutOfRange	OK
ClusteringTest->testClusterByDistance_geometry	OK
ClusteringTest->testClusterByDistance_envelope	OK
ClusteringTest->testClusterByIntersection_geometry	OK
ClusteringTest->testClusterByIntersection_envelope	OK
