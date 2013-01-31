// Copyright 2012 Google Inc. All Rights Reserved.
// Author: onufry@google.com (Onufry Wojtaszczyk)

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <vector>
#include <utility>
#include "server.h"

namespace {

const int kRevBit = 0;

using std::vector;

typedef int32_t int32t;

int random_query_size() {
  return 1000000 + (rand() % 10000000);
}

/**************** Column Server / Eater Interfaces ****************************/

// A class serving data from a single column.
class ColumnServer {
 public:
  ColumnServer(int query_size, int seed) : rows_left_(query_size) {
    srand48_r(seed, &rand_data_);
  }
  virtual ~ColumnServer() {}

  virtual int GetDoubles(int number, double* destination) {
    assert(false);
    return destination[number];  // Unreachable code.
  }
  virtual int GetInts(int number, int32t* destination) {
    assert(false);
    return destination[number];  // Unreachable code.
  }
  virtual int GetByteBools(int number, bool* destination) {
    assert(false);
    return destination[number];  // Unreachable code.
  }
  virtual int GetBitBools(int number, char* destination) {
    assert(false);
    return destination[number];  // Unreachable code.
  }

 protected:
  int random(int lo, int hi) {
    assert(hi >= lo);
    return lo + (lrand() % (hi - lo + 1));
  }

  int log_random(int max_power) {
    return 1 << (lrand() % (max_power + 1));
  }

  // Returns a U(0, 1) variable.
  double uniform() {
    return (double) (1 + (lrand() % ((1 << 30) - 1))) / (double) (1 << 30);
  }

  // Returns a N(0, 1) variable using the Box-Muller method.
  double normal() {
    return sqrt(-2. * log(uniform())) * cos(8 * atan(1) * uniform());
  }

  int Serve(int N) {
    if (N >= rows_left_) {
      N = rows_left_;
    }
    rows_left_ -= N;
    return N;
  }

 private:
  int lrand() {
    long int result;
    lrand48_r(&rand_data_, &result);
    return result;
  }

  struct drand48_data rand_data_;
  int rows_left_;
};

class ColumnEater {
 public:
  ColumnEater() {}
  virtual ~ColumnEater() {}

  virtual void EatDoubles(int number, const double *data) {
    assert(false);
    printf("%d\n", number += data[0]);  // Unreachable code.
  }

  virtual void EatInts(int number, const int *data) {
    assert(false);
    printf("%d\n", number += data[0]);  // Unreachable code.
  }

  virtual void EatByteBools(int number, const bool *data) {
    assert(false);
    printf("%d\n", number += data[0]);  // Unreachable code.
  }

  virtual void EatBitBools(int number, const char *data) {
    assert(false);
    printf("%d\n", number += data[0]);  // Unreachable code.
  }
};

/****************************** Data Server Implementation ********************/

// RealDataServer implementation.
class RealDataServer : public Server {
 public:
  RealDataServer(vector<ColumnServer *> column_servers,
                 vector<ColumnEater *> column_eaters);
  ~RealDataServer();
  int GetDoubles(int column_index, int number, double* destination);
  int GetInts(int column_index, int number, int32t* destination);
  int GetByteBools(int column_index, int number, bool* destination);
  int GetBitBools(int column_index, int number, char* destination);

  // The Consume methods printf the output to screen. This is useful for
  // correctness checking, for running benchmarks you should likely redefine
  // them to do nothing.
  void ConsumeDoubles(int column_index, int number, const double* destination);
  void ConsumeInts(int column_index, int number, const int32t* destination);
  void ConsumeByteBools(int column_index, int number, const bool* destination);
  void ConsumeBitBools(int column_index, int number, const char* destination);

