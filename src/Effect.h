#pragma once
#include <array>
#include <random>

#include "Constants.hpp"
#include "Quad.h"
#include "Utils.h"

class Effect {
protected:
  std::vector<Quad> m_particles;
  std::vector<float> data;
  uint32_t m_size{};
  inline static std::mt19937 rng{std::random_device{}()};

public:
  Effect(uint32_t);
  virtual ~Effect() = default;
  // run -> get_particle -> draw
  virtual void run(float) = 0;
  virtual void setup() = 0;
  virtual void set_size(float) = 0;
  virtual void set_initial_size(float) = 0;
  uint32_t get_size();
  std::vector<float> get_data();
  std::vector<Quad> get_particles();

  bool effect_visible{true};
};
