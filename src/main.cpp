#include <iostream>
#include "autograph.hpp"

int main(int argc, char **argv) {

  // Create generation from input CSV
  Generation G(argc, argv);

  // Print bipartate graph
  G.specimen[0].print_dot("output.dot");
}
