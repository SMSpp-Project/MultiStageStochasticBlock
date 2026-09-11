# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

### Changed

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
