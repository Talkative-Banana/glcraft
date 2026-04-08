#pragma once
#include "Constants.hpp"
#include "Npc.hpp"
#include <vector>

class NpcManager {
public:
  NpcManager() { Npc::s_llm.init(llmModel); };

  void Add(Npc);
  void Add(uint32_t);
  void Send(uint32_t, std::string);
  void recvMsg(uint32_t, const std::string &);
  // Get Responses
  std::vector<std::vector<std::string>> getResponses() { return m_responses; }

private:
  std::vector<Npc> m_npc;
  std::vector<std::vector<std::string>> m_responses;
};
