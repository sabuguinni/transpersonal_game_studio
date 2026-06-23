// PerformanceBudgetConfig.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260623_007
// Translation unit for PerformanceBudgetConfig.
// All constants are defined in the header as constexpr — this file satisfies
// the mandatory .h/.cpp pairing rule and provides a home for any future
// runtime validation functions.

#include "PerformanceBudgetConfig.h"

// All values are compile-time constexpr constants defined in the header.
// No runtime initialisation required.
//
// Console variable overrides applied at startup via UE5 Python (see
// Docs/Performance/perf_cycle_007_report.md for the full command list):
//
//   r.Shadow.CSM.MaxCascades        3
//   r.Shadow.RadiusThreshold        0.03
//   r.Shadow.DistanceScale          0.7
//   r.Streaming.PoolSize            1024
//   r.HZBOcclusion                  1
//   r.ViewDistanceScale             1.5
//   r.TSR.History.ScreenPercentage  200
//   foliage.LODDistanceScale        1.0
//   fx.Budget.Enabled               1
//   fx.Budget.MaxCPUTimeMS          2.0
//   fx.Budget.MaxGPUTimeMS          2.0
//   a.URO.Enable                    1
//   a.ParallelAnimEvaluation        1
//   p.MaxSubsteps                   4
//   p.MaxPhysicsDeltaTime           0.033
//   gc.TimeBetweenPurgingPendingKillObjects 60
