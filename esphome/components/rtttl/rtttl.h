#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace rtttl {

extern uint32_t global_rtttl_id;

class Rtttl : public Component {
 public:
  void set_output(output::FloatOutput *output) { output_ = output; }
  void play(std::string rtttl);
  void stop() {
    next_tone_play_ = 0;
    output_->set_level(0.0);
  }
  void dump_config() override;

  bool is_playing() { return next_tone_play_ != 0; }
  void loop() override;

  void add_on_finished_playback_callback(std::function<void()> callback) {
    this->on_finished_playback_callback_.add(std::move(callback));
  }

 protected:
  std::string rtttl_;
  std::string::const_iterator p_;
  uint32_t wholenote_;
  uint32_t default_duration_;
  uint32_t default_octave_;

  uint32_t next_tone_play_{0};
  uint32_t output_freq_;
  bool note_playing_;
  output::FloatOutput *output_;

  CallbackManager<void()> on_finished_playback_callback_;
};

template<typename... Ts> class PlayAction : public Action<Ts...> {
 public:
  PlayAction(Rtttl *rtttl) : rtttl_(rtttl) {}
  TEMPLATABLE_VALUE(std::string, value)

  void play(Ts... x) override { this->rtttl_->play(this->value_.value(x...)); }

 protected:
  Rtttl *rtttl_;
};

template<typename... Ts> class StopAction : public Action<Ts...>, public Parented<Rtttl> {
 public:
  void play(Ts... x) override { this->parent_->stop(); }
};

template<typename... Ts> class IsPlayingCondition : public Condition<Ts...>, public Parented<Rtttl> {
 public:
  bool check(Ts... x) override { return this->parent_->is_playing(); }
};

class FinishedPlaybackTrigger : public Trigger<> {
 public:
  explicit FinishedPlaybackTrigger(Rtttl *parent) {
    parent->add_on_finished_playback_callback([this]() { this->trigger(); });
  }
};

}  // namespace rtttl
}  // namespace esphome
