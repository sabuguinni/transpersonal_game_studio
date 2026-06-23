// PerformanceBudget.cpp
// Performance Optimizer — Agent #04
// Implementation of UPerf_BudgetConfig runtime query methods.

#include "PerformanceBudget.h"

UPerf_BudgetConfig::UPerf_BudgetConfig()
{
    // All defaults set via UPROPERTY initializers in header.
    // CDO construction is safe — no world pointer dereferences.
}

bool UPerf_BudgetConfig::IsWithinStaticMeshBudget(int32 CurrentCount) const
{
    return CurrentCount < MaxStaticMeshActors;
}

bool UPerf_BudgetConfig::IsWithinDinoBudget(int32 CurrentCount) const
{
    return CurrentCount < MaxActiveDinoPawns;
}

float UPerf_BudgetConfig::GetRecommendedTickInterval(const FString& SystemName) const
{
    if (SystemName == TEXT("Survival"))  return SurvivalTickInterval;
    if (SystemName == TEXT("Biome"))     return BiomeTickInterval;
    if (SystemName == TEXT("DinoAI"))    return DinoAITickInterval;
    if (SystemName == TEXT("Crowd"))     return PERF_CROWD_TICK_INTERVAL_S;
    if (SystemName == TEXT("Weather"))   return PERF_WEATHER_TICK_INTERVAL_S;

    // Unknown system — return conservative default
    return 1.0f;
}
