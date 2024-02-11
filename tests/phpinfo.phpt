--TEST--
phpinfo
--EXTENSIONS--
defer
--FILE--
<?php
phpinfo(INFO_MODULES);
--EXPECTF--
%A
defer
%A
Version => %A
Released => %A
Authors => %A
%A
