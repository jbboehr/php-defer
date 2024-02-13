--TEST--
exception 03
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
function f() {
    var_dump(1);
    throw new Exception();
    var_dump(4);
}
try {
    f();
} catch (\Exception $e) {
    echo get_class($e), PHP_EOL;
}
--EXPECT--
int(1)
Exception
