#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * Performance targets and budgets for the Transpersonal Game Studio project
 * Based on Jurassic Park survival game concept with massive dinosaur ecosystems
 * 
 * Target: 60fps PC (16.67ms) / 30fps Console (33.33ms)
 * World scale: Regional (large open world with streaming)
 * Key challenge: Mass AI simulation (up to 50,000 agents)
 */

UENUM(BlueprintType)
enum class EPerformanceTarget : uint8
{
    PC_HighEnd = 0,     // 60fps - 16.67ms budget
    Console_NextGen,    // 30fps - 33.33ms budget
    Console_Current,    // 30fps - 33.33ms budget (more aggressive culling)
    Mobile_High         // 30fps - 33.33ms budget (future consideration)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceBudget
{
    GENERATED_BODY()

    // Frame time budgets in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float TotalFrameTime = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float GameThreadBudget = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float RenderThreadBudget = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Budget")
    float GPUBudget = 14.0f;

    // Specific system budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Budgets")
    float MassAIBudget = 3.0f;          // Critical: 50k dinosaur simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Budgets")
    float PhysicsBudget = 2.0f;         // Ragdoll + destruction

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Budgets")
    float RenderingBudget = 10.0f;      // Nanite + Lumen + VSM

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Budgets")
    float AudioBudget = 1.0f;           // MetaSounds + 3D audio

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Budgets")
    float StreamingBudget = 0.5f;       // World Partition streaming

    // Memory budgets (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 TotalMemoryBudget = 12288;    // 12GB for PC, 8GB for console

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 TextureMemoryBudget = 4096;   // 4GB texture streaming pool

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 MeshMemoryBudget = 2048;      // 2GB for Nanite geometry

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 AudioMemoryBudget = 512;      // 512MB for audio

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Budget")
    int32 AIMemoryBudget = 1024;        // 1GB for Mass AI data

    FPerformanceBudget()
    {
        // Default to PC high-end targets
    }

    static FPerformanceBudget GetBudgetForTarget(EPerformanceTarget Target)
    {
        FPerformanceBudget Budget;
        
        switch (Target)
        {
        case EPerformanceTarget::PC_HighEnd:
            Budget.TotalFrameTime = 16.67f;
            Budget.GameThreadBudget = 8.0f;
            Budget.RenderThreadBudget = 12.0f;
            Budget.GPUBudget = 14.0f;
            Budget.TotalMemoryBudget = 12288;
            break;
            
        case EPerformanceTarget::Console_NextGen:
            Budget.TotalFrameTime = 33.33f;
            Budget.GameThreadBudget = 15.0f;
            Budget.RenderThreadBudget = 25.0f;
            Budget.GPUBudget = 28.0f;
            Budget.TotalMemoryBudget = 8192;
            Budget.MassAIBudget = 5.0f;  // More time available
            break;
            
        case EPerformanceTarget::Console_Current:
            Budget.TotalFrameTime = 33.33f;
            Budget.GameThreadBudget = 18.0f;
            Budget.RenderThreadBudget = 28.0f;
            Budget.GPUBudget = 30.0f;
            Budget.TotalMemoryBudget = 6144;
            Budget.MassAIBudget = 6.0f;
            Budget.TextureMemoryBudget = 2048;  // Reduced
            Budget.MeshMemoryBudget = 1024;     // Reduced
            break;
        }
        
        return Budget;
    }
};

// Critical performance constants for the dinosaur survival game
namespace PerformanceConstants
{
    // Rendering limits
    static constexpr int32 MAX_DRAW_CALLS_PER_FRAME = 5000;
    static constexpr int32 MAX_TRIANGLES_PER_FRAME = 50000000;  // 50M with Nanite
    static constexpr int32 MAX_INSTANCES_PER_FRAME = 100000;
    
    // Mass AI limits (critical for dinosaur ecosystem)
    static constexpr int32 MAX_MASS_AGENTS = 50000;
    static constexpr int32 MAX_BEHAVIOR_TREES_ACTIVE = 1000;
    static constexpr int32 MAX_PERCEPTION_QUERIES_PER_FRAME = 5000;
    
    // Streaming limits
    static constexpr float WORLD_PARTITION_CELL_SIZE = 12800.0f;  // 128m cells
    static constexpr int32 MAX_STREAMING_CELLS_LOADED = 25;
    static constexpr float TEXTURE_STREAMING_POOL_SIZE_MB = 4096.0f;
    
    // Physics limits
    static constexpr int32 MAX_PHYSICS_BODIES = 10000;
    static constexpr int32 MAX_DESTRUCTION_CHUNKS = 5000;
    static constexpr float PHYSICS_TICK_RATE = 60.0f;
    
    // Audio limits
    static constexpr int32 MAX_AUDIO_SOURCES = 256;
    static constexpr int32 MAX_3D_AUDIO_SOURCES = 64;
    
    // Culling distances (Unreal Units)
    static constexpr float SMALL_OBJECT_CULL_DISTANCE = 5000.0f;
    static constexpr float MEDIUM_OBJECT_CULL_DISTANCE = 15000.0f;
    static constexpr float LARGE_OBJECT_CULL_DISTANCE = 50000.0f;
    static constexpr float DINOSAUR_CULL_DISTANCE = 25000.0f;
}