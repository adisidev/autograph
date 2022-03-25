#pragma once

#include "argparse.hpp"
#include "random.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <boost/container_hash/hash.hpp>

#define t1_prefix "O_"
#define t2_prefix "R_"

// get base random alias which is auto seeded and has static API and internal state
using Random = effolkronium::random_static;

struct Pos {
  int x;
  int y;

  Pos() {
    x = 0; y = 0;
  }
  Pos(int x1, int y1) {
    x = x1; y = y1;
  }
};

struct Segment {
  Pos a; Pos b;
  Segment(Pos a1, Pos b1) {
    a = a1; b = b1;
  }
  bool intersects(Segment &s2);
};

bool Segment::intersects(Segment &s2) {
  return ((a.y < s2.a.y && b.y > s2.b.y) ||
          (a.y > s2.a.y && b.y < s2.b.y) ||
          (a.y == s2.a.y && b.y == s2.b.y));
}

struct Node {
  unsigned int id;
  Pos pos;
  // std::string label;
  std::unordered_set<int> connections;
  bool is_t1;

  Node() {
    is_t1 = true;
  };

  std::string as_dot() {
    std::string dot = "  ";
    if (is_t1) {
      dot += t1_prefix + std::to_string(id) + " [color = blue, ";
    } else {
      dot += t2_prefix + std::to_string(id) + " [color = red, ";
    }
    // dot += "label = " + label + ", "
    dot += "pos = \"" +
      std::to_string(pos.x) +
      "," +
      std::to_string(pos.y) +
      "!\"];\n";
    return dot;
  }

};

struct Edge {
  unsigned int from;
  unsigned int to;
  int weight;
  Edge(int f, int t, int w) {
    from = f; to = t; weight = w;
  }

  std::string as_dot() {
    std::string dot = "  ";
    dot += t1_prefix + std::to_string(from) + " -- " + t2_prefix + std::to_string(to);
    dot += "[label = \"" + std::to_string(weight) + "\"];\n";
    return dot;
  }
};

struct Bipartate {

  unsigned int score;
  std::unordered_map<int, Node> t1;
  std::unordered_map<int, Node> t2;
  std::vector<Edge> edges;
  std::unordered_map<std::pair<int, int>, int,
    boost::hash<std::pair<int, int>>> positions;
  Bipartate(std::string csv_name);
  Bipartate() {};
  void write_dot(std::string file_name);
  Bipartate mutate(uint8_t chance);
  void calc_score();

  // Overloading "<" operator based on score
  bool operator < (const Bipartate &rhs) const {
    return score < rhs.score;
  }
};

Bipartate::Bipartate(std::string csv_name) {

 // https://stackoverflow.com/questions/48994605/csv-data-into-a-2d-array-of-integers
  std::ifstream f;
  f.open(csv_name);

  // Number of lines read
  unsigned int n_row = 0;

  // Buffer to hold one row, one value
  std::string row, val;
  std::vector<std::vector<int>> array;

  // Read one row at a time
  while (std::getline (f, row)) {
    std::stringstream s(row);

    // Initialise t1 at n_row
    t1[n_row].id = n_row;

    // Column index
    unsigned int n_col = 0;

    // Read one , delimitted value at a time
    while (getline (s, val, ',')) {
      int weight = std::stoi(val);

      // Initialise t2 at n_col
      t2[n_col].id = n_col;

      // If linked
      if (weight) {

        // Add connection from t1 to t2
        t1[n_row].connections.insert(n_col);

        // Add connection from t2 to t1
        t2[n_col].connections.insert(n_row);

        // Add edge from t1 to t2
        edges.push_back(Edge(n_row, n_col, weight));
      }
      n_col++;
    }
    n_row++;
  }

  // First tower
  // "Random" order based on unordered_map
  int x = 0;
  int y = 0;
  for (auto& it: t1) {
    positions[{x, y}] = it.first;
    it.second.pos = Pos(x, y);
    y++;
  }

  // Second tower, next to first
  x++;
  y = 0;

  // "Random" order based on unordered_map
  for (auto& it: t2) {
    positions[{x, y}] = it.first;
    it.second.pos = Pos(x, y);
    it.second.is_t1 = false;
    y++;
  }
  write_dot("input.dot");
}

