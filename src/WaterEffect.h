#pragma once
#include <random>

#include "Effect.h"
#include "Utils.h"

class WaterEffect : public Effect {
private:
  glm::vec2 m_pos = {320.0f, 320.0f};
  uint32_t m_count{1};
  float m_damping = 0.0000f, m_size = 0.0f, m_alpha = 0.4f, m_velxfac = 0.0f,
        m_velyfac = 0.0f, m_initialsize = 640.0f;

public:
  WaterEffect(uint32_t);
  virtual ~WaterEffect() = default;
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
