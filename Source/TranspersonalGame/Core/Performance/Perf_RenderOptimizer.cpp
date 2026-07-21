#include "Perf_RenderOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

UPerf_RenderOptimizer::UPerf_RenderOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    RenderSettings = FPerf_RenderSettings();
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    AdaptationSpeed = 2.0f;
    CurrentFrameRate = 60.0f;
    bIsOptimizing = false;
    
    FrameRateHistory.Reserve(MaxHistorySize);
}

void UPerf_RenderOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize frame rate history
    for (int32 i = 0; i < MaxHistorySize; ++i)
    {
        FrameRateHistory.Add(TargetFrameRate);
    }
    
    ApplyRenderSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RenderOptimizer initialized with target FPS: %f"), TargetFrameRate);
}

void UPerf_RenderOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update current frame rate
    CurrentFrameRate = 1.0f / DeltaTime;
    UpdateFrameRateHistory(CurrentFrameRate);
    
    // Check if we need to optimize
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        AdaptToFrameRate(DeltaTime);
        LastOptimizationTime = CurrentTime;
    }
}

void UPerf_RenderOptimizer::OptimizeRenderSettings()
{
    float AvgFrameRate = GetAverageFrameRate();
    
    bIsOptimizing = true;
    
    if (AvgFrameRate < MinFrameRate)
    {
        // Performance is too low, reduce quality
        switch (RenderSettings.QualityLevel)
        {
            case EPerf_RenderQuality::Ultra:
                SetRenderQuality(EPerf_RenderQuality::High);
                break;
            case EPerf_RenderQuality::High:
                SetRenderQuality(EPerf_RenderQuality::Medium);
                break;
            case EPerf_RenderQuality::Medium:
                SetRenderQuality(EPerf_RenderQuality::Low);
                break;
            case EPerf_RenderQuality::Low:
                SetRenderQuality(EPerf_RenderQuality::Potato);
                break;
            default:
                // Already at lowest quality, optimize individual settings
                OptimizeViewDistance();
                OptimizeFoliageSettings();
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance below target (%f FPS), reducing quality to improve performance"), AvgFrameRate);
    }
    else if (AvgFrameRate > TargetFrameRate + 10.0f)
    {
        // Performance is good, try to increase quality
        switch (RenderSettings.QualityLevel)
        {
            case EPerf_RenderQuality::Potato:
                SetRenderQuality(EPerf_RenderQuality::Low);
                break;
            case EPerf_RenderQuality::Low:
                SetRenderQuality(EPerf_RenderQuality::Medium);
                break;
            case EPerf_RenderQuality::Medium:
                SetRenderQuality(EPerf_RenderQuality::High);
                break;
            case EPerf_RenderQuality::High:
                SetRenderQuality(EPerf_RenderQuality::Ultra);
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance above target (%f FPS), increasing quality"), AvgFrameRate);
    }
    
    ApplyRenderSettings();
    bIsOptimizing = false;
}

void UPerf_RenderOptimizer::SetRenderQuality(EPerf_RenderQuality NewQuality)
{
    RenderSettings.QualityLevel = NewQuality;
    
    switch (NewQuality)
    {
        case EPerf_RenderQuality::Ultra:
            RenderSettings.ViewDistanceScale = 1.5f;
            RenderSettings.ShadowDistanceScale = 1.5f;
            RenderSettings.bEnableLumen = true;
            RenderSettings.bEnableNanite = true;
            RenderSettings.MaxFoliageInstances = 20000;
            break;
            
        case EPerf_RenderQuality::High:
            RenderSettings.ViewDistanceScale = 1.0f;
            RenderSettings.ShadowDistanceScale = 1.0f;
            RenderSettings.bEnableLumen = true;
            RenderSettings.bEnableNanite = true;
            RenderSettings.MaxFoliageInstances = 10000;
            break;
            
        case EPerf_RenderQuality::Medium:
            RenderSettings.ViewDistanceScale = 0.8f;
            RenderSettings.ShadowDistanceScale = 0.8f;
            RenderSettings.bEnableLumen = true;
            RenderSettings.bEnableNanite = false;
            RenderSettings.MaxFoliageInstances = 5000;
            break;
            
        case EPerf_RenderQuality::Low:
            RenderSettings.ViewDistanceScale = 0.6f;
            RenderSettings.ShadowDistanceScale = 0.6f;
            RenderSettings.bEnableLumen = false;
            RenderSettings.bEnableNanite = false;
            RenderSettings.MaxFoliageInstances = 2000;
            break;
            
        case EPerf_RenderQuality::Potato:
            RenderSettings.ViewDistanceScale = 0.4f;
            RenderSettings.ShadowDistanceScale = 0.4f;
            RenderSettings.bEnableLumen = false;
            RenderSettings.bEnableNanite = false;
            RenderSettings.MaxFoliageInstances = 500;
            break;
    }
}

void UPerf_RenderOptimizer::AdaptToFrameRate(float DeltaTime)
{
    float AvgFrameRate = GetAverageFrameRate();
    
    if (FMath::Abs(AvgFrameRate - TargetFrameRate) > 5.0f)
    {
        OptimizeRenderSettings();
    }
}

void UPerf_RenderOptimizer::ApplyRenderSettings()
{
    if (UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings())
    {
        // Apply view distance scaling
        GameSettings->SetViewDistanceQuality(static_cast<int32>(RenderSettings.QualityLevel));
        
        // Apply shadow settings
        GameSettings->SetShadowQuality(static_cast<int32>(RenderSettings.QualityLevel));
        
        // Apply texture quality
        GameSettings->SetTextureQuality(static_cast<int32>(RenderSettings.QualityLevel));
        
        // Apply effects quality
        GameSettings->SetVisualEffectQuality(static_cast<int32>(RenderSettings.QualityLevel));
        
        // Apply post-processing quality
        GameSettings->SetPostProcessingQuality(static_cast<int32>(RenderSettings.QualityLevel));
        
        GameSettings->ApplySettings(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied render settings - Quality: %d, ViewDistance: %f, Shadows: %f"), 
           static_cast<int32>(RenderSettings.QualityLevel), 
           RenderSettings.ViewDistanceScale, 
           RenderSettings.ShadowDistanceScale);
}

float UPerf_RenderOptimizer::GetAverageFrameRate() const
{
    if (FrameRateHistory.Num() == 0)
    {
        return TargetFrameRate;
    }
    
    float Sum = 0.0f;
    for (float FrameRate : FrameRateHistory)
    {
        Sum += FrameRate;
    }
    
    return Sum / FrameRateHistory.Num();
}

void UPerf_RenderOptimizer::ResetToDefaults()
{
    RenderSettings = FPerf_RenderSettings();
    SetRenderQuality(EPerf_RenderQuality::High);
    ApplyRenderSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Render optimizer reset to defaults"));
}

void UPerf_RenderOptimizer::UpdateFrameRateHistory(float FrameRate)
{
    // Clamp frame rate to reasonable values
    FrameRate = FMath::Clamp(FrameRate, 1.0f, 300.0f);
    
    if (FrameRateHistory.Num() >= MaxHistorySize)
    {
        FrameRateHistory.RemoveAt(0);
    }
    
    FrameRateHistory.Add(FrameRate);
}

void UPerf_RenderOptimizer::OptimizeViewDistance()
{
    RenderSettings.ViewDistanceScale = FMath::Max(0.2f, RenderSettings.ViewDistanceScale - 0.1f);
    UE_LOG(LogTemp, Log, TEXT("Optimized view distance to %f"), RenderSettings.ViewDistanceScale);
}

void UPerf_RenderOptimizer::OptimizeShadowSettings()
{
    RenderSettings.ShadowDistanceScale = FMath::Max(0.2f, RenderSettings.ShadowDistanceScale - 0.1f);
    UE_LOG(LogTemp, Log, TEXT("Optimized shadow distance to %f"), RenderSettings.ShadowDistanceScale);
}

void UPerf_RenderOptimizer::OptimizeFoliageSettings()
{
    RenderSettings.MaxFoliageInstances = FMath::Max(100, RenderSettings.MaxFoliageInstances - 500);
    UE_LOG(LogTemp, Log, TEXT("Optimized foliage instances to %d"), RenderSettings.MaxFoliageInstances);
}

void UPerf_RenderOptimizer::OptimizeLightingSettings()
{
    if (RenderSettings.bEnableLumen)
    {
        RenderSettings.bEnableLumen = false;
        UE_LOG(LogTemp, Log, TEXT("Disabled Lumen for performance"));
    }
    else if (RenderSettings.bEnableNanite)
    {
        RenderSettings.bEnableNanite = false;
        UE_LOG(LogTemp, Log, TEXT("Disabled Nanite for performance"));
    }
}