Bipartate Bipartate::mutate(uint8_t chance) {

  // Make a copy as the result we will return
  Bipartate bm(*this);

  for (auto& it: bm.t1) {

    // Should this node mutate?
    if (Random::get<uint8_t>(1, 100) > chance) {

      // Should it swap with a neighbour?
      if (Random::get<bool>()) {

        int x = it.second.pos.x; int y = it.second.pos.y;

        // Swap with neighbour above
        if (bm.positions.contains({x, y + 1})) {
          it.second.pos.y = y + 1;
          bm.t1[bm.positions[{x, y + 1}]].pos.y = y;
          bm.positions[{x, y}] = bm.t1[bm.positions[{x, y + 1}]].id;
          bm.positions[{x, y + 1}] = it.second.id;
        }

        // Swap with neighbour below
        else if (bm.positions.contains({x, y - 1})) {
          it.second.pos.y = y - 1;
          bm.t1[bm.positions[{x, y - 1}]].pos.y = y;
          bm.positions[{x, y}] = bm.t1[bm.positions[{x, y - 1}]].id;
          bm.positions[{x, y - 1}] = it.second.id;
        }
      }
      // Else move to new empty cell
    }
  }

  for (auto& it: bm.t2) {

    // Should this node mutate?
    if (Random::get<uint8_t>(1, 100) > chance) {

      // Should it swap with a neighbour?
      if (Random::get<bool>()) {

        int x = it.second.pos.x; int y = it.second.pos.y;

        // Swap with neighbour above
        if (bm.positions.contains({x, y + 1})) {
          it.second.pos.y = y + 1;
          bm.t2[bm.positions[{x, y + 1}]].pos.y = y;
          bm.positions[{x, y}] = bm.t2[bm.positions[{x, y + 1}]].id;
          bm.positions[{x, y + 1}] = it.second.id;
        }

        // Swap with neighbour below
        else if (bm.positions.contains({x, y - 1})) {
          it.second.pos.y = y - 1;
          bm.t2[bm.positions[{x, y - 1}]].pos.y = y;
          bm.positions[{x, y}] = bm.t2[bm.positions[{x, y - 1}]].id;
          bm.positions[{x, y - 1}] = it.second.id;
        }
      }
      // Else move to new empty cell
    }
  }
  bm.calc_score();
  return bm;
}

void Bipartate::write_dot(std::string file_name) {
  std::ofstream f;
  f.open(file_name);
  f << "graph autograph {\n";

  // Nodes
  f << std::endl;
  f << "  // Nodes\n";
  for (auto& it: t1) {
    f << it.second.as_dot();
  }
  for (auto& it: t2) {
    f << it.second.as_dot();
  }

  // Edges
  f << std::endl;
  f << "  // Edges\n";
  for (Edge e : edges) {
    f << e.as_dot();
  }

  f << "\n}\n";
  f.close();
  return;
}

// Calculate score to optimise
void Bipartate::calc_score() {

  score = 0;
  for (unsigned int i = 0; i < edges.size(); ++i) {
    Segment s1(t1[edges[i].from].pos, t2[edges[i].to].pos);
    for (unsigned int j = i + 1; j < edges.size(); ++j) {
      Segment s2(t1[edges[j].from].pos, t2[edges[j].to].pos);
      score += s1.intersects(s2);
    }
  }

}

struct Generation {

  // What generation we are on
  int n_generation;

  unsigned int default_probability;
  unsigned int default_n_gens;
  unsigned int default_n_specimen;
  unsigned int default_output;

  // All Graphs of this generation
  std::vector<Bipartate> specimen;

  // Scores of in this generation
  unsigned int worst_score;
  // double percentile_25; // 25th percentile
  unsigned int median_score; // 50th percentile
  // double percentile_75; // 75th percentile
  unsigned int best_score;

  // Name of CSV that this was initially created from
  std::string csv_name;

  Generation(int argc, char **argv);
  void evolve(unsigned int n_specimen, uint8_t chance);
  void write_dot(bool all);
  void advance(unsigned int n_specimen, uint8_t chance);
  void advance_n_gens(unsigned int n_gens,
                      unsigned int n_specimen,
                      uint8_t chance);
};

void Generation::advance(unsigned int n_specimen, uint8_t chance) {

  // std::cout << "Going from Generation " << n_generation;
  // std::cout << " to Generation " << ++n_generation << std::endl;
  evolve(n_specimen, chance);
}

