# autograph

Autograph takes your bipartate graph (in the form of an adjacency matrix), and attempts to planarize it in a manner that outlines patterns.

## Dependencies (macOS)

### Installing Homebrew

Install [homebrew](brew.sh) by running the following command:

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

### Installing dependencies through Homebrew

Install graphviz, llvm, pkg-config, cgal, and cmake by running the following command:

    brew graphviz install llvm@13 pkg-config cgal cmake

## Installation

Go to the `autograph` directory in your preferred terminal and execute the following commands.

    cmake -B build
    make -C build
    sudo make install -C build

## Usage

Run the following command (replace `your-csv-file.csv` with your csv file which contains graph data as an adjacency matrix):

    autograph your-csv-file.csv -s 100 -g 1000 -p 50 -o 100

You may use `autodot.sh` to convert your graphviz dot files into images.

### Flags

`-s`: Integer: number of specimen per generation

`-g`: Integer: number of generations

`-p`: Integer between 0 and 100: probability of a given node in the graph mutating

`-o`: Integer, n: output best specimen after every n generations

_Note: use the `-h` flag to display these explanations at any time._
