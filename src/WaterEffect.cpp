#include "WaterEffect.h"

WaterEffect::WaterEffect(uint32_t number) : Effect(number), m_count(number) {}

void WaterEffect::setup() {
  for (size_t i = 0; i < m_count; ++i) {
    Quad quad = {m_pos, {}, m_initialsize, false};
    quad.update_alpha_to(m_alpha);
    m_particles.push_back(quad);
  }
}

void WaterEffect::run(float dt) {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (int i = 0; i < m_particles.size(); i++) {
    auto &particle = m_particles[i];
    // Skip non visible particles
    if (!particle.is_visible() || !effect_visible) {
      particle.update_size_to(0);
      particle.update_pos({});
      continue;
    }

    particle.update_size_to(m_initialsize);
    particle.update_pos({m_initialsize / 2.0, m_initialsize / 2.0});
  }
}

const uint32_t WaterEffect::get_count() const { return m_count; }

const glm::vec2 &WaterEffect::get_position() const { return m_pos; }

float WaterEffect::get_initial_size() const { return m_initialsize; }

float WaterEffect::get_damping() const { return m_damping; }

float WaterEffect::get_size() const { return m_size; }

float WaterEffect::get_alpha() const { return m_alpha; }

float WaterEffect::get_vel_x_factor() const { return m_velxfac; }

float WaterEffect::get_vel_y_factor() const { return m_velyfac; }

void WaterEffect::set_count(uint32_t count) { m_count = count; }

void WaterEffect::set_position(const glm::vec2 &pos) { m_pos = pos; }

void WaterEffect::set_position(float x, float y) { m_pos = {x, y}; }

void WaterEffect::set_initial_size(float size) {
  m_initialsize = std::max(1.0f, size);
}

void WaterEffect::set_damping(float damping) {
  m_damping = std::max(0.0f, damping);
}

void WaterEffect::set_size(float size) { m_size = std::max(0.0f, size); }

void WaterEffect::set_alpha(float alpha) {
  m_alpha = glm::clamp(alpha, 0.0f, 1.0f);
}

void WaterEffect::set_velocity_factors(float x, float y) {
  m_velxfac = x;
  m_velyfac = y;
}

void WaterEffect::set_vel_x_factor(float x) { m_velxfac = x; }

void WaterEffect::set_vel_y_factor(float y) { m_velyfac = y; }
