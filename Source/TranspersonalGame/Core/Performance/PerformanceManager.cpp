#include "PerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/GameUserSettings.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogPerformance);

APerformanceManager::APerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // 60fps tick rate
    
    CurrentTarget = EPerformanceTarget::PC_60FPS;
    LastMetricsUpdate = 0.0f;
    
    // Initialize default budgets for 60fps PC
    PerformanceBudget.TargetFrameTime = 16.67f;
    PerformanceBudget.MaxGameThreadTime = 8.0f;
    PerformanceBudget.MaxRenderThreadTime = 12.0f;
    PerformanceBudget.MaxGPUTime = 14.0f;
    PerformanceBudget.MaxDrawCalls = 2000;
    PerformanceBudget.MaxVisiblePrimitives = 15000;
    PerformanceBudget.MaxMemoryUsageMB = 3000.0f;
    PerformanceBudget.MaxActiveAIAgents = 200;
}

void APerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPerformance, Log, TEXT("Performance Manager initialized for target: %s"), 
           CurrentTarget == EPerformanceTarget::PC_60FPS ? TEXT("PC 60FPS") : 
           CurrentTarget == EPerformanceTarget::Console_30FPS ? TEXT("Console 30FPS") : TEXT("Mobile 30FPS"));
    
    // Auto-detect platform and set appropriate target
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    SetPerformanceTarget(EPerformanceTarget::PC_60FPS);
#elif PLATFORM_PS5 || PLATFORM_XBOXONE || PLATFORM_SWITCH
    SetPerformanceTarget(EPerformanceTarget::Console_30FPS);
#else
    SetPerformanceTarget(EPerformanceTarget::Mobile_30FPS);
#endif

    FrameTimeHistory.Reserve(FrameHistorySize);
}

void APerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update metrics at specified interval
    if (CurrentTime - LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        CheckPerformanceBudgets();
        
        if (bEnableAdaptiveQuality)
        {
            ApplyAdaptiveQuality();
        }
        
        LastMetricsUpdate = CurrentTime;
    }
}

void APerformanceManager::UpdatePerformanceMetrics()
{
    // Get frame timing stats
    CurrentMetrics.FrameTime = FPlatformTime::ToMilliseconds(GFrameTime) * 1000.0f;
    
    // Track frame time history
    FrameTimeHistory.Add(CurrentMetrics.FrameTime);
    if (FrameTimeHistory.Num() > FrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Get thread timing from stats system
    if (FThreadStats::IsCollectingData())
    {
        CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime) * 1000.0f;
        CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime) * 1000.0f;
        CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime) * 1000.0f;
    }
    
    // Get rendering stats
    CurrentMetrics.DrawCalls = GNumDrawCallsRHI[0];
    CurrentMetrics.VisiblePrimitives = GNumPrimitivesDrawnRHI[0];
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count active AI agents (simplified - would need proper AI system integration)
    CurrentMetrics.ActiveAIAgents = 0; // TODO: Integrate with AI system
    
    // Count streaming textures
    CurrentMetrics.StreamingTextures = 0; // TODO: Integrate with texture streaming system
}

void APerformanceManager::CheckPerformanceBudgets()
{
    bool bBudgetExceeded = false;
    
    if (CurrentMetrics.FrameTime > PerformanceBudget.TargetFrameTime)
    {
        UE_LOG(LogPerformance, Warning, TEXT("Frame time budget exceeded: %.2fms > %.2fms"), 
               CurrentMetrics.FrameTime, PerformanceBudget.TargetFrameTime);
        bBudgetExceeded = true;
    }
    
    if (CurrentMetrics.GameThreadTime > PerformanceBudget.MaxGameThreadTime)
    {
        UE_LOG(LogPerformance, Warning, TEXT("Game thread budget exceeded: %.2fms > %.2fms"), 
               CurrentMetrics.GameThreadTime, PerformanceBudget.MaxGameThreadTime);
        bBudgetExceeded = true;
    }
    
    if (CurrentMetrics.RenderThreadTime > PerformanceBudget.MaxRenderThreadTime)
    {
        UE_LOG(LogPerformance, Warning, TEXT("Render thread budget exceeded: %.2fms > %.2fms"), 
               CurrentMetrics.RenderThreadTime, PerformanceBudget.MaxRenderThreadTime);
        bBudgetExceeded = true;
    }
    
    if (CurrentMetrics.GPUTime > PerformanceBudget.MaxGPUTime)
    {
        UE_LOG(LogPerformance, Warning, TEXT("GPU budget exceeded: %.2fms > %.2fms"), 
               CurrentMetrics.GPUTime, PerformanceBudget.MaxGPUTime);
        bBudgetExceeded = true;
    }
    
    if (CurrentMetrics.DrawCalls > PerformanceBudget.MaxDrawCalls)
    {
        UE_LOG(LogPerformance, Warning, TEXT("Draw calls budget exceeded: %d > %d"), 
               CurrentMetrics.DrawCalls, PerformanceBudget.MaxDrawCalls);
        bBudgetExceeded = true;
    }
    
    if (bBudgetExceeded)
    {
        OnBudgetExceeded(CurrentMetrics);
    }
}

