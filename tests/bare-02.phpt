--TEST--
bare 02
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
function f() {
    $a = 1;
    defer(var_dump($a));
    $a = 2;
}
f();
--EXPECT--
int(1)
