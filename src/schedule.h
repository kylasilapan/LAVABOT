#pragma once

// True if time is inside either 09:00–09:59 or 14:00–14:59
inline bool inAutoWindow(int h, int m) {
  if (h == 9 && m >= 0) return true;
  if (h > 9 && h < 10) return true;
  if (h == 14 && m >= 0) return true;
  if (h > 14 && h < 15) return true;
  return false;
}

