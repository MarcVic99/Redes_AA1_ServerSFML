#pragma once

#include <array>
#include <cstddef>

// Network configuration.
inline constexpr unsigned short kListenerPort = 3360;

// Game configuration.
inline constexpr std::size_t kMaxPlayers = 4;
inline constexpr int kTopRankingLimit = 10;
inline constexpr int kSelectorWaitTimeMilliseconds = 50;

// Match scoring configuration.
inline constexpr int kFirstWinnerPoints = 400;
inline constexpr int kSecondWinnerPoints = 100;
inline constexpr int kThirdWinnerPoints = 50;
inline constexpr int kLoserPoints = -500;

inline constexpr std::array<int, 3> kWinnerPoints{
    kFirstWinnerPoints,
    kSecondWinnerPoints,
    kThirdWinnerPoints,
};
