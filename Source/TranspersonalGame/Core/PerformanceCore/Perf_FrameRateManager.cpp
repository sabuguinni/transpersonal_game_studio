#include "Perf_FrameRateManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    CurrentMetrics = FPerf_FrameMetrics();
    CurrentTarget = EPerf_PerformanceTarget::High_60FPS;
    bMonitoringActive = false;
    CurrentQualityLevel = 3;
    bAutoAdjustQuality = true;
    MonitoringInterval = 0.1f;
    LastUpdateTime = 0.0f;
    MaxHistorySize = 300;
    
    StartMonitoringCommand = nullptr;
    StopMonitoringCommand = nullptr;
    ReportCommand = nullptr;
}

void UPerf_FrameRateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Initializing performance monitoring system"));
    
    // Register console commands
    RegisterConsoleCommands();
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    // Initialize FPS history array
    FPSHistory.Reserve(MaxHistorySize);
}

void UPerf_FrameRateManager::Deinitialize()
{
    StopPerformanceMonitoring();
    UnregisterConsoleCommands();
    
    Super::Deinitialize();
}

void UPerf_FrameRateManager::StartPerformanceMonitoring()
{
    if (bMonitoringActive)
    {
        return;
    }
    
    bMonitoringActive = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Start timer for regular updates
    GetWorld()->GetTimerManager().SetTimer(
        MonitoringTimerHandle,
        this,
        &UPerf_FrameRateManager::UpdateMetrics,
        MonitoringInterval,
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance monitoring started"));
}

void UPerf_FrameRateManager::StopPerformanceMonitoring()
{
    if (!bMonitoringActive)
    {
        return;
    }
    
    bMonitoringActive = false;
    
    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance monitoring stopped"));
}

void UPerf_FrameRateManager::SetTargetPerformance(EPerf_PerformanceTarget Target)
{
    EPerf_PerformanceTarget OldTarget = CurrentTarget;
    CurrentTarget = Target;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Target performance changed to %d"), (int32)Target);
    
    if (bAutoAdjustQuality)
    {
        AdjustQualityForTarget();
    }
    
    if (OldTarget != CurrentTarget)
    {
        OnPerformanceThresholdCrossed.Broadcast(CurrentTarget);
    }
}

void UPerf_FrameRateManager::AdjustQualityForTarget()
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (!Settings)
    {
        return;
    }
    
    int32 NewQualityLevel = CurrentQualityLevel;
    
    switch (CurrentTarget)
    {
        case EPerf_PerformanceTarget::High_60FPS:
            NewQualityLevel = 3; // Epic
            break;
        case EPerf_PerformanceTarget::Medium_45FPS:
            NewQualityLevel = 2; // High
            break;
        case EPerf_PerformanceTarget::Console_30FPS:
            NewQualityLevel = 1; // Medium
            break;
        case EPerf_PerformanceTarget::Low_24FPS:
            NewQualityLevel = 0; // Low
            break;
    }
    
    ForceQualityLevel(NewQualityLevel);
}

void UPerf_FrameRateManager::ForceQualityLevel(int32 QualityLevel)
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (!Settings)
    {
        return;
    }
    
    QualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    CurrentQualityLevel = QualityLevel;
    
    // Apply quality settings
    Settings->SetOverallScalabilityLevel(QualityLevel);
    Settings->ApplySettings(false);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Quality level set to %d"), QualityLevel);
}

bool UPerf_FrameRateManager::IsPerformanceAcceptable() const
{
    float TargetFPS = GetTargetFPS();
    float Tolerance = TargetFPS * 0.1f; // 10% tolerance
    
    return CurrentMetrics.AverageFPS >= (TargetFPS - Tolerance);
}

float UPerf_FrameRateManager::GetPerformanceScore() const
{
    float TargetFPS = GetTargetFPS();
    if (TargetFPS <= 0.0f)
    {
        return 1.0f;
    }
    
    return FMath::Clamp(CurrentMetrics.AverageFPS / TargetFPS, 0.0f, 2.0f);
}

void UPerf_FrameRateManager::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Min FPS: %.1f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Warning, TEXT("Max FPS: %.1f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Game Thread: %.2f ms"), CurrentMetrics.GameThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("Render Thread: %.2f ms"), CurrentMetrics.RenderThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime);
    UE_LOG(LogTemp, Warning, TEXT("Target: %d"), (int32)CurrentTarget);
    UE_LOG(LogTemp, Warning, TEXT("Quality Level: %d"), CurrentQualityLevel);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), GetPerformanceScore());
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UPerf_FrameRateManager::EnablePerformanceHUD(bool bEnable)
{
    if (!GetWorld())
    {
        return;
    }
    
    if (bEnable)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat fps"));
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat unit"));
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance HUD enabled"));
    }
    else
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat none"));
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance HUD disabled"));
    }
}

