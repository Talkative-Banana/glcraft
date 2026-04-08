#include "NpcManager.hpp"

void NpcManager::Add(Npc npc) {
  m_npc.push_back(std::move(npc));
  m_responses.push_back({});
}

void NpcManager::Add(uint32_t id) {
  auto lambda = [this](uint32_t nid, const std::string &msg) {
    this->recvMsg(nid, msg);
  };
  m_npc.emplace_back(id, lambda);
  m_responses.push_back({});
}

void NpcManager::Send(uint32_t id, std::string msg) {
  if (id < m_npc.size()) {
    m_npc[id].communicate(msg);
  }
}

void NpcManager::recvMsg(uint32_t id, const std::string &msg) {
  m_responses[id].push_back(msg);
}
