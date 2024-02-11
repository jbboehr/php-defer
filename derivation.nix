{
  lib,
  php,
  stdenv,
  autoreconfHook,
  buildPecl,
  src,
  vyrtue,
}:
buildPecl rec {
  pname = "defer";
  name = "defer-${version}";
  version = "v0.1.0";

  inherit src;

  internalDeps = [vyrtue];
  buildInputs = [vyrtue];

  passthru.php = php;
  passthru.vyrtue = vyrtue;

  makeFlags = ["phpincludedir=$(dev)/include"];
  outputs = ["out" "dev"];

  #TEST_PHP_DETAILED = 1;
  NO_INTERACTION = 1;
  REPORT_EXIT_STATUS = 1;
  TEST_PHP_ARGS = "-c ${php.phpIni}";
}
