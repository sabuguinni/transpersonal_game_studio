#include "Perf_FrameRateController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_FrameRateController::UPerf_FrameRateController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for accurate monitoring
    
    CurrentFrameRate = 60.0f;
    CurrentFrameTime = 1.0f / 60.0f;
    AverageFrameRate = 60.0f;
    bIsMonitoring = false;
    PerformanceCheckTimer = 0.0f;
    QualityAdjustmentCooldown = 0.0f;
    ConsecutivePoorFrames = 0;
    
    FrameTimeHistory.Reserve(MaxFrameHistorySize);
}

void UPerf_FrameRateController::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial settings
    ApplyFrameRateSettings();
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Initialized with target %d FPS"), 
           static_cast<int32>(FrameRateSettings.TargetFrameRate));
}

void UPerf_FrameRateController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdateFrameRateMetrics(DeltaTime);
        
        PerformanceCheckTimer += DeltaTime;
        if (PerformanceCheckTimer >= PerformanceCheckInterval)
        {
            CheckPerformanceAndAdjust();
            PerformanceCheckTimer = 0.0f;
        }
        
        // Update cooldown timer
        if (QualityAdjustmentCooldown > 0.0f)
        {
            QualityAdjustmentCooldown -= DeltaTime;
        }
    }
}

void UPerf_FrameRateController::SetTargetFrameRate(EPerf_FrameRateTarget NewTarget)
{
    FrameRateSettings.TargetFrameRate = NewTarget;
    ApplyFrameRateSettings();
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Target frame rate changed to %d"), 
           static_cast<int32>(NewTarget));
}

void UPerf_FrameRateController::SetQualityLevel(EPerf_QualityLevel NewQuality)
{
    FrameRateSettings.QualityLevel = NewQuality;
    ApplyQualitySettings(NewQuality);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Quality level changed to %d"), 
           static_cast<int32>(NewQuality));
}

void UPerf_FrameRateController::EnableAutoQualityAdjustment(bool bEnable)
{
    FrameRateSettings.bAutoAdjustQuality = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Auto quality adjustment %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UPerf_FrameRateController::GetCurrentFrameRate() const
{
    return CurrentFrameRate;
}

float UPerf_FrameRateController::GetCurrentFrameTime() const
{
    return CurrentFrameTime;
}

bool UPerf_FrameRateController::IsPerformanceTargetMet() const
{
    float TargetFrameTime = 1.0f / FrameRateSettings.MaxDesiredFrameRate;
    return CurrentFrameTime <= (TargetFrameTime / FrameRateSettings.PerformanceThreshold);
}

void UPerf_FrameRateController::AdjustQualityForPerformance()
{
    if (!FrameRateSettings.bAutoAdjustQuality || QualityAdjustmentCooldown > 0.0f)
    {
        return;
    }
    
    if (!IsPerformanceTargetMet())
    {
        // Decrease quality
        switch (FrameRateSettings.QualityLevel)
        {
            case EPerf_QualityLevel::Ultra:
                SetQualityLevel(EPerf_QualityLevel::High);
                break;
            case EPerf_QualityLevel::High:
                SetQualityLevel(EPerf_QualityLevel::Medium);
                break;
            case EPerf_QualityLevel::Medium:
                SetQualityLevel(EPerf_QualityLevel::Low);
                break;
            case EPerf_QualityLevel::Low:
                // Already at lowest quality
                break;
            case EPerf_QualityLevel::Auto:
                SetQualityLevel(EPerf_QualityLevel::Medium);
                break;
        }
        
        QualityAdjustmentCooldown = QualityAdjustmentCooldownTime;
        UE_LOG(LogTemp, Warning, TEXT("FrameRateController: Quality reduced due to poor performance"));
    }
    else if (AverageFrameRate > FrameRateSettings.MaxDesiredFrameRate * 1.1f)
    {
        // Increase quality if we have headroom
        switch (FrameRateSettings.QualityLevel)
        {
            case EPerf_QualityLevel::Low:
                SetQualityLevel(EPerf_QualityLevel::Medium);
                break;
            case EPerf_QualityLevel::Medium:
                SetQualityLevel(EPerf_QualityLevel::High);
                break;
            case EPerf_QualityLevel::High:
                SetQualityLevel(EPerf_QualityLevel::Ultra);
                break;
            case EPerf_QualityLevel::Ultra:
                // Already at highest quality
                break;
            case EPerf_QualityLevel::Auto:
                SetQualityLevel(EPerf_QualityLevel::High);
                break;
        }
        
        QualityAdjustmentCooldown = QualityAdjustmentCooldownTime;
        UE_LOG(LogTemp, Log, TEXT("FrameRateController: Quality increased due to performance headroom"));
    }
}

void UPerf_FrameRateController::ResetToDefaultSettings()
{
    FrameRateSettings = FPerf_FrameRateSettings();
    ApplyFrameRateSettings();
    ApplyQualitySettings(FrameRateSettings.QualityLevel);
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Settings reset to defaults"));
}

void UPerf_FrameRateController::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    FrameTimeHistory.Empty();
    PerformanceCheckTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Performance monitoring started"));
}

