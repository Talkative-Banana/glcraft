#include "llama.h"
#include <mutex>
#include <string>
#include <vector>

class LLM {
public:
  llama_model *model;
  llama_context *ctx;
  std::mutex mtx;

  void init(const std::string &path) {
    llama_backend_init();

    llama_model_params mparams = llama_model_default_params();
    model = llama_model_load_from_file(path.c_str(), mparams);

    auto params = llama_context_default_params();
    params.n_ctx = 2048;
    params.n_threads = 8;

    llama_context_params cparams = llama_context_default_params();
    ctx = llama_init_from_model(model, cparams);
  }

  std::string generate(const std::string &playerprompt) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string prompt =
        "You are one of the few people left in a post eliptic world, due to "
        "excessive cold here on earth many people have perished.\n"
        "You speak briefly and stay in character.\n"
        "Player: " +
        playerprompt +
        "\n"
        "You:";

    const llama_vocab *vocab = llama_model_get_vocab(model);

    // --- tokenize ---
    std::vector<llama_token> tokens(prompt.size() + 10);

    int n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(),
                                  tokens.data(), tokens.size(), true, true);

    tokens.resize(n_tokens);

    // --- initial decode ---
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch) != 0) {
      return "[decode error]";
    }

    std::string output;

    // --- create sampler ONCE ---
    llama_sampler *sampler = llama_sampler_init_greedy();

    for (int i = 0; i < 100; i++) {
      // --- sample next token ---
      llama_token token = llama_sampler_sample(sampler, ctx, -1);

      if (token == llama_vocab_eos(vocab)) {
        break;
      }

      // --- convert to string ---
      char buf[1024]; // enough for most tokens

      int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);

      if (n > 0) {
        output.append(buf, n);
      }

      // --- feed token back (THIS replaces llama_eval) ---
      llama_batch next_batch = llama_batch_get_one(&token, 1);

      if (llama_decode(ctx, next_batch) != 0) {
        break;
      }
    }

    llama_sampler_free(sampler);

    return output;
  }
};
