#include <iostream>
#include "autograph.hpp"

int main(int argc, char **argv) {

  // Create first generation based on options
  Generation G(argc, argv);

  G.evolve(10, 127);

  G.write_dot();

  // Print bipartate graph
  // G.specimen[0].write_dot("output.dot");
}
