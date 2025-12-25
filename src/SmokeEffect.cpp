#include "SmokeEffect.h"

SmokeEffect::SmokeEffect(uint32_t number) : Effect(number), m_count(number) {
}

void SmokeEffect::setup() {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  for (size_t i = 0; i < m_count; ++i) {
    glm::vec2 vel{dist(rng) * m_velxfac, dist(rng) * m_velyfac};
    m_particles.push_back({m_pos, vel, m_initialsize, dist(rng) >= 0.5f});
  }
}

void SmokeEffect::run(float dt) {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (int i = 0; i < m_particles.size(); i++) {
    auto& particle = m_particles[i];
    // Skip non visible particles
    if (!particle.is_visible()) continue;

    float angle = dist(rng);
    // rotation
    particle.update_angle(angle / 2.0, dt);
    // increase size
    particle.update_size(m_size);
    // move
    particle.update(m_damping, dt);
    // decrease alpha
    particle.update_alpha(m_alpha);
  }
}

const uint32_t SmokeEffect::get_count() const {
  return m_count;
}

const glm::vec2& SmokeEffect::get_position() const {
  return m_pos;
}

float SmokeEffect::get_initial_size() const {
  return m_initialsize;
}

float SmokeEffect::get_damping() const {
  return m_damping;
}

float SmokeEffect::get_size() const {
  return m_size;
}

float SmokeEffect::get_alpha() const {
  return m_alpha;
}

float SmokeEffect::get_vel_x_factor() const {
  return m_velxfac;
}

float SmokeEffect::get_vel_y_factor() const {
  return m_velyfac;
}

void SmokeEffect::set_count(uint32_t count) {
  m_count = count;
}

void SmokeEffect::set_position(const glm::vec2& pos) {
  m_pos = pos;
}

void SmokeEffect::set_position(float x, float y) {
  m_pos = {x, y};
}

void SmokeEffect::set_initial_size(float size) {
  m_initialsize = std::max(1.0f, size);
}

void SmokeEffect::set_damping(float damping) {
  m_damping = std::max(0.0f, damping);
}

void SmokeEffect::set_size(float size) {
  m_size = std::max(0.0f, size);
}

void SmokeEffect::set_alpha(float alpha) {
  m_alpha = glm::clamp(alpha, 0.0f, 1.0f);
}

void SmokeEffect::set_velocity_factors(float x, float y) {
  m_velxfac = x;
  m_velyfac = y;
}

void SmokeEffect::set_vel_x_factor(float x) {
  m_velxfac = x;
}

void SmokeEffect::set_vel_y_factor(float y) {
  m_velyfac = y;
}