void APerformanceManager::ApplyAdaptiveQuality()
{
    float PerformanceRatio = GetPerformanceRatio();
    
    // If performance is below 85% of target, start reducing quality
    if (PerformanceRatio < 0.85f)
    {
        UE_LOG(LogPerformance, Log, TEXT("Applying adaptive quality reduction. Performance ratio: %.2f"), PerformanceRatio);
        
        // Reduce dinosaur LOD distance
        AdjustDinosaurLOD(PerformanceRatio);
        
        // Reduce vegetation density
        AdjustVegetationDensity(PerformanceRatio);
        
        // Reduce shadow quality
        AdjustShadowQuality(PerformanceRatio);
    }
    // If performance is above 95% of target, we can increase quality
    else if (PerformanceRatio > 0.95f)
    {
        UE_LOG(LogPerformance, VeryVerbose, TEXT("Performance headroom available. Ratio: %.2f"), PerformanceRatio);
        // TODO: Implement quality increase logic
    }
}

void APerformanceManager::SetPerformanceTarget(EPerformanceTarget NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        // Adjust budgets based on target
        switch (NewTarget)
        {
        case EPerformanceTarget::PC_60FPS:
            PerformanceBudget.TargetFrameTime = 16.67f;
            PerformanceBudget.MaxGameThreadTime = 8.0f;
            PerformanceBudget.MaxRenderThreadTime = 12.0f;
            PerformanceBudget.MaxGPUTime = 14.0f;
            PerformanceBudget.MaxDrawCalls = 2000;
            PerformanceBudget.MaxVisiblePrimitives = 15000;
            PerformanceBudget.MaxActiveAIAgents = 200;
            break;
            
        case EPerformanceTarget::Console_30FPS:
            PerformanceBudget.TargetFrameTime = 33.33f;
            PerformanceBudget.MaxGameThreadTime = 16.0f;
            PerformanceBudget.MaxRenderThreadTime = 24.0f;
            PerformanceBudget.MaxGPUTime = 28.0f;
            PerformanceBudget.MaxDrawCalls = 1500;
            PerformanceBudget.MaxVisiblePrimitives = 12000;
            PerformanceBudget.MaxActiveAIAgents = 150;
            break;
            
        case EPerformanceTarget::Mobile_30FPS:
            PerformanceBudget.TargetFrameTime = 33.33f;
            PerformanceBudget.MaxGameThreadTime = 20.0f;
            PerformanceBudget.MaxRenderThreadTime = 28.0f;
            PerformanceBudget.MaxGPUTime = 30.0f;
            PerformanceBudget.MaxDrawCalls = 800;
            PerformanceBudget.MaxVisiblePrimitives = 5000;
            PerformanceBudget.MaxActiveAIAgents = 50;
            break;
        }
        
        UE_LOG(LogPerformance, Log, TEXT("Performance target changed to: %s"), 
               NewTarget == EPerformanceTarget::PC_60FPS ? TEXT("PC 60FPS") : 
               NewTarget == EPerformanceTarget::Console_30FPS ? TEXT("Console 30FPS") : TEXT("Mobile 30FPS"));
        
        OnPerformanceTargetChanged(NewTarget);
    }
}

bool APerformanceManager::IsWithinBudget() const
{
    return CurrentMetrics.FrameTime <= PerformanceBudget.TargetFrameTime &&
           CurrentMetrics.GameThreadTime <= PerformanceBudget.MaxGameThreadTime &&
           CurrentMetrics.RenderThreadTime <= PerformanceBudget.MaxRenderThreadTime &&
           CurrentMetrics.GPUTime <= PerformanceBudget.MaxGPUTime &&
           CurrentMetrics.DrawCalls <= PerformanceBudget.MaxDrawCalls &&
           CurrentMetrics.VisiblePrimitives <= PerformanceBudget.MaxVisiblePrimitives;
}

float APerformanceManager::GetPerformanceRatio() const
{
    if (PerformanceBudget.TargetFrameTime <= 0.0f)
        return 1.0f;
        
    return PerformanceBudget.TargetFrameTime / FMath::Max(CurrentMetrics.FrameTime, 0.1f);
}

void APerformanceManager::AdjustDinosaurLOD(float PerformanceRatio)
{
    // Reduce LOD distance for dinosaurs based on performance
    float LODScale = FMath::Clamp(PerformanceRatio * 1.2f, 0.5f, 1.0f);
    
    // This would integrate with the dinosaur system
    // For now, we'll use console variables
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ViewDistanceScale %f"), LODScale));
    }
    
    UE_LOG(LogPerformance, Log, TEXT("Adjusted dinosaur LOD scale to: %.2f"), LODScale);
}

void APerformanceManager::AdjustVegetationDensity(float PerformanceRatio)
{
    // Reduce foliage density based on performance
    float DensityScale = FMath::Clamp(PerformanceRatio * 1.1f, 0.3f, 1.0f);
    
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("foliage.DensityScale %f"), DensityScale));
    }
    
    UE_LOG(LogPerformance, Log, TEXT("Adjusted vegetation density to: %.2f"), DensityScale);
}

void APerformanceManager::AdjustShadowQuality(float PerformanceRatio)
{
    // Reduce shadow quality based on performance
    int32 ShadowQuality = FMath::Clamp(FMath::FloorToInt(PerformanceRatio * 4.0f), 0, 3);
    
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("sg.ShadowQuality %d"), ShadowQuality));
    }
    
    UE_LOG(LogPerformance, Log, TEXT("Adjusted shadow quality to: %d"), ShadowQuality);
}