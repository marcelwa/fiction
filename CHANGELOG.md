# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## v0.3.2 - 2021-01-06
*Sometimes fiction is more easily understood than true events.* &mdash; Young-ha Kim

### Added
- Command `onepass` for a combined SAT-based logic synthesis and physical design using [Mugen](https://github.com/whaaswijk/mugen). Thanks to Winston Haaswijk for cooperating with me on this project!
- SVG output for irregular (cell-based) clocked `fcn_cell_layout`s (thanks to Gregor Kuhn!)
- `csv_writer` for conveniently formatting experiments' results
- `tt_reader` for reading truth tables from a [file format used by Alan Mishchenko](https://people.eecs.berkeley.edu/~alanmi/temp5/)

### Changed
- `exact --asynchronous/-a` has been renamed to `exact --async/-a` and `exact --asynchronous_max/-A` has been renamed to `exact --async_max`
- outsourced Verilog and AIGER file handling into a distinct `network_reader` class so that it can be used in custom experiments

### Fixed
- `Docker` build that broke down due to updates to `mockturtle` and `bill`

## v0.3.1 - 2020-06-04
*There is no doubt fiction makes a better job of the truth.* &mdash; Doris Lessing

### Added
- Command `equiv` for logical and delay equivalence checking of `fcn_gate_layout`s against a specification
- Command `energy` to print and log energy dissipation of current `fcn_gate_layout` based on a physical model for the QCA-ONE library
- Command `area` to print and log area usage in nm²
- Parameter `-a` and flag `-A` to enable asynchronous parallelism for `exact`
- Flag `--minimize_wires/-w` for `exact` to compute the minimum amount of wire segments needed
- Flag `-s` for `show -n` for less detailed visualization of `logic_network` objects
- `Dockerfile` and instructions for how to create an image using [Docker](https://www.docker.com/) (thanks to Mario Kneidinger!)
- CMake option to toggle animated progress bars on command line

### Changed
- `exact` has been completely reworked to utilize true incremental SMT solving without push/pop mechanics (thanks to Alan Mishchenko for the inspiration!)
- `exact --artificial_latches/-a` has been renamed to `exact --clock_latches/-l`
- `exact -m` has been renamed to `exact -c`
- Standard resolves for clocking scheme names to their commonly used variants, e.g., `2DDWave` becomes `2DDWave4`
- Energy dissipation will no longer be logged using command `ps -g`; use new command `energy` instead
- Command `cell` can be found in command class `Technology` now
- Increased font size of clock numbers in SVG files by 2pt for better readability
- Changed constructor parameter types for core data structures (network and layouts)
- Changed `std::size_t` to fixed-size data types in lots of places
- Use library caching for *Travis* builds to speed up build time
- Moved to the latest releases of all libraries

### Fixed
- Python detection in CMake under different versions
- Runtime logging in `exact`
- Performance issues in `ortho`
- SEGFAULTS caused by `ortho` on large networks when compiling with gcc
- `ortho -b` losing bent wire connections
- `fcn_layout::random_face`'s index to coordinate mapping again, but for real now (thanks to Till Schlechtweg!)
- `logic_network`s are deep-copied for each physical design call now to secure them from external changes
- Gates and wires without directions assigned are mapped to standard rotations using QCA-ONE library now
- Rotation issues with border gate-pin I/Os using QCA-ONE library
- 3-output fan-outs are correctly printed as fan-outs when using `print -g` now
- Testing `ofstream`'s for `is_open` in writers now (thanks to DeepCode!)
- Several compiler issues under MacOS and Windows (thanks to Umberto Garlando and Fabrizio Riente for pointing them out!)
- Z3 build script error under Unix with CMake version <= 3.12.0
- Z3 linking on MacOS (thanks to Daniel Staack!)
- bibTeX citation information correctly handles last names and arXiv prefixes now

### Removed
- `exact --limit_crossings/-c` and `exact --limit_wires/-w` as they have been replaced by respective optimization flags
- Legends in `print -g/-c`

## v0.3.0 - 2019-11-22
*Sometimes, fiction was so powerful that it even had reverberations in the real world.* &mdash; Delphine de Vigan

### Added
- Support for iNML technology using [ToPoliNano](https://topolinano.polito.it/)'s gate library and clocking scheme. Thanks to Umberto Garlando for cooperating with me on this project!
- Support for vertically shifted `fcn_layout`s to emulate column-based clocking schemes
- Enhanced `logic_network` by incorporating [mockturtle](https://github.com/lsils/mockturtle) for logic representation
- Truth table store (mnemonic `-t`) and command `tt`. Thanks to Mathias Soeken for granting permission to use code from [CirKit](https://github.com/msoeken/cirkit)!
- Command `simulate` to compute `truth_table`s for `logic_network` and `fcn_gate_layout` objects. Thanks to Mathias Soeken for granting permission to use code from [CirKit](https://github.com/msoeken/cirkit)!
- Command `akers` to perform Akers' Majority synthesis to generate a `logic_network` from a `truth_table`
- Command `random` to generate random `logic_network` objects
- Command `check` to verify structural integrity of designed `fcn_gate_layout` objects
- Command `gates` to list gate counts for each vertex type in the current `logic_network`
- Command `fanouts` to substitute high-degree inputs into fan-out vertices in `logic_network`s using a given strategy
- Command `balance` to subdivide `logic_network` edges to equalize path lengths by inserting auxiliary wire vertices
- Command `qcc` to write `iNML` `cell_layout`s to component files readable by [ToPoliNano and MagCAD](https://topolinano.polito.it/) 
- Capability to enforce straight inverter gates in `exact` with flag `-n`
- Capability to minimize the number of used crossing tiles in `exact` with flag `-m`
- Capability to parse AIGER (`*.aig`) files using `read`
- Parameter `-b` for `ortho`
- Progress bars for `exact` and `ortho`
- `show -n` to display `logic_network` objects
- Several convenience functions in the core data structures for easier access
- An overview [paper](./bib/paper.pdf) and a [poster](./bib/poster.pdf) about the features of *fiction*. Please find citation information in the [README](./README.md#references)

### Changed
- Moved to C++17
- Moved to version 1.0 of [cppitertools](https://github.com/ryanhaining/cppitertools)
- Included latest updates for [alice](https://github.com/msoeken/alice)
- Switched `logic_network`'s CLI mnemonic to `-n` as it is no longer reserved by `alice`
- Renamed `pi`/`po_count` to `num_pis`/`pos`
- `read` does no longer substitute fan-outs automatically, `exact` and `ortho` do it instead if the user did not call `fanouts`
- `exact --path_discrepancy/-p` has been renamed to `exact --desynchronize/-d` to express its use case better
- `exact --timeout/-t` expects its parameter in seconds instead of milliseconds now
- `exact --fixed_size/-f` expects its own parameter instead of using `--upper_bound`'s one
- Renamed `version.h` to `version_info.h` 
- Renamed *Placement & Routing* to *Physical Design* where appropriate to match the documentation

### Fixed
- Segfault when using `ortho -i` with certain compilers in release mode
- Missing input ports for 3-output fan-out gates in QCA-ONE library
- Wire vertices not handled properly by QCA-ONE library
- Wrong clocking look-up for `BANCS` clocking in `fcn_cell_layout`
- Tile directions when assigning and dissociating multiple edges
- `fcn_layout::random_face`'s index to coordinate mapping (thanks to Till Schlechtweg!)
- Format issues with benchmark files
- Constant outputs of some benchmark files
- Additionally, there are several performance improvements in core data structures and algorithms

### Removed
- Submodule `lorina` as it is included in `mockturtle`
- `verilog_parser.h` as `mockturtle` comes with its own one
- `print -n` as it is replaced by `show -n`
- `operation::BUF`; use `operation::W` instead
- `operation::CONST0`, `operation::CONST1`, and `operation::XOR`
- `operator[x][y][z]` for `fcn_layout`s as it was slow and therefore not used; use `face/tile/cell{x,y,z}` instead

## v0.2.1 - 2019-05-02
*Fiction is art and art is the triumph over chaos.* &mdash; John Cheever

### Added
- Support for BANCS clocking and integration in `exact`
- Name strings for `fcn_clocking_scheme` objects and corresponding name-based look-up
- Version and build information accessible within the code by including `util/version.h`
- Parameter `-i` for command `ortho`
- `shortcuts.fs` with predefined flows
- `benchmarks/MAJ/` folder with some TOY benchmarks using MAJ gates

### Changed
- Calls to `exact -s` now need to name the desired clocking, e.g. `exact -s use` (case insensitive)
- `incoming`/`outgoing_information_flow_tiles` have been renamed to `incoming`/`outgoing_data_flow` and handle multi wires now
- Renamed diagonal clocking schemes to 2DDWAVE and gave proper credit
- More verbose error messages

### Fixed
- TP calculation for layouts without designated I/O pins (thanks to Mario Kneidinger!)
- I/O port orientation of PI/PO gates using QCA-ONE library
- Usage of non-PI/PO MAJ gates in QCA-ONE library
- Visualization of clock latches in `show -c` (thanks to Gregor Kuhn!)
- Multi direction assignment to wires and gates in `exact` leading to physically impossible layouts
- `shrink_to_fit` in `fcn_gate_layout` incorporates the BGL bug now. Minimum size in each dimension is 2.
For more information, see <https://svn.boost.org/trac10/ticket/11735>
- Parameters for `exact` no longer get stuck once set

### Removed
- Parameter `-n` for `exact`

## v0.2.0 - 2019-03-21
*Fiction reveals the truth that reality obscures.* &mdash; Jessamyn West

### Added
- Export `fcn_cell_layout` objects as SVG using `show -c`. See README for more information
- `ps -g` displays and logs critical path and throughput of `fcn_gate_layout` objects
- Support for RES clocking and integration in `exact` 
- New TOY benchmarks in respective folder
- New command `clear` to remove all elements from stores (as a shorthand for `store --clear ...`)
- Information about how to build fiction for WSL
- Functions to distinguish different `logic_network` types like AIGs/MIGs/...
- Parameter `-n` for command `ortho`

### Changed
- Revised folder structure due to the increasing amount of source files
- `read_verilog` is now called `read` and can process directories
- `gate_to_cell` is now called `cell`
- `write_qca` is now called `qca` and handles file names automatically if necessary
- `-u` is not a required parameter for `exact` anymore
- Richer output for `print -w`
- Included latest bugfixes for [alice](https://github.com/msoeken/alice)
- Included latest update for [lorina](https://github.com/hriener/lorina)

### Fixed
- Starting layout size for calls to `exact -i` was too low and has been corrected
- Several code and comment inconsistencies

### Removed
- ITC99 benchmark files


## v0.1.1 - 2018-12-29
*Literature is a luxury; fiction is a necessity.* &mdash; G. K. Chesterton

### Added
- Technology-specific energy model for `fcn_gate_layout`; supports QCA thus far
- Support for `print -c` to write a textual representation of `fcn_cell_layout` objects
- Information on nested fiction scripts and documentation generation in README
- *linguist* flags in `.gitattributes` to prevent benchmark files from being viewed as source code

### Changed
- Moved to version 0.4 of [alice](https://github.com/msoeken/alice)
- Moved to version 4.8.4 of [Z3](https://github.com/Z3Prover/z3)
- `fcn_gate_library` objects now have name strings
- `print -g` now displays incorrectly assigned directions by bidirectional arrows
- "Release" is the standard build mode now

### Fixed
- Copy and move constructors of `logic_network` work properly now
- Calculation of `bounding_box` size on `fcn_gate_layout` now handles empty layouts correctly
- Several minor and rare bugs, code inconsistencies, and performance issues

### Removed
- Nothing

## v0.1.0 - 2018-10-29
*Let there be a fiction*

This is the initial release. Please find a feature overview in the [README](README.md).
