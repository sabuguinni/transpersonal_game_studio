#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceManager, Log, All);

UPerformanceManager::UPerformanceManager()
{
    CurrentTarget = EPerformanceTarget::PC_HighEnd_60FPS;
    CurrentLevel = EPerformanceLevel::High;
    bEnableDynamicQualityAdjustment = true;
    bShowPerformanceHUD = false;
    PerformanceUpdateInterval = 0.1f;
    
    FrameTimeHistory.Reserve(HistorySize);
    GPUTimeHistory.Reserve(HistorySize);
}

void UPerformanceManager::Initialize(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    
    // Set performance budgets based on target platform
    switch (Target)
    {
        case EPerformanceTarget::PC_HighEnd_60FPS:
            CurrentBudget.TargetFrameTime = 16.67f; // 60fps
            CurrentBudget.GameThreadBudget = 8.0f;
            CurrentBudget.RenderThreadBudget = 12.0f;
            CurrentBudget.GPUBudget = 14.0f;
            CurrentBudget.MaxDrawCalls = 3000;
            CurrentBudget.MaxTriangles = 3000000;
            CurrentBudget.TextureMemoryBudget = 4096.0f; // 4GB
            CurrentBudget.MeshMemoryBudget = 2048.0f; // 2GB
            SetPerformanceLevel(EPerformanceLevel::Epic);
            break;
            
        case EPerformanceTarget::Console_30FPS:
            CurrentBudget.TargetFrameTime = 33.33f; // 30fps
            CurrentBudget.GameThreadBudget = 16.0f;
            CurrentBudget.RenderThreadBudget = 24.0f;
            CurrentBudget.GPUBudget = 28.0f;
            CurrentBudget.MaxDrawCalls = 2000;
            CurrentBudget.MaxTriangles = 2000000;
            CurrentBudget.TextureMemoryBudget = 2048.0f; // 2GB
            CurrentBudget.MeshMemoryBudget = 1024.0f; // 1GB
            SetPerformanceLevel(EPerformanceLevel::High);
            break;
            
        case EPerformanceTarget::PC_MidRange_30FPS:
            CurrentBudget.TargetFrameTime = 33.33f; // 30fps
            CurrentBudget.GameThreadBudget = 18.0f;
            CurrentBudget.RenderThreadBudget = 26.0f;
            CurrentBudget.GPUBudget = 30.0f;
            CurrentBudget.MaxDrawCalls = 1500;
            CurrentBudget.MaxTriangles = 1500000;
            CurrentBudget.TextureMemoryBudget = 1536.0f; // 1.5GB
            CurrentBudget.MeshMemoryBudget = 768.0f; // 768MB
            SetPerformanceLevel(EPerformanceLevel::Medium);
            break;
            
        case EPerformanceTarget::Mobile_30FPS:
            CurrentBudget.TargetFrameTime = 33.33f; // 30fps
            CurrentBudget.GameThreadBudget = 20.0f;
            CurrentBudget.RenderThreadBudget = 28.0f;
            CurrentBudget.GPUBudget = 30.0f;
            CurrentBudget.MaxDrawCalls = 800;
            CurrentBudget.MaxTriangles = 500000;
            CurrentBudget.TextureMemoryBudget = 512.0f; // 512MB
            CurrentBudget.MeshMemoryBudget = 256.0f; // 256MB
            SetPerformanceLevel(EPerformanceLevel::Low);
            break;
    }
    
    ApplyPerformanceBudget();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager initialized for target: %d"), (int32)Target);
    UE_LOG(LogPerformanceManager, Log, TEXT("Target frame time: %.2fms (%.1f fps)"), 
           CurrentBudget.TargetFrameTime, 1000.0f / CurrentBudget.TargetFrameTime);
}

void UPerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    CurrentLevel = Level;
    UpdateScalabilitySettings();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance level set to: %d"), (int32)Level);
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    if (!GEngine || !GEngine->GetWorld())
    {
        return;
    }

    FDateTime Now = FDateTime::Now();
    if ((Now - LastPerformanceUpdate).GetTotalSeconds() < PerformanceUpdateInterval)
    {
        return;
    }
    LastPerformanceUpdate = Now;

    // Get frame time metrics
    float DeltaTime = GEngine->GetWorld()->GetDeltaSeconds();
    CurrentMetrics.CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;

    // Update frame time history
    FrameTimeHistory.Add(CurrentMetrics.CurrentFrameTime);
    if (FrameTimeHistory.Num() > HistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Collect rendering and memory metrics
    CollectRenderingMetrics();
    CollectMemoryMetrics();

    // Check if we're within budget
    CheckPerformanceThresholds();

    // Auto-adjust quality if enabled and needed
    if (bEnableDynamicQualityAdjustment && !CurrentMetrics.bIsWithinBudget)
    {
        AdjustQualityForPerformance();
    }
}

