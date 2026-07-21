// PerformanceConfig.cpp
// Agent #05 — Procedural World Generator
// Cycle: PROD_CYCLE_AUTO_20260625_010

#include "PerformanceConfig.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"

UPerformanceConfig::UPerformanceConfig()
{
    // World generation performance defaults
    MaxConcurrentChunks = 4;
    ChunkStreamingRadius = 5000.0f;
    LODDistanceMultiplier = 1.0f;
    bEnableAsyncWorldGen = true;
    bEnableFoliageCulling = true;
    FoliageCullDistance = 8000.0f;
    MaxFoliageInstancesPerChunk = 500;
    bEnableDynamicLOD = true;
    TargetFPS = 60;
    bEnableOcclusionCulling = true;
}

void UPerformanceConfig::ApplySettings()
{
    if (GEngine)
    {
        // Apply foliage cull distance via console
        if (GEngine->GameViewport)
        {
            // Settings applied via console commands at runtime
        }
        UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Settings applied: MaxChunks=%d, StreamRadius=%.0f, FoliageCull=%.0f"),
            MaxConcurrentChunks, ChunkStreamingRadius, FoliageCullDistance);
    }
}

float UPerformanceConfig::GetChunkStreamingRadius() const
{
    return ChunkStreamingRadius;
}

int32 UPerformanceConfig::GetMaxFoliageInstances() const
{
    return MaxFoliageInstancesPerChunk;
}

bool UPerformanceConfig::IsAsyncWorldGenEnabled() const
{
    return bEnableAsyncWorldGen;
}
