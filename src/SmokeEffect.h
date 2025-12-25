#pragma once
#include <random>

#include "Effect.h"
#include "Utils.h"

class SmokeEffect : public Effect {
 private:
  glm::vec2 m_pos = {0.0f, 0.0f};
  uint32_t m_count{128};
  float m_damping = 0.0001f, m_size = 1.2f, m_alpha = 0.999f, m_velxfac = 200.0f,
        m_velyfac = 200.0f, m_initialsize = 64.0f;

 public:
  SmokeEffect(uint32_t);
  virtual ~SmokeEffect() = default;
  virtual void run(float);
  virtual void setup();

  const uint32_t get_count() const;

  const glm::vec2& get_position() const;

  float get_initial_size() const;

  float get_damping() const;

  float get_size() const;

  float get_alpha() const;

  float get_vel_x_factor() const;

  float get_vel_y_factor() const;

  void set_count(uint32_t);

  void set_position(const glm::vec2&);

  void set_position(float, float);

  void set_initial_size(float);

  void set_damping(float);

  void set_size(float);

  void set_alpha(float);

  void set_velocity_factors(float, float);

  void set_vel_x_factor(float);

  void set_vel_y_factor(float);
};
