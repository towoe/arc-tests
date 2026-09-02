#include "counter-arc.h"
#include <cstdio>
#include <cstring>
#include <string>

// A 10ns clock period. The runtime keeps time in femtoseconds.
static constexpr uint64_t halfPeriodFs = 5'000'000;

class Testbench {
public:
  // From "counter-arc.h"
  Counter model;

  // `args` is the runtime option string, e.g. "vcd;traceFile=counter.vcd".
  explicit Testbench(const char *args) : model(args) { set_time(); }

  void tick() {
    model.view.clock = 0;
    step();
    model.view.clock = 1;
    step();
  }

private:
  void step() {
    time += halfPeriodFs;
    set_time();
    model.eval();
  }

  // The runtime reads the simulation time as an i64 from offset 0 of the model
  // state; it timestamps trace samples with it.
  void set_time() { *reinterpret_cast<uint64_t *>(model.storage) = time; }

  uint64_t time = 0;
};

int main(int argc, char **argv) {
  std::string runtimeArgs;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--trace") == 0 && i + 1 < argc) {
      // `vcd` selects the encoder, `traceFile` overrides the default name.
      runtimeArgs += "vcd;traceFile=" + std::string(argv[++i]) + ";";
    } else {
      fprintf(stderr, "usage: %s [--trace <file.vcd>]\n", argv[0]);
      return 1;
    }
  }

  Testbench tb(runtimeArgs.empty() ? nullptr : runtimeArgs.c_str());
  auto &view = tb.model.view;

  view.reset = 1;
  view.en = 0;
  view.load = 0;
  view.load_value = 0;
  tb.tick();
  tb.tick();
  view.reset = 0;
  view.en = 1;
  for (unsigned i = 1; i <= 16; ++i) {
    tb.tick();
  }
}
