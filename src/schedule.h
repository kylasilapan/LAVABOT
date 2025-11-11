#pragma once

// True if time is inside either 09:00–09:59 or 14:00–14:59
// Returns false for invalid time values (hours outside 0-23, minutes outside 0-59)
inline bool inAutoWindow(int h, int m) {
  // Validate input: hours must be 0-23, minutes must be 0-59
  if (h < 0 || h > 23 || m < 0 || m > 59) {
    return false;
  }
  
  // Check if within schedule windows
  if (h == 9) return true;  // 09:00-09:59
  if (h == 14) return true;  // 14:00-14:59
  return false;
}