bool UPerformanceManager::IsWithinPerformanceBudget() const
{
    return CurrentMetrics.bIsWithinBudget;
}

void UPerformanceManager::AdjustQualityForPerformance()
{
    if (CurrentMetrics.CurrentFrameTime > CurrentBudget.TargetFrameTime * 1.2f) // 20% over budget
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("Performance over budget (%.2fms vs %.2fms target). Adjusting quality."),
               CurrentMetrics.CurrentFrameTime, CurrentBudget.TargetFrameTime);

        // Progressive quality reduction
        if (CurrentMetrics.GPUTime > CurrentBudget.GPUBudget)
        {
            // GPU bound - reduce visual quality
            AdjustShadowQuality(FMath::Max(0, (int32)CurrentLevel - 1));
            AdjustEffectsQuality(FMath::Max(0, (int32)CurrentLevel - 1));
            SetDynamicResolutionScale(0.9f); // Reduce resolution by 10%
        }
        else if (CurrentMetrics.GameThreadTime > CurrentBudget.GameThreadBudget)
        {
            // Game thread bound - reduce simulation complexity
            AdjustViewDistance(0.8f); // Reduce view distance by 20%
        }
        else if (CurrentMetrics.RenderThreadTime > CurrentBudget.RenderThreadBudget)
        {
            // Render thread bound - reduce draw calls
            AdjustViewDistance(0.85f);
            AdjustTextureQuality(FMath::Max(0, (int32)CurrentLevel - 1));
        }
    }
    else if (CurrentMetrics.CurrentFrameTime < CurrentBudget.TargetFrameTime * 0.8f) // 20% under budget
    {
        // We have performance headroom - can increase quality
        if ((int32)CurrentLevel < (int32)EPerformanceLevel::Epic)
        {
            UE_LOG(LogPerformanceManager, Log, TEXT("Performance under budget. Considering quality increase."));
            // Cautiously increase quality
        }
    }
}

void UPerformanceManager::SetDynamicResolutionScale(float Scale)
{
    Scale = FMath::Clamp(Scale, 0.5f, 1.0f);
    
    if (GEngine && GEngine->GameViewport)
    {
        // Apply dynamic resolution scaling
        static IConsoleVariable* CVarScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
        if (CVarScreenPercentage)
        {
            float CurrentScale = CVarScreenPercentage->GetFloat() / 100.0f;
            float NewScale = FMath::Clamp(CurrentScale * Scale, 50.0f, 100.0f);
            CVarScreenPercentage->Set(NewScale);
            
            UE_LOG(LogPerformanceManager, Log, TEXT("Dynamic resolution scale adjusted to %.1f%%"), NewScale);
        }
    }
}

void UPerformanceManager::StartPerformanceCapture(const FString& CaptureName)
{
    if (bIsCapturing)
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("Performance capture already in progress. Stopping previous capture."));
        StopPerformanceCapture();
    }

    CurrentCaptureName = CaptureName;
    bIsCapturing = true;
    
    // Start Unreal Insights capture
    FString Command = FString::Printf(TEXT("stat startfile %s"), *CaptureName);
    GEngine->Exec(GEngine->GetWorld(), *Command);
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Started performance capture: %s"), *CaptureName);
}

void UPerformanceManager::StopPerformanceCapture()
{
    if (!bIsCapturing)
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("No performance capture in progress."));
        return;
    }

    // Stop Unreal Insights capture
    GEngine->Exec(GEngine->GetWorld(), TEXT("stat stopfile"));
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Stopped performance capture: %s"), *CurrentCaptureName);
    
    bIsCapturing = false;
    CurrentCaptureName.Empty();
}

