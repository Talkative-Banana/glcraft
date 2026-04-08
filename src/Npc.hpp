#pragma once
#include "Llama.hpp"
#include "Utils.h"
#include <functional>
#include <thread>

class Npc {
public:
  using Callback = std::function<void(uint32_t, const std::string &)>;
  Npc(uint32_t, Callback);
  // communicate
  void communicate(std::string);

  Npc(const Npc &) = delete;
  Npc &operator=(const Npc &) = delete;

  Npc(Npc &&) noexcept = default;
  Npc &operator=(Npc &&) noexcept = default;

public:
  inline static LLM s_llm;

private:
  uint32_t m_id{};
  std::string m_name{"unnamed"};
  std::string m_context{"default"};
  Callback m_callback;
};
