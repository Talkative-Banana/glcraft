#include "Npc.hpp"

Npc::Npc(uint32_t id, Callback cb) : m_id(id), m_callback(cb) {
  std::cout << "NPC initalized\n";
}

// communicate
void Npc::communicate(std::string msg) {
  std::thread([this, msg]() {
    std::string reply = s_llm.generate(msg);
    m_callback(m_id, reply);
  }).detach();
}
