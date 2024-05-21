//
// Created by konstantin on 14.05.24.
//

#pragma once

#include <runner.h>

#include <buffer/buffer.h>
#include <engine/engine_context.h>

namespace go::impl::ctx {

class Context final : private Runner {
public:
    Context() = default;

    void Setup(BufferView target_buffer, Runner* runner);

    void SwitchTo(Context& target);

    void ExitTo(Context& target);

private:
    void Run() override;

    EngineContext engine_ctx_;
    Runner* runner_{nullptr};
};

}  // namespace go::impl::ctx