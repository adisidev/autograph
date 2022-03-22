#pragma once

#include "argparse.hpp"

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

#define t2_more_than_t1 1000

struct Pos {
  int x;
  int y;

  Pos() {
    x = 0; y = 0;
  }
  Pos(double x1, double y1) {
    x = x1; y = y1;
  }
};

struct Node {
  int id;
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
      dot += std::to_string(id) + "[color = blue, ";
    } else {
      dot += std::to_string(id + t2_more_than_t1) + "[color = red, ";
    }
    // dot += "label = " + label + ", "
    dot += "pos = " +
      std::to_string(pos.x) +
      "," +
      std::to_string(pos.y) +
      "!];\n";
    return dot;
  }

};

struct Edge {
  int from;
  int to;
  int weight;
  Edge(int f, int t) {
    from = f; to = t;
  }

  std::string as_dot() {
    std::string dot = "  ";
    dot += std::to_string(from) + " -- " + std::to_string(to + t2_more_than_t1);
    dot += "[label = \"" + std::to_string(weight) + "\"];\n";
    return dot;
  }
};

struct Bipartate {

  std::unordered_map<int, Node> t1;
  std::unordered_map<int, Node> t2;
  std::vector<Edge> edges;
  std::unordered_map<std::pair<int, int>, int,
    boost::hash<std::pair<int, int>>> positions;
  Bipartate(std::string csv_name);
  void print_dot(std::string file_name);

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
    t1[n_row];

    // Column index
    unsigned int n_col = 0;

    // Read one , delimitted value at a time
    while (getline (s, val, ',')) {
      unsigned int weight = std::stoi(val);

      // Initialise t2 at n_col
      t2[n_col];

      // If linked
      if (weight) {

        // Add connection from t1 to t2
        t1[n_row].connections.insert(n_col);

        // Add connection from t2 to t1
        t2[n_col].connections.insert(n_row);

        // Add edge from t1 to t2
        edges.push_back(Edge(n_row, n_col));
      }
    }
  }

  // First tower
  // "Random" order based on unordered_map
  unsigned int x = 0;
  unsigned int y = 0;
  for (auto& it: t1) {
    positions[std::make_pair(x, y)] = it.first;
    it.second.pos = Pos(x, y);
    y++;
  }

  // Next to first tower
  x++;
  y = 0;

  // "Random" order based on unordered_map
  for (auto& it: t2) {
    positions[std::make_pair(x, y)] = it.first;
    it.second.pos = Pos(x, y);
    it.second.is_t1 = false;
    y++;
  }
}

void Bipartate::print_dot(std::string file_name) {
  std::ofstream f;
  f.open(file_name);
  f << "graph autograph {\n";
  f << "// Nodes\n";

  for (auto& it: t1) {
    f << it.second.as_dot();
  }
  for (auto& it: t2) {
    f << it.second.as_dot();
  }

  f << "// Edges\n";
  for (Edge e : edges) {
    f << e.as_dot();
  }
  f << "}";
  f.close();
  return;
}

struct Generation {

  // What generation we are on
  int gen_count;

  // All Graphs of this generation
  std::vector<Bipartate> gen;

  // Scores of in this generation
  unsigned int worst_score;
  // double percentile_25; // 25th percentile
  unsigned int median_score; // 50th percentile
  // double percentile_75; // 75th percentile
  unsigned int best_score;

  // Name of CSV that this was initially created from
  std::string csv_name;

  Generation(int argc, char **argv);

};

Generation::Generation(int argc, char **argv) {

  // Create parser for arguments using argparse.
  // From https://github.com/p-ranav/argparse
  argparse::ArgumentParser arguments("autograph", "1.0");

  // Positional argument accepting CSV file
  arguments.add_argument("CSV")
    .default_value("none")
    .help("File path: CSV file");

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

  Bipartate b1(csv_name);
}
