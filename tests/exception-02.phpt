--TEST--
exception 02
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
function f() {
    var_dump(1);
    defer(function () {
        var_dump(3);
        throw new \Exception();
    });
    defer(function () {
        var_dump(2);
        throw new \Exception();
    });
    var_dump(4);
}
try {
    f();
} catch (\Exception $e) {
    echo get_class($e), PHP_EOL;
    echo get_class($e->getPrevious()), PHP_EOL;
}
--EXPECT--
int(1)
int(4)
int(2)
int(3)
Exception
Exception
