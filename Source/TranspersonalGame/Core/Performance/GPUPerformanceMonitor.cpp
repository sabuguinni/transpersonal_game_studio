// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "GPUPerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/StatsData.h"
#include "RenderingThread.h"
#include "RHIStats.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogGPUPerformance);

void UGPUPerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogGPUPerformance, Log, TEXT("GPU Performance Monitor initialized"));
    
    // Detect platform-specific GPU budget
    DetectPlatformGPUBudget();
    
    // Initialize GPU monitoring
    if (bGPUMonitoringEnabled)
    {
        InitializeGPUMonitoring();
    }
}

void UGPUPerformanceMonitor::Deinitialize()
{
    // Clear monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GPUMonitoringTimer);
    }
    
    UE_LOG(LogGPUPerformance, Log, TEXT("GPU Performance Monitor deinitialized"));
    
    Super::Deinitialize();
}

void UGPUPerformanceMonitor::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    if (bGPUMonitoringEnabled)
    {
        // Start GPU monitoring timer
        InWorld.GetTimerManager().SetTimer(
            GPUMonitoringTimer,
            this,
            &UGPUPerformanceMonitor::UpdateGPUMetrics,
            0.1f, // Update every 100ms
            true
        );
    }
}

UGPUPerformanceMonitor* UGPUPerformanceMonitor::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UGPUPerformanceMonitor>();
    }
    return nullptr;
}

void UGPUPerformanceMonitor::InitializeGPUMonitoring()
{
    UE_LOG(LogGPUPerformance, Log, TEXT("Initializing GPU monitoring with budget: %.2fms"), GPUBudgetMS);
    
    // Initialize performance history arrays
    GPUFrameTimeHistory.Reserve(300); // 30 seconds at 10Hz
    VRAMUsageHistory.Reserve(300);
    DrawCallHistory.Reserve(300);
    
    // Clear current metrics
    CurrentGPUMetrics = FGPUPerformanceAnalysis();
    NaniteMetrics = FNanitePerformanceMetrics();
    LumenMetrics = FLumenPerformanceMetrics();
    
    // Force initial GPU statistics gathering
    GatherGPUStatistics();
}

float UGPUPerformanceMonitor::GetGPUFrameTimeMS() const
{
    return CurrentGPUMetrics.GPUFrameTimeMS;
}

float UGPUPerformanceMonitor::GetVRAMUsageMB() const
{
    return CurrentGPUMetrics.VRAMUsageMB;
}

int32 UGPUPerformanceMonitor::GetDrawCallCount() const
{
    return CurrentGPUMetrics.DrawCalls;
}

int32 UGPUPerformanceMonitor::GetTriangleCount() const
{
    return CurrentGPUMetrics.Triangles;
}

bool UGPUPerformanceMonitor::IsGPUWithinBudget() const
{
    return CurrentGPUMetrics.BudgetUsagePercent <= 100.0f;
}

UGPUPerformanceMonitor::FGPUPerformanceAnalysis UGPUPerformanceMonitor::AnalyzeGPUPerformance()
{
    // Update current metrics
    UpdateGPUMetrics();
    
    // Calculate budget usage
    CurrentGPUMetrics.BudgetUsagePercent = CalculateGPUBudgetUsage();
    
    // Clear previous analysis
    CurrentGPUMetrics.Bottlenecks.Empty();
    CurrentGPUMetrics.Recommendations.Empty();
    CurrentGPUMetrics.bEmergencyOptimizationRequired = false;
    
    // Analyze GPU frame time
    if (CurrentGPUMetrics.GPUFrameTimeMS > GPUBudgetMS * (1.0f + EmergencyThreshold / 100.0f))
    {
        CurrentGPUMetrics.bEmergencyOptimizationRequired = true;
        CurrentGPUMetrics.Bottlenecks.Add(TEXT("GPU Frame Time Critical"));
        CurrentGPUMetrics.Recommendations.Add(TEXT("Reduce rendering quality immediately"));
    }
    else if (CurrentGPUMetrics.GPUFrameTimeMS > GPUBudgetMS)
    {
        CurrentGPUMetrics.Bottlenecks.Add(TEXT("GPU Frame Time Over Budget"));
        CurrentGPUMetrics.Recommendations.Add(TEXT("Consider reducing LOD or effects quality"));
    }
    
    // Analyze VRAM usage
    if (CurrentGPUMetrics.VRAMUsageMB > VRAMWarningThresholdMB)
    {
        CurrentGPUMetrics.Bottlenecks.Add(TEXT("High VRAM Usage"));
        CurrentGPUMetrics.Recommendations.Add(TEXT("Reduce texture quality or streaming pool size"));
    }
    
    // Analyze draw calls
    if (CurrentGPUMetrics.DrawCalls > DrawCallWarningThreshold)
    {
        CurrentGPUMetrics.Bottlenecks.Add(TEXT("High Draw Call Count"));
        CurrentGPUMetrics.Recommendations.Add(TEXT("Enable instancing or reduce object count"));
    }
    
    // Analyze triangle count
    if (CurrentGPUMetrics.Triangles > TriangleWarningThreshold)
    {
        CurrentGPUMetrics.Bottlenecks.Add(TEXT("High Triangle Count"));
        CurrentGPUMetrics.Recommendations.Add(TEXT("Use Nanite or reduce mesh complexity"));
    }
    
    // Log warnings if needed
    if (!CurrentGPUMetrics.Bottlenecks.IsEmpty())
    {
        LogGPUPerformanceWarnings(CurrentGPUMetrics);
    }
    
    return CurrentGPUMetrics;
}