 private:
  vector<ColumnServer *> column_servers_;
  vector<ColumnEater *> column_eaters_;
};


RealDataServer::RealDataServer(vector<ColumnServer *> column_servers,
                               vector<ColumnEater *> column_eaters)
    : column_servers_(column_servers),
      column_eaters_(column_eaters) {}

RealDataServer::~RealDataServer() {
  {
    vector<ColumnServer *>::iterator it;
    for (it = column_servers_.begin(); it != column_servers_.end(); ++it) {
      delete *it;
    }
  }
  {
    vector<ColumnEater *>::iterator it;
    for (it = column_eaters_.begin(); it != column_eaters_.end(); ++it) {
      delete *it;
    }
  }
}

int RealDataServer::GetDoubles(int column, int number, double* data) {
  return column_servers_[column]->GetDoubles(number, data);
}

int RealDataServer::GetInts(int column, int number, int32t* data) {
  return column_servers_[column]->GetInts(number, data);
}

int RealDataServer::GetByteBools(int column, int number, bool* data) {
  return column_servers_[column]->GetByteBools(number, data);
}

int RealDataServer::GetBitBools(int column, int number, char* data) {
  return column_servers_[column]->GetBitBools(number, data);
}

void RealDataServer::ConsumeDoubles(int column, int number, const double* data) {
  column_eaters_[column]->EatDoubles(number, data);
}

void RealDataServer::ConsumeInts(int column, int number, const int32t* data) {
  column_eaters_[column]->EatInts(number, data);
}

void RealDataServer::ConsumeByteBools(int column,
                                      int number,
                                      const bool* data) {
  column_eaters_[column]->EatByteBools(number, data);
}

void RealDataServer::ConsumeBitBools(int column,
                                     int number,
                                     const char* data) {
  column_eaters_[column]->EatBitBools(number, data);
}

/********************* Column Server Implementations **************************/

class DoubleColumnServer : public ColumnServer {
 public:
  DoubleColumnServer(int query_size, int seed)
      : ColumnServer(query_size, seed),
        low_range_(log_random(30)),
        high_range_(low_range_ + log_random(30)),
        zoom_range_(log_random(30)),
        normal_(random(0, 1)) {
  }

  DoubleColumnServer(int query_size, int around, int seed)
      : ColumnServer(query_size, seed),
      low_range_(around - log_random(20)),
      high_range_(around + log_random(20)),
      zoom_range_(log_random(10)),
      normal_(random(0, 1)) {
  }

  int GetDoubles(int number, double *destination) {
    number = Serve(number);
    for (int i = 0; i < number; ++i) {
      destination[i] = Generate();
    }
    return number;
  }

 private:
  double Generate() {
    if (!normal_) {
      return (double) random(low_range_, high_range_) /
             (double) random(1, zoom_range_);
    } else {
      return (low_range_ + high_range_) / 2. +
             normal() * (high_range_ - low_range_) / (2. * zoom_range_);
    }
  }

  const int low_range_;
  const int high_range_;
  const int zoom_range_;
  const bool normal_;
};

class IntColumnServer : public ColumnServer {
 public:
  IntColumnServer(int query_size, int seed)
      : ColumnServer(query_size, seed),
        low_range_(log_random(30)),
        high_range_(low_range_ + log_random(30)),
        normal_(random(0, 1)) {
  }

  IntColumnServer(int query_size, int around, int seed)
      : ColumnServer(query_size, seed),
        low_range_(around - log_random(20)),
        high_range_(around + log_random(20)),
        normal_(random(0, 1)) {
  }

  IntColumnServer(int query_size, int around, int spread, int seed)
      : ColumnServer(query_size, seed),
        low_range_(around - spread),
        high_range_(around + spread),
        normal_(random(0, 1)) {
  }

  int GetInts(int number, int *destination) {
    number = Serve(number);
    for (int i = 0; i < number; ++i) {
      destination[i] = Generate();
    }
    return number;
  }

 private:
  double Generate() {
    if (!normal_) {
      return random(low_range_, high_range_);
    } else {
      return (low_range_ + high_range_) / 2. +
             normal() * (high_range_ - low_range_) / 2.;
    }
  }

  const int low_range_;
  const int high_range_;
  const bool normal_;
};

class BoolColumnServer : public ColumnServer {
 public:
  BoolColumnServer(int query_size, int seed)
      : ColumnServer(query_size, seed),
        probability_(random(0, 100)) {
  }

  int GetByteBools(int number, bool *destination) {
    number = Serve(number);
    for (int i = 0; i < number; ++i) {
      destination[i] = Generate();
    }
    return number;
  }

  virtual int GetBitBools(int number, char* destination) {
    number = Serve(number);
    for (int i = 0; i < number; ++i) {
      if (!(i & 7)) destination[i / 8] = 0;
      destination[i / 8] |= (Generate() << (kRevBit ? 7 - (i & 7) : i & 7));
    }
    return number;
  }

 private:
  char Generate() {
    return (random(0, 99) < probability_);
  }

