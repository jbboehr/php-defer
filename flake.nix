{
  description = "php-defer";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11";
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
    gitignore = {
      url = "github:hercules-ci/gitignore.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    pre-commit-hooks = {
      url = "github:cachix/pre-commit-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.nixpkgs-stable.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
      inputs.gitignore.follows = "gitignore";
    };
    ext-vyrtue = {
      url = "github:jbboehr/php-vyrtue";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    gitignore,
    pre-commit-hooks,
    ext-vyrtue,
  } @ args:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;

        src' = gitignore.lib.gitignoreSource ./.;

        src = pkgs.lib.cleanSourceWith {
          name = "php-defer-source";
          src = src';
          filter = gitignore.lib.gitignoreFilterWith {
            basePath = ./.;
            extraRules = ''
              .clang-format
              composer.json
              composer.lock
              .editorconfig
              .envrc
              .gitattributes
              .github
              .gitignore
              *.md
              *.nix
              flake.*
            '';
          };
        };

        makePhp = {
          php,
          vyrtue,
        }:
          php.buildEnv {
            extensions = {
              enabled,
              all,
            }:
              enabled ++ [all.ast all.opcache vyrtue];
          };

        makePackage = {
          php,
          vyrtue,
        }:
          pkgs.callPackage ./derivation.nix {
            php = makePhp {
              inherit php vyrtue;
            };
            inherit (php) buildPecl;
            inherit src vyrtue;
          };

        makeCheck = package:
          package.overrideAttrs (old: {
            doCheck = true;
          });

        pre-commit-check = pre-commit-hooks.lib.${system}.run {
          src = src';
          hooks = {
            actionlint.enable = true;
            alejandra.enable = true;
            alejandra.excludes = ["\/vendor\/"];
            clang-format.enable = true;
            clang-format.types_or = ["c" "c++"];
            clang-format.files = "\\.(c|h)$";
            markdownlint.enable = true;
            markdownlint.excludes = ["LICENSE\.md"];
            shellcheck.enable = true;
          };
        };

        mkDevShell = package:
          pkgs.mkShell {
            inputsFrom = [package];
            buildInputs = with pkgs; [
              actionlint
              clang-tools
              lcov
              gdb
              package.php
              package.php.packages.composer
              valgrind
            ];
            shellHook = ''
              ${pre-commit-check.shellHook}
              #ln -sf ${package.php.unwrapped.dev}/include/php/ .direnv/php-include
              export REPORT_EXIT_STATUS=1
              export NO_INTERACTION=1
              export PATH="$PWD/vendor/bin:$PATH"
              # opcache isn't getting loaded for tests because tests are run with '-n' and nixos doesn't compile
              # in opcache and relies on mkWrapper to load extensions
              export TEST_PHP_ARGS='-c ${package.php.phpIni}'
            '';
          };
      in rec {
        packages = rec {
          php81 = makePackage {
            php = pkgs.php81;
            vyrtue = ext-vyrtue.packages.${system}.php81;
          };
          php82 = makePackage {
            php = pkgs.php82;
            vyrtue = ext-vyrtue.packages.${system}.php82;
          };
          php83 = makePackage {
            php = pkgs.php83;
            vyrtue = ext-vyrtue.packages.${system}.php83;
          };
          default = php81;
        };

        devShells = rec {
          php81 = mkDevShell packages.php81;
          php82 = mkDevShell packages.php82;
          php83 = mkDevShell packages.php83;
          default = php81;
        };

        checks = {
          inherit pre-commit-check;
          php81 = makeCheck packages.php81;
          php82 = makeCheck packages.php82;
          php83 = makeCheck packages.php83;
        };

        formatter = pkgs.alejandra;
      }
    );
}