void Generation::advance_n_gens(unsigned int n_gens = 0,
                                unsigned int n_specimen = 0,
                                uint8_t chance = 0) {

  if (!n_gens || !n_specimen || !chance) {
    n_gens = default_n_gens;
    n_specimen = default_n_specimen;
    chance = default_probability;
  }

  for (unsigned int i = 0; i <= n_gens; i++) {
    if (!(i % default_output)) {
      std::cout << "Best score for Generation " << n_generation << ": ";
      std::cout << specimen[0].score << std::endl;
      write_dot(false);
    }
    evolve(n_specimen, chance);
  }
}

Generation::Generation(int argc, char **argv) {

  n_generation = 0;

  // Create parser for arguments using argparse.
  // From https://github.com/p-ranav/argparse
  argparse::ArgumentParser arguments("autograph", "1.0");

  // Positional argument accepting CSV file
  arguments.add_argument("CSV")
    .default_value("none")
    .help("File path: CSV file");

  // Optional argument
  arguments.add_argument("-s", "--n_specimen")
    .default_value(static_cast<unsigned int>(1000))
    .scan<'u', unsigned int>()
    .help("Integer: Number of specimen per generation");

  // Optional argument
  arguments.add_argument("-g", "--n_generations")
    .default_value(static_cast<unsigned int>(1000))
    .scan<'u', unsigned int>()
    .help("Integer: Number of generations");

  // Optional argument
  arguments.add_argument("-p", "--probability")
    .default_value(static_cast<unsigned int>(50))
    .scan<'u', unsigned int>()
    .help("Integer between 0 and 100: Proportion of mutations");

  // Optional argument
  arguments.add_argument("-o", "--output")
    .default_value(static_cast<unsigned int>(100))
    .scan<'u', unsigned int>()
    .help("Integer: Output best every n generations");

  // Parse command line arguments
  try {
    arguments.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << "ERROR: " << err.what() << std::endl;
    std::cerr << arguments;
    std::exit(1);
  }

  // Print names of CSV provided
  if (arguments.is_used("CSV")) {
    csv_name = arguments.get<std::string>("CSV");
    std::cout << "Importing data from from file " << csv_name << std::endl;

  } else {

    // CSV file not provided
    std::cerr << arguments << std::endl;
    std::cerr << "ERROR: No CSV file provided!" << std::endl;
    _Exit(16);
  }

  std::cout << std::endl;
  default_n_specimen = arguments.get<unsigned int>("-s");
  default_probability = arguments.get<unsigned int>("-p");
  default_n_gens = arguments.get<unsigned int>("-g");
  default_output = arguments.get<unsigned int>("-o");

  std::cout << "n_specimen: " << default_n_specimen << std::endl;
  std::cout << "n_generations: " << default_n_gens << std::endl;
  std::cout << "mutation probability: " << default_probability << std::endl;
  std::cout << "output every n generations: " << default_output << std::endl;
  std::cout << std::endl;

  Bipartate b1(csv_name);
  b1.calc_score();
  specimen.push_back(b1);
  std::cout << "number of nodes: ";
  std::cout << std::to_string(b1.t1.size() + b1.t2.size()) << std::endl;
  std::cout << "number of edges: " << std::to_string(b1.edges.size());
  std::cout << std::endl;
  std::cout << "Score for Generation 0: " << b1.score << std::endl;
  std::cout << std::endl;
}

void Generation::evolve(unsigned int n_specimen, uint8_t chance) {

  // Killing by chance
  unsigned int n_size = specimen.size();

  std::vector<Bipartate> specimen_copy;

  for (unsigned int i = 0; i < n_size; i++) {
    if (i < Random::get<uint8_t>(1, n_size)) {
      specimen_copy.push_back(specimen[i]);
    }
  }

  // Replace specimen with copy
  specimen.clear(); specimen = specimen_copy; specimen_copy.clear();

  unsigned int i = 0;
  n_size = specimen.size();
  while (n_size < n_specimen) {
    specimen.push_back(specimen[i].mutate(chance));
    i++; n_size++;
  }
  std::sort(specimen.begin(), specimen.end());
  n_generation++;
}

void Generation::write_dot(bool all) {
  specimen[0].write_dot("best_gen_" +
                            std::to_string(n_generation) +
                            ".dot");

  if (all) {
    for (unsigned int i = 1; i < specimen.size(); ++i) {
      specimen[i].write_dot("specimen_" + std::to_string(i) + ".dot");
    }
  }
}
