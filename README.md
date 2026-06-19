# MultiStageStochasticBlock

SMS++ Block for multi-stage stochastic programming problems.

The `MultiStageStochasticBlock` is a `:Block` that represents a multi-stage
stochastic programming problem as an aggregation of `TwoStageStochasticBlock`,
in the same way a multi-stage scenario tree is obtained by recursively nesting
two-stage subtrees. It builds the explicit (extensive) form of the problem by:

- holding one `TwoStageStochasticBlock` per outer-stage scenario as its inner
  sub-Blocks, each of which represents the two-stage subtree rooted at that
  scenario and already enforces non-anticipativity over its own here-and-now
  variables,

- adding an outer layer of non-anticipativity constraints that ties the
  first-stage (root) here-and-now variables across all the inner
  `TwoStageStochasticBlock`,

- combining the objectives of the inner `TwoStageStochasticBlock` weighted by
  the outer-stage scenario probabilities, so that, composed with the inner
  probabilities, every leaf scenario ends up weighted by its joint
  probability.

For a three-stage problem the first stage holds the common here-and-now
decisions, the second stage holds one `TwoStageStochasticBlock` per outer
scenario, and the third stage holds the recourse decisions in the leaves of
each inner two-stage subtree. Deeper trees are obtained by letting the inner
sub-Blocks be themselves `MultiStageStochasticBlock`.

The aggregation reuses the machinery of `TwoStageStochasticBlock`, including
the recursive `TwoStageStochasticBlockSolution`, so that the whole tree is
serialized, solved and read back without any of the inner Blocks needing to be
aware of the multi-stage structure. As with `TwoStageStochasticBlock`, the
`AbstractPath` to the first-stage variables locates them inside the inner
Blocks, and scenario data is supplied through `DataMapping` and
`ScenarioGenerator`.


## Getting started

These instructions will let you build MultiStageStochasticBlock on your system.

### Requirements

- [SMS++ TwoStageStochasticBlock](https://gitlab.com/smspp/twostagestochasticblock),
  which in turn requires the SMS++ StochasticBlock and the "core SMS++".

### Build and install with CMake

Configure and build the library with:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

The library has the same configuration options of
[SMS++](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration).

Optionally, install the library in the system with:

```sh
cmake --install .
```

### Usage with CMake

After the library is built, you can use it in your CMake project with:

```cmake
find_package(MultiStageStochasticBlock)
target_link_libraries(<my_target> SMS++::MultiStageStochasticBlock)
```

### Build and install with makefiles

Carefully hand-crafted makefiles have also been developed for those unwilling
to use CMake. Makefiles build the executable in-source (in the same directory
tree where the code is) as opposed to out-of-source (in the copy of the
directory tree constructed in the build/ folder) and therefore it is more
convenient when having to recompile often, such as when developing/debugging
a new module, as opposed to the compile-and-forget usage envisioned by CMake.

Each executable using `MultiStageStochasticBlock` has to include a "main
makefile" of the module, which typically is either [makefile-c](makefile-c)
including all necessary libraries comprised the "core SMS++" one, or
[makefile-s](makefile-s) including all necessary libraries but not the "core
SMS++" one (for the common case in which this is used together with other
modules that already include them). The makefiles in turn recursively include
all the required other makefiles, hence one should only need to edit the "main
makefile" for compilation type (C++ compiler and its options) and it all
should be good to go. In case some of the external libraries are not at their
default location, it should only be necessary to create the
`../extlib/makefile-paths` out of the `extlib/makefile-default-paths-*` for
your OS `*` and edit the relevant bits (commenting out all the rest).

Check the [SMS++ installation wiki](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration#location-of-required-libraries)
for further details.

## Tests

The [test](test) folder contains a tester for `MultiStageStochasticBlock`,
which loads an instance from a netCDF file, attaches one or two `:Solver`
through a `BlockSolverConfig` and compares their results.

## Getting help

If you need support, you want to submit bugs or propose a new feature, you can
[open a new issue](https://gitlab.com/smspp/multistagestochasticblock/-/issues/new).

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of
conduct, and the process for submitting merge requests to us.

## Authors

### Current Lead Authors

- **Donato Meoli**  
  Dipartimento di Informatica  
  Università di Pisa

### Contributors

- **Antonio Frangioni**  
  Dipartimento di Informatica  
  Università di Pisa

## License

This code is provided free of charge under the [GNU Lesser General Public
License version 3.0](https://opensource.org/licenses/lgpl-3.0.html) -
see the [LICENSE](LICENSE) file for details.

## Disclaimer

The code is currently provided free of charge under an open-source license.
As such, it is provided "*as is*", without any explicit or implicit warranty
that it will properly behave or it will suit your needs. The Authors of
the code cannot be considered liable, either directly or indirectly, for
any damage or loss that anybody could suffer for having used it. More
details about the non-warranty attached to this code are available in the
license description file.