void UGPUPerformanceMonitor::ForceGPUOptimization()
{
    UE_LOG(LogGPUPerformance, Warning, TEXT("Force GPU optimization triggered - GPU budget exceeded by %.1f%%"), 
           CurrentGPUMetrics.BudgetUsagePercent - 100.0f);
    
    ApplyGPUOptimizations();
}

void UGPUPerformanceMonitor::SetGPUMonitoring(bool bEnabled)
{
    bGPUMonitoringEnabled = bEnabled;
    
    if (UWorld* World = GetWorld())
    {
        if (bEnabled && !World->GetTimerManager().IsTimerActive(GPUMonitoringTimer))
        {
            World->GetTimerManager().SetTimer(
                GPUMonitoringTimer,
                this,
                &UGPUPerformanceMonitor::UpdateGPUMetrics,
                0.1f,
                true
            );
        }
        else if (!bEnabled)
        {
            World->GetTimerManager().ClearTimer(GPUMonitoringTimer);
        }
    }
    
    UE_LOG(LogGPUPerformance, Log, TEXT("GPU monitoring %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

UGPUPerformanceMonitor::FNanitePerformanceMetrics UGPUPerformanceMonitor::GetNaniteMetrics() const
{
    return NaniteMetrics;
}

UGPUPerformanceMonitor::FLumenPerformanceMetrics UGPUPerformanceMonitor::GetLumenMetrics() const
{
    return LumenMetrics;
}

void UGPUPerformanceMonitor::UpdateGPUMetrics()
{
    if (!bGPUMonitoringEnabled)
    {
        return;
    }
    
    // Gather current GPU statistics
    GatherGPUStatistics();
    
    // Update performance history
    GPUFrameTimeHistory.Add(CurrentGPUMetrics.GPUFrameTimeMS);
    VRAMUsageHistory.Add(CurrentGPUMetrics.VRAMUsageMB);
    DrawCallHistory.Add(CurrentGPUMetrics.DrawCalls);
    
    // Keep history size manageable
    if (GPUFrameTimeHistory.Num() > 300)
    {
        GPUFrameTimeHistory.RemoveAt(0);
        VRAMUsageHistory.RemoveAt(0);
        DrawCallHistory.RemoveAt(0);
    }
    
    // Check if emergency optimization is needed
    if (CalculateGPUBudgetUsage() > 100.0f + EmergencyThreshold)
    {
        ForceGPUOptimization();
    }
}

float UGPUPerformanceMonitor::CalculateGPUBudgetUsage() const
{
    if (GPUBudgetMS <= 0.0f)
    {
        return 0.0f;
    }
    
    return (CurrentGPUMetrics.GPUFrameTimeMS / GPUBudgetMS) * 100.0f;
}

void UGPUPerformanceMonitor::ApplyGPUOptimizations()
{
    // Apply emergency GPU optimizations
    
    // Reduce screen percentage
    if (auto* GameViewport = GEngine->GameViewport)
    {
        static IConsoleVariable* ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
        if (ScreenPercentageCVar)
        {
            float CurrentScreenPercentage = ScreenPercentageCVar->GetFloat();
            float NewScreenPercentage = FMath::Max(50.0f, CurrentScreenPercentage * 0.8f);
            ScreenPercentageCVar->Set(NewScreenPercentage);
            
            UE_LOG(LogGPUPerformance, Warning, TEXT("Reduced screen percentage to %.1f%% for GPU optimization"), NewScreenPercentage);
        }
    }
    
    // Reduce shadow quality
    static IConsoleVariable* ShadowQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    if (ShadowQualityCVar)
    {
        int32 CurrentShadowQuality = ShadowQualityCVar->GetInt();
        int32 NewShadowQuality = FMath::Max(0, CurrentShadowQuality - 1);
        ShadowQualityCVar->Set(NewShadowQuality);
        
        UE_LOG(LogGPUPerformance, Warning, TEXT("Reduced shadow quality to %d for GPU optimization"), NewShadowQuality);
    }
    
    // Reduce effects quality
    static IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    if (EffectsQualityCVar)
    {
        int32 CurrentEffectsQuality = EffectsQualityCVar->GetInt();
        int32 NewEffectsQuality = FMath::Max(0, CurrentEffectsQuality - 1);
        EffectsQualityCVar->Set(NewEffectsQuality);
        
        UE_LOG(LogGPUPerformance, Warning, TEXT("Reduced effects quality to %d for GPU optimization"), NewEffectsQuality);
    }
}

void UGPUPerformanceMonitor::GatherGPUStatistics()
{
    // Get GPU frame time from stats
    float GPUFrameTime = 0.0f;
    if (FStats::IsThreadingReady())
    {
        // Try to get GPU time from render thread stats
        ENQUEUE_RENDER_COMMAND(GetGPUFrameTime)(
            [&GPUFrameTime](FRHICommandListImmediate& RHICmdList)
            {
                // This would need proper RHI implementation
                // For now, use a simple approximation
                GPUFrameTime = FPlatformTime::Seconds() * 1000.0f; // Convert to ms
            });
        FlushRenderingCommands();
    }
    
    // Fallback: estimate from engine stats
    if (GPUFrameTime <= 0.0f)
    {
        if (UEngine* Engine = GEngine)
        {
            // Use frame time as approximation (not ideal but functional)
            GPUFrameTime = Engine->GetMaxTickRate() > 0 ? (1000.0f / Engine->GetMaxTickRate()) : 16.67f;
        }
    }
    
    CurrentGPUMetrics.GPUFrameTimeMS = GPUFrameTime;
    
    // Get VRAM usage (platform-specific)
    float VRAMUsage = 0.0f;
    if (GRHISupportsMemoryInfo)
    {
        FRHIMemoryInfo MemInfo;
        RHIGetMemoryInfo(MemInfo);
        VRAMUsage = MemInfo.TotalGraphicsMemory / (1024.0f * 1024.0f); // Convert to MB
    }
    else
    {
        // Fallback estimation
        VRAMUsage = 2048.0f; // 2GB estimation
    }
    
    CurrentGPUMetrics.VRAMUsageMB = VRAMUsage;
    
    // Get draw calls and triangles from stats
    CurrentGPUMetrics.DrawCalls = 1000; // Placeholder - would need proper stat gathering
    CurrentGPUMetrics.Triangles = 500000; // Placeholder - would need proper stat gathering
    
    // Update Nanite metrics
    NaniteMetrics.NaniteTriangles = 250000; // Placeholder
    NaniteMetrics.NaniteClusters = 5000; // Placeholder
    NaniteMetrics.NaniteGPUTimeMS = GPUFrameTime * 0.3f; // Estimate 30% of GPU time
    NaniteMetrics.NaniteMemoryMB = VRAMUsage * 0.4f; // Estimate 40% of VRAM
    NaniteMetrics.CullingEfficiency = 0.85f; // 85% efficiency
    
    // Update Lumen metrics
    LumenMetrics.LumenGITimeMS = GPUFrameTime * 0.2f; // Estimate 20% of GPU time
    LumenMetrics.LumenReflectionsTimeMS = GPUFrameTime * 0.1f; // Estimate 10% of GPU time
    LumenMetrics.SurfaceCacheMemoryMB = VRAMUsage * 0.15f; // Estimate 15% of VRAM
    LumenMetrics.LumenRayCount = 100000; // Placeholder
    LumenMetrics.bHardwareRayTracingEnabled = GRHISupportsRayTracing;
}

void UGPUPerformanceMonitor::DetectPlatformGPUBudget()
{
    // Detect platform and set appropriate GPU budget
    FString PlatformName = UGameplayStatics::GetPlatformName();
    
    if (PlatformName.Contains(TEXT("Windows")) || PlatformName.Contains(TEXT("Mac")) || PlatformName.Contains(TEXT("Linux")))
    {
        // PC platforms - target 60fps
        GPUBudgetMS = 16.67f;
        VRAMWarningThresholdMB = 6000.0f; // 6GB
        UE_LOG(LogGPUPerformance, Log, TEXT("PC platform detected - GPU budget set to %.2fms (60fps)"), GPUBudgetMS);
    }
    else
    {
        // Console platforms - target 30fps
        GPUBudgetMS = 33.33f;
        VRAMWarningThresholdMB = 4000.0f; // 4GB for consoles
        UE_LOG(LogGPUPerformance, Log, TEXT("Console platform detected - GPU budget set to %.2fms (30fps)"), GPUBudgetMS);
    }
}

void UGPUPerformanceMonitor::LogGPUPerformanceWarnings(const FGPUPerformanceAnalysis& Analysis)
{
    for (const FString& Bottleneck : Analysis.Bottlenecks)
    {
        UE_LOG(LogGPUPerformance, Warning, TEXT("GPU Performance Bottleneck: %s"), *Bottleneck);
    }
    
    for (const FString& Recommendation : Analysis.Recommendations)
    {
        UE_LOG(LogGPUPerformance, Log, TEXT("GPU Performance Recommendation: %s"), *Recommendation);
    }
    
    if (Analysis.bEmergencyOptimizationRequired)
    {
        UE_LOG(LogGPUPerformance, Error, TEXT("EMERGENCY GPU OPTIMIZATION REQUIRED - Performance critically degraded!"));
    }
}

// GPU Budget Enforcer Component Implementation

UGPUBudgetEnforcerComponent::UGPUBudgetEnforcerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.2f; // Check every 200ms
}

void UGPUBudgetEnforcerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogGPUPerformance, Log, TEXT("GPU Budget Enforcer started for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UGPUBudgetEnforcerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoEnforceGPUBudget)
    {
        float BudgetUsage = GetGPUBudgetUsagePercent();
        if (BudgetUsage > 100.0f + GPUViolationThreshold)
        {
            ForceGPUBudgetCompliance();
        }
    }
}

void UGPUBudgetEnforcerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogGPUPerformance, Log, TEXT("GPU Budget Enforcer stopped for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    Super::EndPlay(EndPlayReason);
}

bool UGPUBudgetEnforcerComponent::IsGPUWithinBudget() const
{
    if (UGPUPerformanceMonitor* Monitor = UGPUPerformanceMonitor::Get(this))
    {
        return Monitor->IsGPUWithinBudget();
    }
    return true;
}

float UGPUBudgetEnforcerComponent::GetGPUBudgetUsagePercent() const
{
    if (UGPUPerformanceMonitor* Monitor = UGPUPerformanceMonitor::Get(this))
    {
        FGPUPerformanceAnalysis Analysis = Monitor->AnalyzeGPUPerformance();
        return Analysis.BudgetUsagePercent;
    }
    return 0.0f;
}

void UGPUBudgetEnforcerComponent::ForceGPUBudgetCompliance()
{
    UE_LOG(LogGPUPerformance, Warning, TEXT("Forcing GPU budget compliance for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // Apply LOD bias
    if (LODBiasSteps.Num() > 0)
    {
        float BudgetUsage = GetGPUBudgetUsagePercent();
        int32 BiasIndex = FMath::Clamp(FMath::FloorToInt((BudgetUsage - 100.0f) / 25.0f), 0, LODBiasSteps.Num() - 1);
        SetLODBias(LODBiasSteps[BiasIndex]);
    }
    
    // Reduce texture quality
    float BudgetOverage = GetGPUBudgetUsagePercent() - 100.0f;
    if (BudgetOverage > 0.0f)
    {
        float TextureScale = FMath::Max(0.5f, 1.0f - (BudgetOverage / 100.0f));
        SetTextureQualityScale(TextureScale);
    }
}

void UGPUBudgetEnforcerComponent::SetLODBias(float Bias)
{
    if (AActor* Owner = GetOwner())
    {
        // Apply LOD bias to all static mesh components
        TArray<UStaticMeshComponent*> MeshComponents;
        Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp)
            {
                MeshComp->SetForcedLodModel(FMath::FloorToInt(Bias) + 1);
            }
        }
        
        UE_LOG(LogGPUPerformance, Log, TEXT("Set LOD bias to %.1f for actor: %s"), Bias, *Owner->GetName());
    }
}

void UGPUBudgetEnforcerComponent::SetTextureQualityScale(float Scale)
{
    // This would require more complex material parameter manipulation
    // For now, just log the intention
    UE_LOG(LogGPUPerformance, Log, TEXT("Set texture quality scale to %.2f for actor: %s"), 
           Scale, GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}