  const int probability_;
};

/************************** Hasher ColumnEater Implementation *****************/

class Hasher : public ColumnEater {
 public:
  explicit Hasher(int column) : rows_(0), hash_(0), column_(column) {}
  virtual ~Hasher() {
    fprintf(stderr,
            "Column %d: ate %d rows, hash %d.\n",
            column_, rows_, hash_);
  }

  int rows() const { return rows_; }
  int hash() const { return hash_; }

 protected:
  void add_rows(int rows) { rows_ += rows; }
  // Note that the hash will not depend on the row order. That's crucial for
  // correctness checking of group-by queries.
  void add_hash(int hash) { hash_ ^= hash; }
  // This will be used by custom hash implementation for Bool columns.
  void set_hash(int hash) { hash_ = hash; }

 private:
  int rows_;
  int hash_;
  const int column_;
};

class DoubleHasher : public Hasher {
 public:
  explicit DoubleHasher(int column) : Hasher(column) {}

  void EatDoubles(int number, const double *data) {
    add_rows(number);
    for (int i = 0; i < number; ++i) {
      add_hash(20000000 * data[i]);
    }
  }
};

class IntHasher : public Hasher {
 public:
  explicit IntHasher(int column) : Hasher(column) {}

  void EatInts(int number, const int *data) {
    add_rows(number);
    for (int i = 0; i < number; ++i) {
      add_hash(data[i]);
    }
  }
};

class BoolHasher : public Hasher {
 public:
  explicit BoolHasher(int column) : Hasher(column), trues_(0), falses_(0) {}

  void EatByteBools(int number, const bool *data) {
    add_rows(number);
    for (int i = 0; i < number; ++i) {
      add_bool_data(data[i]);
    }
  }

  void EatBitBools(int number, const char *data) {
    add_rows(number);
    for (int i = 0; i < number; ++i) {
      add_bool_data(data[i / 8] & (1 << (i & 7)));
    }
  }

 private:
  void add_bool_data(bool value) {
    if (value) trues_++;
    else falses_++;
    set_hash(trues_ ^ (19 * falses_ + 7));
  }

  int trues_;
  int falses_;
};

/************************ Printer ColumnEater Implementation ******************/

class IntPrinter : public ColumnEater {
 public:
  explicit IntPrinter(int column) : row_(0), column_(column) {}
  virtual ~IntPrinter() {}

  void EatInts(int number, const int *data) {
    for (int i = 0; i < number; ++i) {
      fprintf(stderr, "%d %d %d\n", row_++, column_, data[i]);
    }
  }

 private:
  int row_;
  const int column_;
};

class DoublePrinter : public ColumnEater {
 public:
  explicit DoublePrinter(int column) : row_(0), column_(column) {}
  virtual ~DoublePrinter() {}

  void EatDoubles(int number, const double *data) {
    for (int i = 0; i < number; ++i) {
      fprintf(stderr, "%d %d %.12lf\n", row_++, column_, data[i]);
    }
  }

 private:
  int row_;
  const int column_;
};

class BoolPrinter : public ColumnEater {
 public:
  explicit BoolPrinter(int column) : row_(0), column_(column) {}
  virtual ~BoolPrinter() {}

  void EatByteBools(int number, const bool *data) {
    for (int i = 0; i < number; ++i) {
      fprintf(stderr, "%d %d %d\n", row_++, column_, data[i]);
    }
  }

  void EatBitBools(int number, const char *data) {
    for (int i = 0; i < number; ++i) {
      fprintf(stderr, "%d %d %d\n", row_++, column_,
             (data[i/8] & (1 << (i & 7))) ? 1 : 0);
    }
  }

 private:
  int row_;
  const int column_;
};

class CompositeColumnEater : public ColumnEater {
 public:
  explicit CompositeColumnEater(vector<ColumnEater *> delegates)
      : delegates_(delegates) {}

  virtual ~CompositeColumnEater() {
    vector<ColumnEater *>::iterator it;
    for (it = delegates_.begin(); it != delegates_.end(); ++it) {
      delete *it;
    }
  }

  virtual void EatInts(int number, const int *data) {
    vector<ColumnEater *>::iterator it;
    for (it = delegates_.begin(); it != delegates_.end(); ++it) {
      (*it)->EatInts(number, data);
    }
  }