void UPerf_FrameRateController::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateController: Performance monitoring stopped"));
}

void UPerf_FrameRateController::LogPerformanceStats()
{
    UE_LOG(LogTemp, Log, TEXT("FrameRateController Performance Stats:"));
    UE_LOG(LogTemp, Log, TEXT("  Current FPS: %.2f"), CurrentFrameRate);
    UE_LOG(LogTemp, Log, TEXT("  Average FPS: %.2f"), AverageFrameRate);
    UE_LOG(LogTemp, Log, TEXT("  Current Frame Time: %.4f ms"), CurrentFrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("  Target Met: %s"), IsPerformanceTargetMet() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("  Quality Level: %d"), static_cast<int32>(FrameRateSettings.QualityLevel));
}

void UPerf_FrameRateController::UpdateFrameRateMetrics(float DeltaTime)
{
    CurrentFrameTime = DeltaTime;
    CurrentFrameRate = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    // Add to history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > MaxFrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average
    AverageFrameRate = CalculateAverageFrameRate();
}

void UPerf_FrameRateController::CheckPerformanceAndAdjust()
{
    if (!IsPerformanceTargetMet())
    {
        ConsecutivePoorFrames++;
        
        if (ConsecutivePoorFrames >= PoorFrameThreshold)
        {
            AdjustQualityForPerformance();
            ConsecutivePoorFrames = 0;
        }
    }
    else
    {
        ConsecutivePoorFrames = 0;
    }
}

void UPerf_FrameRateController::ApplyFrameRateSettings()
{
    if (IConsoleManager& ConsoleManager = IConsoleManager::Get())
    {
        // Set target frame rate
        int32 TargetFPS = 60;
        switch (FrameRateSettings.TargetFrameRate)
        {
            case EPerf_FrameRateTarget::FPS_30:
                TargetFPS = 30;
                break;
            case EPerf_FrameRateTarget::FPS_60:
                TargetFPS = 60;
                break;
            case EPerf_FrameRateTarget::FPS_120:
                TargetFPS = 120;
                break;
            case EPerf_FrameRateTarget::FPS_Unlimited:
                TargetFPS = 0;
                break;
        }
        
        if (auto* MaxFPSVar = ConsoleManager.FindConsoleVariable(TEXT("t.MaxFPS")))
        {
            MaxFPSVar->Set(TargetFPS);
        }
        
        // Set VSync
        if (auto* VSyncVar = ConsoleManager.FindConsoleVariable(TEXT("r.VSync")))
        {
            VSyncVar->Set(FrameRateSettings.bEnableVSync ? 1 : 0);
        }
        
        // Set frame smoothing
        if (auto* SmoothFrameRateVar = ConsoleManager.FindConsoleVariable(TEXT("t.SmoothFrameRate")))
        {
            SmoothFrameRateVar->Set(FrameRateSettings.bEnableFrameSmoothing ? 1 : 0);
        }
        
        // Set min/max desired frame rates
        if (auto* MinFrameRateVar = ConsoleManager.FindConsoleVariable(TEXT("t.MinDesiredFrameRate")))
        {
            MinFrameRateVar->Set(FrameRateSettings.MinDesiredFrameRate);
        }
        
        if (auto* MaxFrameRateVar = ConsoleManager.FindConsoleVariable(TEXT("t.MaxDesiredFrameRate")))
        {
            MaxFrameRateVar->Set(FrameRateSettings.MaxDesiredFrameRate);
        }
    }
}

