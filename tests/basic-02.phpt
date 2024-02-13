--TEST--
basic 02
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
function f() {
    var_dump(1);
    defer(fn() => var_dump(2));
    defer(fn() => var_dump(3));
    var_dump(4);
}
f();
--EXPECT--
int(1)
int(4)
int(3)
int(2)