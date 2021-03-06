#include <engine/core/game_loop.hpp>

#include <engine/core/clock.hpp>
#include <engine/core/ecs.hpp>
#include <engine/core/update_system.hpp>

namespace engine::core {

Game_loop::Game_loop(class Clock& game_clock, class Ecs& ecs,
                     class Update_system& update_system)
    : game_clock_(game_clock), ecs_(ecs), update_system_(update_system) {}

void Game_loop::run() {
    if (running_) {
        return;
    }

    running_ = true;

    std::chrono::nanoseconds accumulated_dt{0};
    auto previous = std::chrono::steady_clock::now();
    while (running_) {
        auto current = std::chrono::steady_clock::now();

        game_clock_.update(current - previous);
        ecs_.update();

        std::chrono::nanoseconds dt = game_clock_.update_dt();
        accumulated_dt += dt;

        double simulation_rate_seconds =
                std::chrono::duration<double>{simulation_rate_}.count();

        while (accumulated_dt >= simulation_rate_) {
            accumulated_dt -= simulation_rate_;

            simulation_rate_seconds =
                    std::chrono::duration<double>{simulation_rate_}.count();

            update_system_.broadcast_fixed_update(simulation_rate_seconds);
        }

        update_system_.broadcast_interpolation_update(
                std::chrono::duration<double>{accumulated_dt}.count() /
                simulation_rate_seconds);

        update_system_.broadcast_variable_update(
                std::chrono::duration<double>{dt}.count());

        previous = current;
    }
}

} // namespace engine::core