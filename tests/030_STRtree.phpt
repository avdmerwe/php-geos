--TEST--
GEOSSTRtree spatial index (Item 17)
--SKIPIF--
<?php if (!extension_loaded('geos')) { print "geos extension not loaded\n"; exit(1); } ?>
--FILE--
<?php

require './tests/TestHelper.php';

class STRtreeTest extends GEOSTest
{
    private function reader() { return new GEOSWKTReader(); }

    /* Helper: a small AABB envelope geometry. */
    private function box($xmin, $ymin, $xmax, $ymax)
    {
        return $this->reader()->read(
            "POLYGON(($xmin $ymin, $xmax $ymin, "
            . "$xmax $ymax, $xmin $ymax, $xmin $ymin))");
    }

    public function testClassExists()
    {
        $this->assertTrue(class_exists('GEOSSTRtree'));
    }

    public function testConstructDefault()
    {
        $t = new GEOSSTRtree();
        $this->assertNotNull($t);
    }

    public function testConstructCustomCapacity()
    {
        $t = new GEOSSTRtree(16);
        $this->assertNotNull($t);
    }

    public function testConstructCapacityTooSmallThrows()
    {
        try {
            $t = new GEOSSTRtree(1);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('nodeCapacity', $e->getMessage());
        }
    }

    public function testInsertAndQueryCollect()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        $t->insert($this->box(2, 2, 3, 3), 2);
        $t->insert($this->box(10, 10, 11, 11), 3);

        /* Query with an envelope covering only the first two boxes. */
        $hits = $t->query($this->box(-1, -1, 4, 4));
        sort($hits);
        $this->assertEquals(array(1, 2), $hits);

