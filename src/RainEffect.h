#pragma once
#include <random>

#include "Effect.h"
#include "Utils.h"

class RainEffect : public Effect {
private:
  glm::vec2 m_pos = {4096.0f, 2048.0f};
  uint32_t m_count{128};
  float m_damping = 0.0001f, m_size = 0.0f, m_alpha = 1.0f, m_velxfac = 0.0f,
        m_velyfac = -800.0f, m_initialsize = 256.0f;

public:
  explicit RainEffect(uint32_t);
  virtual ~RainEffect() = default;
  virtual void run(float);
  virtual void setup();

  const uint32_t get_count() const;

  const glm::vec2 &get_position() const;

  float get_initial_size() const;

  float get_damping() const;

  float get_size() const;

  float get_alpha() const;

  float get_vel_x_factor() const;

  float get_vel_y_factor() const;

  void set_count(uint32_t);

  void set_position(const glm::vec2 &);

  void set_position(float, float);

  void set_initial_size(float);

  void set_damping(float);

  void set_size(float);

  void set_alpha(float);

  void set_velocity_factors(float, float);

  void set_vel_x_factor(float);

  void set_vel_y_factor(float);
};
