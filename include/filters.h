#pragma once
/**
 * filters.h
 * Blocos de processamento de sinal reutilizáveis:
 *  - MovingAverage: média móvel em buffer circular (suaviza ruído do ADC/DHT)
 *  - Hysteresis: comparador com histerese (evita liga/desliga repetido)
 */
#include <stddef.h>

template <size_t N>
class MovingAverage {
 public:
  float update(float sample) {
    sum_ -= buffer_[index_];
    buffer_[index_] = sample;
    sum_ += sample;
    index_ = (index_ + 1) % N;
    if (count_ < N) count_++;
    return value();
  }
  float value() const { return count_ ? sum_ / count_ : 0.0f; }
  void reset() {
    for (size_t i = 0; i < N; i++) buffer_[i] = 0.0f;
    sum_ = 0.0f; index_ = 0; count_ = 0;
  }

 private:
  float buffer_[N] = {0};
  float sum_ = 0.0f;
  size_t index_ = 0;
  size_t count_ = 0;
};

class Hysteresis {
 public:
  /**
   * @param onThreshold   limiar que ativa a saída
   * @param offThreshold  limiar que desativa a saída
   * @param activeAbove   true: ativa quando o valor SOBE até onThreshold
   *                      false: ativa quando o valor DESCE até onThreshold
   */
  Hysteresis(float onThreshold, float offThreshold, bool activeAbove)
      : on_(onThreshold), off_(offThreshold), activeAbove_(activeAbove) {}

  bool update(float v) {
    if (activeAbove_) {
      if (!state_ && v >= on_) state_ = true;
      else if (state_ && v <= off_) state_ = false;
    } else {
      if (!state_ && v <= on_) state_ = true;
      else if (state_ && v >= off_) state_ = false;
    }
    return state_;
  }
  bool state() const { return state_; }
  void reset() { state_ = false; }

 private:
  float on_, off_;
  bool activeAbove_;
  bool state_ = false;
};