        /* Query with an envelope covering all three boxes. */
        $all = $t->query($this->box(-5, -5, 20, 20));
        sort($all);
        $this->assertEquals(array(1, 2, 3), $all);
    }

    public function testQueryEmptyTreeReturnsArray()
    {
        $t = new GEOSSTRtree();
        $hits = $t->query($this->box(0, 0, 1, 1));
        $this->assertTrue(is_array($hits));
        $this->assertEquals(0, count($hits));
    }

    public function testQueryWithCallback()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), "a");
        $t->insert($this->box(2, 2, 3, 3), "b");
        $t->insert($this->box(10, 10, 11, 11), "c");

        $seen = array();
        $ret = $t->query($this->box(-1, -1, 4, 4),
            function($payload) use (&$seen) { $seen[] = $payload; });

        /* Callback mode returns an empty array. */
        $this->assertTrue(is_array($ret));
        $this->assertEquals(0, count($ret));

        sort($seen);
        $this->assertEquals(array("a", "b"), $seen);
    }

    public function testMixedTypePayloads()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 42);
        $t->insert($this->box(0, 0, 1, 1), "string-payload");
        $t->insert($this->box(0, 0, 1, 1), array("a", "b"));
        $obj = new stdClass();
        $obj->name = "thing";
        $t->insert($this->box(0, 0, 1, 1), $obj);

        $hits = $t->query($this->box(-1, -1, 2, 2));
        $this->assertEquals(4, count($hits));

        $foundInt = $foundString = $foundArray = $foundObj = false;
        foreach ($hits as $h) {
            if (is_int($h) && $h === 42) $foundInt = true;
            elseif (is_string($h) && $h === "string-payload") $foundString = true;
            elseif (is_array($h) && $h === array("a", "b")) $foundArray = true;
            elseif (is_object($h) && $h->name === "thing") $foundObj = true;
        }
        $this->assertTrue($foundInt);
        $this->assertTrue($foundString);
        $this->assertTrue($foundArray);
        $this->assertTrue($foundObj);
    }

    public function testIterate()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        $t->insert($this->box(2, 2, 3, 3), 2);
        $t->insert($this->box(10, 10, 11, 11), 3);

        $seen = array();
        $t->iterate(function($p) use (&$seen) { $seen[] = $p; });
        sort($seen);
        $this->assertEquals(array(1, 2, 3), $seen);
    }

    public function testNearest()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), "near");
        $t->insert($this->box(100, 100, 101, 101), "far");

        $probe = $this->reader()->read("POINT(0.5 0.5)");
        $payload = $t->nearest($probe);
        $this->assertEquals("near", $payload);

        $probe2 = $this->reader()->read("POINT(100.5 100.5)");
        $payload2 = $t->nearest($probe2);
        $this->assertEquals("far", $payload2);
    }

    public function testNearestEmptyTreeReturnsNull()
    {
        $t = new GEOSSTRtree();
        $probe = $this->reader()->read("POINT(0 0)");
        $r = $t->nearest($probe);
        $this->assertNull($r);
    }

    public function testRemoveByIntPayload()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        $t->insert($this->box(2, 2, 3, 3), 2);

        $ok = $t->remove($this->box(0, 0, 1, 1), 1);
        $this->assertTrue($ok);

        /* Second remove of same returns false. */
        $ok2 = $t->remove($this->box(0, 0, 1, 1), 1);
        $this->assertFalse($ok2);

        /* The other entry is still queryable. */
        $hits = $t->query($this->box(-5, -5, 5, 5));
        $this->assertEquals(array(2), $hits);
    }

    public function testRemoveByObjectPayload_identitySemantic()
    {
        $t = new GEOSSTRtree();
        $obj1 = new stdClass();
        $obj1->v = 1;
        $obj2 = new stdClass();
        $obj2->v = 2;
        $t->insert($this->box(0, 0, 1, 1), $obj1);
        $t->insert($this->box(0, 0, 1, 1), $obj2);

        /* A "look-alike" object (==-equal but different instance) MUST NOT
         * remove obj1 — we use object identity. */
        $look = new stdClass();
        $look->v = 1;
        $r1 = $t->remove($this->box(0, 0, 1, 1), $look);
        $this->assertFalse($r1);

        /* The actual obj1 instance removes correctly. */
        $r2 = $t->remove($this->box(0, 0, 1, 1), $obj1);
        $this->assertTrue($r2);

        /* Tree still has obj2. */
        $hits = $t->query($this->box(-1, -1, 2, 2));
        $this->assertEquals(1, count($hits));
        $this->assertTrue($hits[0] === $obj2);
    }

    public function testCallbackExceptionPropagates()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        $t->insert($this->box(0, 0, 1, 1), 2);
        $t->insert($this->box(0, 0, 1, 1), 3);

        $count = 0;
        try {
            $t->query($this->box(-1, -1, 2, 2),
                function($p) use (&$count) {
                    $count++;
                    throw new Exception("callback boom: $p");
                });
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('callback boom', $e->getMessage());
        }
        /* Sticky-error: only the first invocation should have run the body. */
        $this->assertEquals(1, $count);
    }

    public function testCallbackExceptionInIterate()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        $t->insert($this->box(0, 0, 1, 1), 2);

        try {
            $t->iterate(function($p) {
                throw new Exception("iter boom");
            });
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('iter boom', $e->getMessage());
        }
    }

    public function testInsertAfterQueryRejected()
    {
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), 1);
        /* First query builds the tree. */
        $t->query($this->box(0, 0, 1, 1));

        try {
            $t->insert($this->box(2, 2, 3, 3), 2);
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('build-once', $e->getMessage());
        }
    }

    public function testRemoveAfterNearestRejected()
    {
        /* Regression: GEOSSTRtree_remove_r against a built tree was
         * undefined behaviour and segfaulted in libgeos 3.14. The binding
         * now extends the build-once gate to remove() too. */
        $t = new GEOSSTRtree();
        $envA = $this->box(0, 0, 1, 1);
        $t->insert($envA, 'A');
        $t->insert($this->box(2, 2, 3, 3), 'B');
        $t->nearest($this->box(-1, -1, -1, -1));

        try {
            $t->remove($envA, 'A');
            $this->assertTrue(false);
        } catch (Exception $e) {
            $this->assertContains('build-once', $e->getMessage());
        }
    }

    public function testInsertEnvLifetimeAfterCallerDrops()
    {
        /* Regression: insert() previously stored the caller's GEOSGeometry*
         * pointer in the tree. Dropping the caller's $env before query
         * could leave a dangling reference. The binding now stores its own
         * cloned env (box->env), so this is safe. */
        $t = new GEOSSTRtree();

        $envA = $this->box(0, 0, 1, 1);
        $t->insert($envA, 'A');
        unset($envA);
        gc_collect_cycles();

        $envB = $this->box(2, 2, 3, 3);
        $t->insert($envB, 'B');
        unset($envB);
        gc_collect_cycles();

        $hits = $t->query($this->box(-5, -5, 5, 5));
        sort($hits);
        $this->assertEquals(array('A', 'B'), $hits);
    }

    public function testLifecycleObjectRefcount()
    {
        /* When the tree is destroyed, payload object refcounts must drop. */
        $obj = new stdClass();
        $obj->tag = "watched";

        /* Establish the baseline refcount of $obj first. */
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), $obj);
        $t->insert($this->box(0, 0, 1, 1), $obj); /* insert twice */

        $weak = WeakReference::create($obj);

        /* Free the only PHP-side strong refs by overwriting them. The tree
         * still holds two boxed copies. The object should NOT be collected
         * yet. */
        unset($obj);
        $this->assertNotNull($weak->get());

        /* Now drop the tree — this must free both boxes and let the object
         * go. */
        unset($t);
        $this->assertNull($weak->get());
    }

    public function testRemoveDecrementsPayloadRefcount()
    {
        $obj = new stdClass();
        $obj->tag = "x";
        $t = new GEOSSTRtree();
        $t->insert($this->box(0, 0, 1, 1), $obj);
        $weak = WeakReference::create($obj);
        $r = $t->remove($this->box(0, 0, 1, 1), $obj);
        $this->assertTrue($r);
        unset($obj);
        /* No more refs anywhere, object should be gone. */
        $this->assertNull($weak->get());
    }
}

STRtreeTest::run();

?>
--EXPECT--
STRtreeTest->testClassExists	OK
STRtreeTest->testConstructDefault	OK
STRtreeTest->testConstructCustomCapacity	OK
STRtreeTest->testConstructCapacityTooSmallThrows	OK
STRtreeTest->testInsertAndQueryCollect	OK
STRtreeTest->testQueryEmptyTreeReturnsArray	OK
STRtreeTest->testQueryWithCallback	OK
STRtreeTest->testMixedTypePayloads	OK
STRtreeTest->testIterate	OK
STRtreeTest->testNearest	OK
STRtreeTest->testNearestEmptyTreeReturnsNull	OK
STRtreeTest->testRemoveByIntPayload	OK
STRtreeTest->testRemoveByObjectPayload_identitySemantic	OK
STRtreeTest->testCallbackExceptionPropagates	OK
STRtreeTest->testCallbackExceptionInIterate	OK
STRtreeTest->testInsertAfterQueryRejected	OK
STRtreeTest->testRemoveAfterNearestRejected	OK
STRtreeTest->testInsertEnvLifetimeAfterCallerDrops	OK
STRtreeTest->testLifecycleObjectRefcount	OK
STRtreeTest->testRemoveDecrementsPayloadRefcount	OK