float UPerf_FrameRateManager::GetTargetFPS() const
{
    switch (CurrentTarget)
    {
        case EPerf_PerformanceTarget::High_60FPS:
            return 60.0f;
        case EPerf_PerformanceTarget::Medium_45FPS:
            return 45.0f;
        case EPerf_PerformanceTarget::Console_30FPS:
            return 30.0f;
        case EPerf_PerformanceTarget::Low_24FPS:
            return 24.0f;
        default:
            return 60.0f;
    }
}

void UPerf_FrameRateManager::UpdateMetrics()
{
    if (!bMonitoringActive || !GetWorld())
    {
        return;
    }
    
    // Get current frame time
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to ms
    }
    
    // Update FPS history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Calculate average FPS
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        float Min = FPSHistory[0];
        float Max = FPSHistory[0];
        
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
            Min = FMath::Min(Min, FPS);
            Max = FMath::Max(Max, FPS);
        }
        
        CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
        CurrentMetrics.MinFPS = Min;
        CurrentMetrics.MaxFPS = Max;
    }
    
    // Get additional timing information from engine stats
    if (GEngine && GEngine->GetEngineSubsystem<UPerf_FrameRateManager>())
    {
        // These would require access to engine internals
        // For now, use approximations
        CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f;
        CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.7f;
        CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.8f;
    }
    
    // Check if we need to adjust quality
    if (bAutoAdjustQuality)
    {
        CheckPerformanceThresholds();
    }
}

void UPerf_FrameRateManager::CheckPerformanceThresholds()
{
    if (FPSHistory.Num() < 30) // Need at least 3 seconds of data
    {
        return;
    }
    
    float TargetFPS = GetTargetFPS();
    float LowThreshold = TargetFPS * 0.85f; // 15% below target
    float HighThreshold = TargetFPS * 1.1f; // 10% above target
    
    // Check if we're consistently below target
    if (CurrentMetrics.AverageFPS < LowThreshold && CurrentQualityLevel > 0)
    {
        AdjustQualityDown();
    }
    // Check if we're consistently above target and can increase quality
    else if (CurrentMetrics.AverageFPS > HighThreshold && CurrentQualityLevel < 3)
    {
        AdjustQualityUp();
    }
}

void UPerf_FrameRateManager::AdjustQualityDown()
{
    if (CurrentQualityLevel > 0)
    {
        ForceQualityLevel(CurrentQualityLevel - 1);
        UE_LOG(LogTemp, Warning, TEXT("FrameRateManager: Quality reduced to %d due to low performance"), CurrentQualityLevel);
        
        // Clear history to avoid immediate re-adjustment
        FPSHistory.Empty();
    }
}

void UPerf_FrameRateManager::AdjustQualityUp()
{
    if (CurrentQualityLevel < 3)
    {
        ForceQualityLevel(CurrentQualityLevel + 1);
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Quality increased to %d due to good performance"), CurrentQualityLevel);
        
        // Clear history to avoid immediate re-adjustment
        FPSHistory.Empty();
    }
}

void UPerf_FrameRateManager::RegisterConsoleCommands()
{
    StartMonitoringCommand = new FAutoConsoleCommand(
        TEXT("perf.StartMonitoring"),
        TEXT("Start performance monitoring"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_FrameRateManager::StartPerformanceMonitoring)
    );
    
    StopMonitoringCommand = new FAutoConsoleCommand(
        TEXT("perf.StopMonitoring"),
        TEXT("Stop performance monitoring"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_FrameRateManager::StopPerformanceMonitoring)
    );
    
    ReportCommand = new FAutoConsoleCommand(
        TEXT("perf.Report"),
        TEXT("Log performance report"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_FrameRateManager::LogPerformanceReport)
    );
}

void UPerf_FrameRateManager::UnregisterConsoleCommands()
{
    if (StartMonitoringCommand)
    {
        delete StartMonitoringCommand;
        StartMonitoringCommand = nullptr;
    }
    
    if (StopMonitoringCommand)
    {
        delete StopMonitoringCommand;
        StopMonitoringCommand = nullptr;
    }
    
    if (ReportCommand)
    {
        delete ReportCommand;
        ReportCommand = nullptr;
    }
}