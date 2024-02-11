--TEST--
version
--EXTENSIONS--
defer
--FILE--
<?php
var_dump(DeferExt\VERSION);
--EXPECTF--
string(%d) "%d.%d.%d"