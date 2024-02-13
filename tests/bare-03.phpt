--TEST--
bare 02
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
$filename = null;
function f() {
    $filename = tempnam(sys_get_temp_dir(), "FOO");
    $GLOBALS['filename'] = $filename;
    var_dump(file_exists($filename));
    defer(unlink($filename));
    // do other things
}
f();
var_dump(file_exists($filename));
--EXPECT--
bool(true)
bool(false)
