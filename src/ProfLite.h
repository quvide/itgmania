#pragma once
// ProfLite: minimal wall-clock profiler used to investigate music wheel frame
// time. Scopes accumulate per-name call count / total / max / min in
// microseconds. Results are written to the log as "[CProf]" lines every
// DumpIntervalUs (MaybeDump is called once per frame from the game loop), and
// each dump resets the accumulators so every dump is a fresh window.
//
// Define PROFLITE_ENABLED 0 to compile every macro to nothing.

#include <algorithm>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "RageLog.h"
#include "RageTimer.h"

#ifndef PROFLITE_ENABLED
#define PROFLITE_ENABLED 0
#endif

// Per-child timing of the wheel item "Set" dispatch. Costs roughly 150 us per
// item Set on top of the real work, so keep it off unless attributing that cost.
#ifndef PROFLITE_PER_CHILD
#define PROFLITE_PER_CHILD 0
#endif

namespace ProfLite {

struct Stat {
  uint64_t n = 0;
  uint64_t total = 0;
  uint64_t max = 0;
  uint64_t min = UINT64_MAX;
};

inline std::mutex& Mutex() {
  static std::mutex m;
  return m;
}
inline std::map<std::string, Stat>& Stats() {
  static std::map<std::string, Stat> s;
  return s;
}
inline std::map<std::string, uint64_t>& Counters() {
  static std::map<std::string, uint64_t> c;
  return c;
}
inline uint64_t& LastDump() {
  static uint64_t t = 0;
  return t;
}
constexpr uint64_t DumpIntervalUs = 10ull * 1000 * 1000;

inline uint64_t Now() {
#if PROFLITE_ENABLED
  return RageTimer::GetTimeSinceStartMicroseconds();
#else
  return 0;
#endif
}

inline void Add(const std::string& name, uint64_t us) {
#if !PROFLITE_ENABLED
  (void)name;
  (void)us;
  return;
#endif
  std::lock_guard<std::mutex> lock(Mutex());
  Stat& s = Stats()[name];
  s.n++;
  s.total += us;
  if (us > s.max) s.max = us;
  if (us < s.min) s.min = us;
}

inline void Count(const std::string& name, uint64_t n = 1) {
#if !PROFLITE_ENABLED
  (void)name;
  (void)n;
  return;
#endif
  std::lock_guard<std::mutex> lock(Mutex());
  Counters()[name] += n;
}

inline void Dump(const char* reason) {
  std::vector<std::pair<std::string, Stat>> rows;
  std::vector<std::pair<std::string, uint64_t>> counts;
  {
    std::lock_guard<std::mutex> lock(Mutex());
    rows.assign(Stats().begin(), Stats().end());
    counts.assign(Counters().begin(), Counters().end());
    Stats().clear();
    Counters().clear();
    LastDump() = Now();
  }
  std::sort(rows.begin(), rows.end(),
            [](const std::pair<std::string, Stat>& a,
               const std::pair<std::string, Stat>& b) {
              return a.second.total > b.second.total;
            });
  LOG->Trace("[CProf] ---- %s ----", reason);
  LOG->Trace("[CProf] %-52s %8s %10s %9s %9s %9s", "section", "calls",
             "total ms", "avg us", "max us", "min us");
  for (const auto& r : rows) {
    const Stat& s = r.second;
    LOG->Trace("[CProf] %-52s %8llu %10.2f %9.1f %9llu %9llu",
               r.first.c_str(), (unsigned long long)s.n, s.total / 1000.0,
               s.n ? double(s.total) / double(s.n) : 0.0,
               (unsigned long long)s.max, (unsigned long long)s.min);
  }
  for (const auto& c : counts) {
    LOG->Trace("[CProf] count %-46s %8llu", c.first.c_str(),
               (unsigned long long)c.second);
  }
}

inline void MaybeDump() {
#if !PROFLITE_ENABLED
  return;
#endif
  uint64_t t = Now();
  if (LastDump() == 0) {
    LastDump() = t;
    return;
  }
  if (t - LastDump() >= DumpIntervalUs) Dump("periodic");
}

class Scope {
 public:
  explicit Scope(const char* name) : m_name(name), m_t0(Now()) {}
  explicit Scope(std::string name) : m_name(std::move(name)), m_t0(Now()) {}
  ~Scope() { Add(m_name, Now() - m_t0); }

 private:
  std::string m_name;
  uint64_t m_t0;
};

}  // namespace ProfLite

#if PROFLITE_ENABLED
#define PROFLITE_CAT2(a, b) a##b
#define PROFLITE_CAT(a, b) PROFLITE_CAT2(a, b)
#define PROF_SCOPE(name) \
  ProfLite::Scope PROFLITE_CAT(proflite_scope_, __LINE__)(name)
#define PROF_COUNT(name, n) ProfLite::Count(name, n)
#define PROF_MAYBE_DUMP() ProfLite::MaybeDump()
#else
#define PROF_SCOPE(name) \
  do {                   \
  } while (0)
#define PROF_COUNT(name, n) \
  do {                      \
  } while (0)
#define PROF_MAYBE_DUMP() \
  do {                    \
  } while (0)
#endif
