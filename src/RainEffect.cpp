#include "RainEffect.h"

RainEffect::RainEffect(uint32_t number) : Effect(number), m_count(number) {}

void RainEffect::setup() {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  for (size_t i = 0; i < m_count; ++i) {
    glm::vec2 vel{0, m_velyfac * (dist(rng) + 1.0f)};
    glm::vec2 tmp_pos = {dist(rng) * m_pos.x, m_pos.y};
    m_particles.push_back({tmp_pos, vel, m_initialsize, dist(rng) >= 0.5f});
  }
}

void RainEffect::run(float dt) {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (int i = 0; i < m_particles.size(); i++) {
    auto &particle = m_particles[i];
    // Skip non visible particles
    if (!particle.is_visible() || !effect_visible ||
        (i >= fraction * m_particles.size())) {
      particle.update_alpha_to({});
      continue;
    }
    particle.update_alpha_to(m_alpha);
    particle.update(m_damping, dt);
    auto pos = particle.get_position();
    if (pos.y < -m_initialsize) {
      float X = dist(rng) * m_pos.x;
      particle.update_pos({X, m_pos.y});
      glm::vec2 vel{0, m_velyfac * (dist(rng) + 1.0f)};
      particle.update_vel(vel);
    }
  }
}

const uint32_t RainEffect::get_count() const { return m_count; }

const glm::vec2 &RainEffect::get_position() const { return m_pos; }

float RainEffect::get_initial_size() const { return m_initialsize; }

float RainEffect::get_damping() const { return m_damping; }

float RainEffect::get_size() const { return m_size; }

float RainEffect::get_alpha() const { return m_alpha; }

float RainEffect::get_vel_x_factor() const { return m_velxfac; }

float RainEffect::get_vel_y_factor() const { return m_velyfac; }

void RainEffect::set_count(uint32_t count) { m_count = count; }

void RainEffect::set_position(const glm::vec2 &pos) { m_pos = pos; }

void RainEffect::set_position(float x, float y) { m_pos = {x, y}; }

void RainEffect::set_initial_size(float size) {
  m_initialsize = std::max(1.0f, size);
}

void RainEffect::set_damping(float damping) {
  m_damping = std::max(0.0f, damping);
}

void RainEffect::set_size(float size) { m_size = std::max(0.0f, size); }

void RainEffect::set_alpha(float alpha) {
  m_alpha = glm::clamp(alpha, 0.0f, 1.0f);
}

void RainEffect::set_velocity_factors(float x, float y) {
  m_velxfac = x;
  m_velyfac = y;
}

void RainEffect::set_vel_x_factor(float x) { m_velxfac = x; }

void RainEffect::set_vel_y_factor(float y) { m_velyfac = y; }