void UPerf_FrameRateController::ApplyQualitySettings(EPerf_QualityLevel Quality)
{
    if (IConsoleManager& ConsoleManager = IConsoleManager::Get())
    {
        switch (Quality)
        {
            case EPerf_QualityLevel::Low:
                // Low quality settings
                if (auto* ShadowQuality = ConsoleManager.FindConsoleVariable(TEXT("r.ShadowQuality")))
                    ShadowQuality->Set(1);
                if (auto* TextureQuality = ConsoleManager.FindConsoleVariable(TEXT("r.TextureQuality")))
                    TextureQuality->Set(1);
                if (auto* EffectsQuality = ConsoleManager.FindConsoleVariable(TEXT("r.EffectsQuality")))
                    EffectsQuality->Set(1);
                break;
                
            case EPerf_QualityLevel::Medium:
                // Medium quality settings
                if (auto* ShadowQuality = ConsoleManager.FindConsoleVariable(TEXT("r.ShadowQuality")))
                    ShadowQuality->Set(2);
                if (auto* TextureQuality = ConsoleManager.FindConsoleVariable(TEXT("r.TextureQuality")))
                    TextureQuality->Set(2);
                if (auto* EffectsQuality = ConsoleManager.FindConsoleVariable(TEXT("r.EffectsQuality")))
                    EffectsQuality->Set(2);
                break;
                
            case EPerf_QualityLevel::High:
                // High quality settings
                if (auto* ShadowQuality = ConsoleManager.FindConsoleVariable(TEXT("r.ShadowQuality")))
                    ShadowQuality->Set(3);
                if (auto* TextureQuality = ConsoleManager.FindConsoleVariable(TEXT("r.TextureQuality")))
                    TextureQuality->Set(3);
                if (auto* EffectsQuality = ConsoleManager.FindConsoleVariable(TEXT("r.EffectsQuality")))
                    EffectsQuality->Set(3);
                break;
                
            case EPerf_QualityLevel::Ultra:
                // Ultra quality settings
                if (auto* ShadowQuality = ConsoleManager.FindConsoleVariable(TEXT("r.ShadowQuality")))
                    ShadowQuality->Set(4);
                if (auto* TextureQuality = ConsoleManager.FindConsoleVariable(TEXT("r.TextureQuality")))
                    TextureQuality->Set(4);
                if (auto* EffectsQuality = ConsoleManager.FindConsoleVariable(TEXT("r.EffectsQuality")))
                    EffectsQuality->Set(4);
                break;
                
            case EPerf_QualityLevel::Auto:
                // Auto will be handled by the adjustment system
                break;
        }
    }
}

float UPerf_FrameRateController::CalculateAverageFrameRate() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 60.0f;
    }
    
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    
    float AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    return (AverageFrameTime > 0.0f) ? (1.0f / AverageFrameTime) : 0.0f;
}

bool UPerf_FrameRateController::ShouldAdjustQuality() const
{
    return FrameRateSettings.bAutoAdjustQuality && 
           QualityAdjustmentCooldown <= 0.0f && 
           FrameTimeHistory.Num() >= MaxFrameHistorySize / 2;
}