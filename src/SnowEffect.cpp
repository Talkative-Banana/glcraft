#include "SnowEffect.h"

SnowEffect::SnowEffect(uint32_t number) : Effect(number), m_count(number) {
}

void SnowEffect::setup() {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  for (size_t i = 0; i < m_count; ++i) {
    glm::vec2 vel{dist(rng) * m_velxfac, dist(rng) * m_velyfac};
    m_pos.x = dist(rng) * 4096.0f;
    m_particles.push_back({m_pos, vel, m_initialsize, dist(rng) >= 0.5f});
  }
}

void SnowEffect::run(float dt) {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (int i = 0; i < m_particles.size(); i++) {
    auto& particle = m_particles[i];
    // Skip non visible particles
    if (!particle.is_visible()) continue;

    // float angle = dist(rng);
    // rotation
    // particle.update_angle(angle / 2.0, dt);
    // increase size
    // particle.update_size(m_size);
    // move
    particle.update(m_damping, dt);
    // decrease alpha
    // particle.update_alpha(m_alpha);
    auto pos = particle.get_position();
    if (pos.x < -256.0f || pos.y < -256.0f) {
      float X = dist(rng) * 4096.0f;
      particle.update_pos({X, 2048.0f});
      glm::vec2 vel{dist(rng) * m_velxfac, dist(rng) * m_velyfac};
      particle.update_vel(vel);
    }
  }
}

const uint32_t SnowEffect::get_count() const {
  return m_count;
}

const glm::vec2& SnowEffect::get_position() const {
  return m_pos;
}

float SnowEffect::get_initial_size() const {
  return m_initialsize;
}

float SnowEffect::get_damping() const {
  return m_damping;
}

float SnowEffect::get_size() const {
  return m_size;
}

float SnowEffect::get_alpha() const {
  return m_alpha;
}

float SnowEffect::get_vel_x_factor() const {
  return m_velxfac;
}

float SnowEffect::get_vel_y_factor() const {
  return m_velyfac;
}

void SnowEffect::set_count(uint32_t count) {
  m_count = count;
}

void SnowEffect::set_position(const glm::vec2& pos) {
  m_pos = pos;
}

void SnowEffect::set_position(float x, float y) {
  m_pos = {x, y};
}

void SnowEffect::set_initial_size(float size) {
  m_initialsize = std::max(1.0f, size);
}

void SnowEffect::set_damping(float damping) {
  m_damping = std::max(0.0f, damping);
}

void SnowEffect::set_size(float size) {
  m_size = std::max(0.0f, size);
}

void SnowEffect::set_alpha(float alpha) {
  m_alpha = glm::clamp(alpha, 0.0f, 1.0f);
}

void SnowEffect::set_velocity_factors(float x, float y) {
  m_velxfac = x;
  m_velyfac = y;
}

void SnowEffect::set_vel_x_factor(float x) {
  m_velxfac = x;
}

void SnowEffect::set_vel_y_factor(float y) {
  m_velyfac = y;
}
