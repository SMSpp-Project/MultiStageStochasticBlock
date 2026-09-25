# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

### Changed

- the makefile asks for `-O3 -DNDEBUG` and nothing else, the macro of the
  patch for `boost::any` on macOS having no reason to be there since there is
  no `boost::any` left in the core

- whoever links the module keeps it: the classes of a module register
  themselves in the factory from a static initialiser, and a linker that
  drops what looks unused takes the registration away with it, so the target
  now tells whoever links it to keep the symbol that forces the module in,
  and on ELF, where naming the symbol is not enough, the library as a whole

### Fixed

- the test of this module enters the build and the pipeline runs it: its
  directory was never added, `BUILD_TESTING` being consumed by a comment, so
  the check of the direct construction and of the registration in the factory
  was built by nobody, and it now carries the label of the module, which is
  what `ctest -L <module>` selects

### Fixed

## [0.1.0] - 2026-09-12

### Added

- `MultiStageStochasticBlock`, a multi-stage stochastic program as an
  aggregation of TwoStageStochasticBlock, one per scenario of the outer stage,
  tied by an outer layer of non-anticipativity constraints; the scenario tree
  is walked through the View of a MultiStageScenarioGenerator, each inner
  Block getting a View of its own, cloned from the one of the root and
  descended to its node

- the leaves of the whole tree are enumerated by flattening the subtrees, the
  sub-Block of a node being one representative of each

### Changed

- the version of the module is the git tag of its repository, or the
  VERSION.txt of a release tarball, and the shared library carries it: its
  SONAME is major.minor while the major is 0, and it is installed with an
  RPATH relative to itself, so that an installed tree keeps working wherever
  it is moved

[Unreleased]: https://gitlab.com/smspp/multistagestochasticblock/-/compare/0.1.0...develop
[0.1.0]: https://gitlab.com/smspp/multistagestochasticblock/-/tags/0.1.0
