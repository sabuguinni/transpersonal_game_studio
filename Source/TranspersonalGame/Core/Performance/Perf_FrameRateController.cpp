#include "Perf_FrameRateController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "RHI.h"

UPerf_FrameRateController::UPerf_FrameRateController()
{
    CurrentFPS = 0.0f;
    AverageFPS = 0.0f;
    FrameTimeVariance = 0.0f;
    bIsFrameRateStable = false;
    LastFrameTime = 0.0f;
    FrameCounter = 0;
    
    FrameTimeHistory.Reserve(MaxFrameTimeHistory);
}

void UPerf_FrameRateController::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Initializing frame rate control system"));
    
    // Apply default settings
    ApplyTargetFrameRate();
    
    // Start frame rate monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FrameRateUpdateTimer,
            this,
            &UPerf_FrameRateController::UpdateFrameRateStats,
            0.1f, // Update every 100ms
            true
        );
    }
}

void UPerf_FrameRateController::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(FrameRateUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UPerf_FrameRateController::SetTargetFrameRate(EPerf_FrameRateTarget NewTarget)
{
    CurrentSettings.TargetFrameRate = NewTarget;
    ApplyTargetFrameRate();
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Target frame rate changed to %d"), (int32)NewTarget);
}

float UPerf_FrameRateController::GetCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
    }
    return CurrentFPS;
}

float UPerf_FrameRateController::GetAverageFrameRate() const
{
    return AverageFPS;
}

void UPerf_FrameRateController::EnableVSync(bool bEnable)
{
    CurrentSettings.bUseVSync = bEnable;
    
    // Apply VSync setting
    static IConsoleVariable* VSyncCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"));
    if (VSyncCVar)
    {
        VSyncCVar->Set(bEnable ? 1 : 0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: VSync %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_FrameRateController::SetFrameRateSmoothing(bool bEnable, float SmoothingFactor)
{
    CurrentSettings.bUseFrameRateSmoothing = bEnable;
    CurrentSettings.FrameRateSmoothingFactor = FMath::Clamp(SmoothingFactor, 0.01f, 1.0f);
    
    // Apply smoothing setting
    static IConsoleVariable* SmoothCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    if (SmoothCVar && bEnable)
    {
        // Enable frame rate smoothing through engine
        static IConsoleVariable* SmoothingCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FrameRateSmoothing"));
        if (SmoothingCVar)
        {
            SmoothingCVar->Set(bEnable ? 1 : 0);
        }
    }
}

void UPerf_FrameRateController::EnableAdaptiveFrameRate(bool bEnable)
{
    CurrentSettings.bAdaptiveFrameRate = bEnable;
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Adaptive frame rate %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UPerf_FrameRateController::IsFrameRateStable() const
{
    return bIsFrameRateStable;
}

void UPerf_FrameRateController::ApplyFrameRateSettings(const FPerf_FrameRateSettings& Settings)
{
    CurrentSettings = Settings;
    
    ApplyTargetFrameRate();
    EnableVSync(Settings.bUseVSync);
    SetFrameRateSmoothing(Settings.bUseFrameRateSmoothing, Settings.FrameRateSmoothingFactor);
    EnableAdaptiveFrameRate(Settings.bAdaptiveFrameRate);
}

void UPerf_FrameRateController::TestFrameRateControl()
{
    UE_LOG(LogTemp, Warning, TEXT("FrameRateController: Testing frame rate control"));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), GetCurrentFrameRate());
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.2f"), GetAverageFrameRate());
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate Stable: %s"), IsFrameRateStable() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Target: %d"), (int32)CurrentSettings.TargetFrameRate);
}

void UPerf_FrameRateController::UpdateFrameRateStats()
{
    float DeltaTime = 0.0f;
    if (GEngine && GEngine->GetGameViewport())
    {
        UWorld* World = GEngine->GetGameViewport()->GetWorld();
        if (World)
        {
            DeltaTime = World->GetDeltaSeconds();
        }
    }
    
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
        
        // Add to history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average FPS
        if (FrameTimeHistory.Num() > 0)
        {
            float AverageFrameTime = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                AverageFrameTime += FrameTime;
            }
            AverageFrameTime /= FrameTimeHistory.Num();
            AverageFPS = 1.0f / AverageFrameTime;
            
            // Calculate variance
            float Variance = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                float Diff = FrameTime - AverageFrameTime;
                Variance += Diff * Diff;
            }
            FrameTimeVariance = Variance / FrameTimeHistory.Num();
            
            // Check stability (low variance means stable frame rate)
            bIsFrameRateStable = FrameTimeVariance < 0.001f; // 1ms variance threshold
        }
    }
    
    // Check adaptive frame rate
    if (CurrentSettings.bAdaptiveFrameRate)
    {
        CheckAdaptiveFrameRate();
    }
    
    FrameCounter++;
}

void UPerf_FrameRateController::ApplyTargetFrameRate()
{
    static IConsoleVariable* MaxFPSCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    if (!MaxFPSCVar)
    {
        return;
    }
    
    switch (CurrentSettings.TargetFrameRate)
    {
        case EPerf_FrameRateTarget::FPS_30:
            MaxFPSCVar->Set(30);
            break;
        case EPerf_FrameRateTarget::FPS_60:
            MaxFPSCVar->Set(60);
            break;
        case EPerf_FrameRateTarget::FPS_120:
            MaxFPSCVar->Set(120);
            break;
        case EPerf_FrameRateTarget::FPS_VSYNC:
            MaxFPSCVar->Set(0); // 0 = VSync
            EnableVSync(true);
            break;
        case EPerf_FrameRateTarget::FPS_UNLIM:
            MaxFPSCVar->Set(0);
            EnableVSync(false);
            break;
    }
}

void UPerf_FrameRateController::CheckAdaptiveFrameRate()
{
    if (AverageFPS < CurrentSettings.MinFrameRateThreshold)
    {
        // Frame rate too low - could trigger quality reduction
        UE_LOG(LogTemp, Warning, TEXT("FrameRateController: Frame rate below threshold (%.2f < %.2f)"), 
               AverageFPS, CurrentSettings.MinFrameRateThreshold);
    }
    else if (AverageFPS > CurrentSettings.MaxFrameRateThreshold)
    {
        // Frame rate high - could allow quality increase
        UE_LOG(LogTemp, Log, TEXT("FrameRateController: Frame rate above threshold (%.2f > %.2f)"), 
               AverageFPS, CurrentSettings.MaxFrameRateThreshold);
    }
}