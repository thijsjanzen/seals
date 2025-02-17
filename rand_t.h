#ifndef RANDOM_THIJS
#define RANDOM_THIJS
#include <random>

struct rnd_t {
  std::mt19937 rndgen;

  rnd_t() {
    std::random_device rd;
    std::mt19937 rndgen_t(rd());
    rndgen = rndgen_t;
  }

  rnd_t(size_t seed) {
    std::mt19937 rndgen_t(seed);
    rndgen = rndgen_t;
  }

  int random_number(size_t n)    {
    if(n <= 1) return 0;
    return std::uniform_int_distribution<> (0, static_cast<int>(n - 1))(rndgen);
  }

  void set_seed(unsigned seed)    {
    std::mt19937 new_randomizer(seed);
    rndgen = new_randomizer;
  }

  bool bernouilli(double p) {
    std::bernoulli_distribution d(p);
    return(d(rndgen));
  }

  double normal_positive(double m, double s) {
    std::normal_distribution<double> norm_dist(m, s);
    double  output = norm_dist(rndgen);
    while(output < 0) output = norm_dist(rndgen);
    return output;
  }
};


#endif