  virtual void EatDoubles(int number, const double *data) {
    vector<ColumnEater *>::iterator it;
    for (it = delegates_.begin(); it != delegates_.end(); ++it) {
      (*it)->EatDoubles(number, data);
    }
  }

  virtual void EatByteBools(int number, const bool *data) {
    vector<ColumnEater *>::iterator it;
    for (it = delegates_.begin(); it != delegates_.end(); ++it) {
      (*it)->EatByteBools(number, data);
    }
  }

  virtual void EatBitBools(int number, const char *data) {
    vector<ColumnEater *>::iterator it;
    for (it = delegates_.begin(); it != delegates_.end(); ++it) {
      (*it)->EatBitBools(number, data);
    }
  }

 private:
  vector<ColumnEater *> delegates_;
};

/************************ ColumnMap *******************************************/

// Utilities for ColumnMap.
vector<int> CreateVector(int c1) {
  vector<int> result;
  result.push_back(c1);
  return result;
}

vector<int> CreateVector(int c1, int c2) {
  vector<int> result = CreateVector(c1);
  result.push_back(c2);
  return result;
}

vector<int> CreateVector(int c1, int c2, int c3) {
  vector<int> result = CreateVector(c1, c2);
  result.push_back(c3);
  return result;
}

vector<int> CreateVector(int c1, int c2, int c3, int c4) {
  vector<int> result = CreateVector(c1, c2, c3);
  result.push_back(c4);
  return result;
}

vector<int> CreateVector(int c1, int c2, int c3, int c4, int c5) {
  vector<int> result = CreateVector(c1, c2, c3, c4);
  result.push_back(c5);
  return result;
}


// Column maps for queries. Correspond to the maps in sample queries given in
// the Wiki. Define your own if you want to run your own test queries.
//
// Recall: 1: INT, 2: DOUBLE, 3: BOOL.
vector<int> InputColumnMap(int query_id) {
  vector<int> result;
  switch(query_id) {
    case 1: return CreateVector(3, 3, 2, 2, 1);
    case 2: return CreateVector(1, 1);
    case 3: return CreateVector(2);
    case 4: return CreateVector(1, 1);
    case 5: return CreateVector(2, 2);
    case 6: return CreateVector(3, 3, 3, 1);
    case 7: return CreateVector(1);
    case 8: return CreateVector(2);
    case 9: return CreateVector(2, 1);
    case 10: return CreateVector(1, 1, 3);
    case 11: return CreateVector(1, 1);
    case 12: return CreateVector(1, 1, 2);
    default: assert(false);
  }
  return result;  // Unreachable code.
}

vector<int> OutputColumnMap(int query_id) {
  vector<int> result;
  switch(query_id) {
    case 1: return CreateVector(3, 3, 2, 2, 1);
    case 2: return CreateVector(1);
    case 3: return CreateVector(2);
    case 4: return CreateVector(2);
    case 5: return CreateVector(2);
    case 6: return CreateVector(2, 3, 1);
    case 7: return CreateVector(1, 1);
    case 8: return CreateVector(2);
    case 9: return CreateVector(2);
    case 10: return CreateVector(1, 1);
    case 11: return CreateVector(1);
    case 12: return CreateVector(1, 1);
    default: assert(false);
  }
  return result;  // Unreachable code.
}

/********************** Real Server Creation **********************************/
// The column_types vector should
// contain types of columns - if column_types[i] = j, then the i-th
// (0-indexed) column of the input is of type j (where 1 means int, 2 means
// double and 3 means bool).

ColumnEater *CreateHasherEater(int column, int type) {
  switch(type) {
    case 1: return new IntHasher(column);
    case 2: return new DoubleHasher(column);
    case 3: return new BoolHasher(column);
    default: assert(false);
  }
  return NULL;  // Unreachable code.
}

ColumnEater *CreatePrinterEater(int column, int type) {
  switch(type) {
    case 1: return new IntPrinter(column);
    case 2: return new DoublePrinter(column);
    case 3: return new BoolPrinter(column);
    default: assert(false);
  }
  return NULL;  // Unreachable code.
}

ColumnEater *CreateCompositeEater(int column, int type) {
  vector<ColumnEater *> delegates;
  delegates.push_back(CreateHasherEater(column, type));
  delegates.push_back(CreatePrinterEater(column, type));
  return new CompositeColumnEater(delegates);
}

ColumnEater *CreateVariantEater(int column, int type,
                                bool hasher, bool printer) {
  assert(hasher || printer);
  if (hasher && printer) return CreateCompositeEater(column, type);
  if (hasher) return CreateHasherEater(column, type);
  if (printer) return CreatePrinterEater(column, type);
  return NULL;  // Unreachable code.
}

Server *CreateDataServer(const vector<int> &in_column_types,
                         const vector<int> &out_column_types,
                         bool hashers,
                         bool printers,
                         int query_seed) {
  int query_size = random_query_size();
  assert(hashers || printers);

  vector<ColumnServer *> column_servers;
  for (unsigned i = 0; i < in_column_types.size(); ++i) {
    int seed = query_seed * 10 + i;
    switch(in_column_types[i]) {
      case 1:
        column_servers.push_back(new IntColumnServer(query_size, seed));
        break;
      case 2:
        column_servers.push_back(new DoubleColumnServer(query_size, seed));
        break;
      case 3:
        column_servers.push_back(new BoolColumnServer(query_size, seed));
        break;
      default: assert(false);
    }
  }
  vector<ColumnEater *> column_eaters;
  for (unsigned i = 0; i < out_column_types.size(); ++i) {
    int type = out_column_types[i];
    column_eaters.push_back(CreateVariantEater(i, type, hashers, printers));
  }
  return new RealDataServer(column_servers, column_eaters);
}

}  // namespace

