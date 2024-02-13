--TEST--
exception 01
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
    defer(fn() => var_dump(2));
    var_dump(4);
}
try {
    f();
} catch (\Exception $e) {
    echo get_class($e);
}
--EXPECT--
int(1)
int(4)
int(2)
int(3)
Exception