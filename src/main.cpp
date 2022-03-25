#include <iostream>
#include "autograph.hpp"

int main(int argc, char **argv) {

  // Create first generation based on options
  Generation G(argc, argv);
  G.advance_n_gens();
}
