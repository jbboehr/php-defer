# Copyright (c) anno Domini nostri Jesu Christi MMXXIV John Boehr & contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