Server *CreateServer(int query_id) {
  const bool kHashers = true;
  const bool kPrinters = false;

  srand(query_id);
  int query_size;
  vector<ColumnServer *> column_servers;
  vector<ColumnEater *> column_eaters;
  switch(query_id) {
    case 0:
      // Testing query:
      query_size = 5;
      column_servers.push_back(new IntColumnServer(5, 1));
      column_servers.push_back(new DoubleColumnServer(5, 2));
      column_servers.push_back(new BoolColumnServer(5, 3));
      column_eaters.push_back(CreateVariantEater(0, 1, true, true));
      column_eaters.push_back(CreateVariantEater(1, 2, true, true));
      column_eaters.push_back(CreateVariantEater(2, 3, true, true));
      return new RealDataServer(column_servers, column_eaters);
    case 7:
      // Intended to be an easy query, so we want to have roughly 10000 groups.
      query_size = random_query_size();
      column_servers.push_back(new IntColumnServer(query_size, 0, 5000, 70));
      column_eaters.push_back(CreateVariantEater(0, 1, kHashers, kPrinters));
      column_eaters.push_back(CreateVariantEater(1, 1, kHashers, kPrinters));
      return new RealDataServer(column_servers, column_eaters);
    case 8:
      // We group by a > 1., so it would be nice to have both positive and
      // negative a.
      query_size = random_query_size();
      column_servers.push_back(new DoubleColumnServer(query_size, 1, 80));
      column_eaters.push_back(CreateVariantEater(0, 2, kHashers, kPrinters));
      return new RealDataServer(column_servers, column_eaters);
    case 9:
      // We group by b, we aim at many groups.
      query_size = random_query_size();
      column_servers.push_back(new DoubleColumnServer(query_size, 90));
      column_servers.push_back(
          new IntColumnServer(query_size, 0, 100000000, 91));
      column_eaters.push_back(CreateVariantEater(0, 2, kHashers, kPrinters));
      return new RealDataServer(column_servers, column_eaters);
    case 11:
      // We have a == b in the query, so we want the two input variables to
      // overlap.
      query_size = 98765432;
      column_servers.push_back(
          new IntColumnServer(query_size, 1234, 10000, 110));
      column_servers.push_back(
          new IntColumnServer(query_size, 1357, 10000, 111));
      column_eaters.push_back(CreateVariantEater(0, 1, kHashers, kPrinters));
      return new RealDataServer(column_servers, column_eaters);
    case 12:
      query_size = 10000000;
      column_servers.push_back(new IntColumnServer(query_size, 0, 5000, 120));
      column_servers.push_back(new IntColumnServer(query_size, 0, 5000, 121));
      column_servers.push_back(new DoubleColumnServer(query_size, 0, 122));
      column_eaters.push_back(CreateVariantEater(0, 1, kHashers, kPrinters));
    default: return CreateDataServer(InputColumnMap(query_id),
                                     OutputColumnMap(query_id),
                                     kHashers,
                                     kPrinters,
                                     query_id);
  }
}
