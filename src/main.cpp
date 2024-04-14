#include <iostream>
#include <vector>

double kahan_sum(std::vector<double> &values) {
  double c = 0.0;
  double sum = 0.0;

  for (auto i = values.begin(); i != values.end(); ++i) {
    double y = *i - c;
    double t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }

  return sum;
}

int main() {
  return 0;
}