void UPerformanceManager::LogPerformanceReport()
{
    UE_LOG(LogPerformanceManager, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogPerformanceManager, Log, TEXT("Target: %d, Level: %d"), (int32)CurrentTarget, (int32)CurrentLevel);
    UE_LOG(LogPerformanceManager, Log, TEXT("Current FPS: %.1f (Target: %.1f)"), 
           CurrentMetrics.CurrentFPS, 1000.0f / CurrentBudget.TargetFrameTime);
    UE_LOG(LogPerformanceManager, Log, TEXT("Frame Time: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.CurrentFrameTime, CurrentBudget.TargetFrameTime);
    UE_LOG(LogPerformanceManager, Log, TEXT("Game Thread: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.GameThreadTime, CurrentBudget.GameThreadBudget);
    UE_LOG(LogPerformanceManager, Log, TEXT("Render Thread: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget);
    UE_LOG(LogPerformanceManager, Log, TEXT("GPU Time: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.GPUTime, CurrentBudget.GPUBudget);
    UE_LOG(LogPerformanceManager, Log, TEXT("Draw Calls: %d (Budget: %d)"), 
           CurrentMetrics.CurrentDrawCalls, CurrentBudget.MaxDrawCalls);
    UE_LOG(LogPerformanceManager, Log, TEXT("Triangles: %d (Budget: %d)"), 
           CurrentMetrics.CurrentTriangles, CurrentBudget.MaxTriangles);
    UE_LOG(LogPerformanceManager, Log, TEXT("Within Budget: %s"), 
           CurrentMetrics.bIsWithinBudget ? TEXT("YES") : TEXT("NO"));
    
    if (CurrentMetrics.PerformanceWarnings.Num() > 0)
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("Performance Warnings:"));
        for (const FString& Warning : CurrentMetrics.PerformanceWarnings)
        {
            UE_LOG(LogPerformanceManager, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("========================"));
}

void UPerformanceManager::ShowPerformanceStats()
{
    if (GEngine)
    {
        GEngine->Exec(GEngine->GetWorld(), TEXT("stat unit"));
        GEngine->Exec(GEngine->GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GEngine->GetWorld(), TEXT("stat engine"));
    }
}

void UPerformanceManager::SetPerformanceTarget(int32 TargetFPS)
{
    float NewTargetFrameTime = 1000.0f / FMath::Clamp(TargetFPS, 15, 120);
    CurrentBudget.TargetFrameTime = NewTargetFrameTime;
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance target set to %d fps (%.2fms frame time)"), 
           TargetFPS, NewTargetFrameTime);
}

void UPerformanceManager::TogglePerformanceHUD()
{
    bShowPerformanceHUD = !bShowPerformanceHUD;
    
    if (bShowPerformanceHUD)
    {
        ShowPerformanceStats();
    }
    else if (GEngine)
    {
        GEngine->Exec(GEngine->GetWorld(), TEXT("stat none"));
    }
}

void UPerformanceManager::ApplyPerformanceBudget()
{
    // Apply frame rate cap
    if (GEngine)
    {
        float TargetFPS = 1000.0f / CurrentBudget.TargetFrameTime;
        FString Command = FString::Printf(TEXT("t.MaxFPS %.1f"), TargetFPS);
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
    
    UpdateScalabilitySettings();
}

void UPerformanceManager::UpdateScalabilitySettings()
{
    if (!GEngine)
    {
        return;
    }

    UWorld* World = GEngine->GetWorld();
    if (!World)
    {
        return;
    }

    // Apply scalability settings based on performance level
    int32 QualityLevel = (int32)CurrentLevel;
    
    // Set scalability groups
    FString Commands[] = {
        FString::Printf(TEXT("sg.ResolutionQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.ViewDistanceQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.AntiAliasingQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.ShadowQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.PostProcessQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.TextureQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.EffectsQuality %d"), QualityLevel),
        FString::Printf(TEXT("sg.FoliageQuality %d"), QualityLevel)
    };
    
    for (const FString& Command : Commands)
    {
        GEngine->Exec(World, *Command);
    }
}

void UPerformanceManager::CheckPerformanceThresholds()
{
    CurrentMetrics.PerformanceWarnings.Empty();
    CurrentMetrics.bIsWithinBudget = true;

    // Check frame time budget
    if (CurrentMetrics.CurrentFrameTime > CurrentBudget.TargetFrameTime * 1.1f) // 10% tolerance
    {
        CurrentMetrics.bIsWithinBudget = false;
        CurrentMetrics.PerformanceWarnings.Add(
            FString::Printf(TEXT("Frame time over budget: %.2fms vs %.2fms"), 
                          CurrentMetrics.CurrentFrameTime, CurrentBudget.TargetFrameTime));
    }

    // Check thread budgets
    if (CurrentMetrics.GameThreadTime > CurrentBudget.GameThreadBudget)
    {
        CurrentMetrics.bIsWithinBudget = false;
        CurrentMetrics.PerformanceWarnings.Add(
            FString::Printf(TEXT("Game thread over budget: %.2fms vs %.2fms"), 
                          CurrentMetrics.GameThreadTime, CurrentBudget.GameThreadBudget));
    }

    if (CurrentMetrics.RenderThreadTime > CurrentBudget.RenderThreadBudget)
    {
        CurrentMetrics.bIsWithinBudget = false;
        CurrentMetrics.PerformanceWarnings.Add(
            FString::Printf(TEXT("Render thread over budget: %.2fms vs %.2fms"), 
                          CurrentMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget));
    }

    if (CurrentMetrics.GPUTime > CurrentBudget.GPUBudget)
    {
        CurrentMetrics.bIsWithinBudget = false;
        CurrentMetrics.PerformanceWarnings.Add(
            FString::Printf(TEXT("GPU time over budget: %.2fms vs %.2fms"), 
                          CurrentMetrics.GPUTime, CurrentBudget.GPUBudget));
    }

    // Check draw call budget
    if (CurrentMetrics.CurrentDrawCalls > CurrentBudget.MaxDrawCalls)
    {
        CurrentMetrics.bIsWithinBudget = false;
        CurrentMetrics.PerformanceWarnings.Add(
            FString::Printf(TEXT("Draw calls over budget: %d vs %d"), 
                          CurrentMetrics.CurrentDrawCalls, CurrentBudget.MaxDrawCalls));
    }
}

void UPerformanceManager::CollectRenderingMetrics()
{
    // These would be populated by actual engine stats in a real implementation
    // For now, we'll use placeholder values that would come from the rendering system
    
    // In a real implementation, these would come from:
    // - RenderThread stats for draw calls and triangles
    // - GPU profiler for GPU timing
    // - Engine stats for various rendering metrics
    
    CurrentMetrics.CurrentDrawCalls = 1500; // Placeholder
    CurrentMetrics.CurrentTriangles = 1800000; // Placeholder
    CurrentMetrics.CurrentInstances = 8500; // Placeholder
    
    // These would come from actual profiling
    CurrentMetrics.GameThreadTime = 12.5f; // Placeholder
    CurrentMetrics.RenderThreadTime = 15.2f; // Placeholder
    CurrentMetrics.GPUTime = 13.8f; // Placeholder
}

void UPerformanceManager::CollectMemoryMetrics()
{
    // Memory metrics would be collected from the engine's memory tracking systems
    CurrentMetrics.UsedTextureMemory = 1800.0f; // Placeholder - MB
    CurrentMetrics.UsedMeshMemory = 950.0f; // Placeholder - MB
    CurrentMetrics.UsedAudioMemory = 280.0f; // Placeholder - MB
}

void UPerformanceManager::AdjustViewDistance(float Multiplier)
{
    if (GEngine)
    {
        FString Command = FString::Printf(TEXT("r.ViewDistanceScale %.2f"), Multiplier);
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
}

void UPerformanceManager::AdjustShadowQuality(int32 Quality)
{
    if (GEngine)
    {
        FString Command = FString::Printf(TEXT("sg.ShadowQuality %d"), FMath::Clamp(Quality, 0, 3));
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
}

void UPerformanceManager::AdjustTextureQuality(int32 Quality)
{
    if (GEngine)
    {
        FString Command = FString::Printf(TEXT("sg.TextureQuality %d"), FMath::Clamp(Quality, 0, 3));
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
}

void UPerformanceManager::AdjustEffectsQuality(int32 Quality)
{
    if (GEngine)
    {
        FString Command = FString::Printf(TEXT("sg.EffectsQuality %d"), FMath::Clamp(Quality, 0, 3));
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
}

void UPerformanceManager::AdjustPostProcessQuality(int32 Quality)
{
    if (GEngine)
    {
        FString Command = FString::Printf(TEXT("sg.PostProcessQuality %d"), FMath::Clamp(Quality, 0, 3));
        GEngine->Exec(GEngine->GetWorld(), *Command);
    }
}