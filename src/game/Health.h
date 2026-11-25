// =========================================================================
// Simple hit-point pool for players and enemies
// =========================================================================
#ifndef __game__Health__h__
#define __game__Health__h__

namespace game {
struct Health {
  int current = 0;
  int maximum = 0;

  bool IsAlive() const { return (this->current > 0); }

  int ApplyDamage(int amount) {
    if (amount <= 0 || !this->IsAlive())
      return (0);

    const int before = this->current;
    this->current -= amount;
    if (this->current < 0)
      this->current = 0;
    return (before - this->current);
  }
};
} // namespace game

#endif // __game__Health__h__

// eof - Health.h
