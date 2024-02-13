--TEST--
basic 03
--EXTENSIONS--
defer
--FILE--
<?php
use function DeferExt\defer;
function f() {
    $a = 1;
    defer(function() use ($a) { var_dump($a); });
    $a = 2;
}
f();
--EXPECT--
int(1)