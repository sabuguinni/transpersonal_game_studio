#include "Perf_QualityScaler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/ConfigCacheIni.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerfQualityScaler, Log, All);

UPerf_QualityScaler::UPerf_QualityScaler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize default settings
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    FrameRateBuffer = 5.0f;
    
    CurrentQualityLevel = EPerf_QualityLevel::High;
    bAutoScalingEnabled = true;
    bForceUpdate = false;
    
    FrameTimeHistory.Reserve(60); // Store 60 frames of history
    
    // Initialize quality settings
    InitializeQualitySettings();
}

void UPerf_QualityScaler::BeginPlay()
{
    Super::BeginPlay();
    
    // Get initial game user settings
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        UserSettings->LoadSettings();
        CurrentQualityLevel = static_cast<EPerf_QualityLevel>(UserSettings->GetOverallScalabilityLevel());
    }
    
    UE_LOG(LogPerfQualityScaler, Log, TEXT("Quality Scaler initialized with level: %d"), static_cast<int32>(CurrentQualityLevel));
}

void UPerf_QualityScaler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAutoScalingEnabled && !bForceUpdate)
    {
        return;
    }
    
    // Update frame rate tracking
    UpdateFrameRateTracking(DeltaTime);
    
    // Check if we need to adjust quality
    if (ShouldAdjustQuality())
    {
        AdjustQualityLevel();
        bForceUpdate = false;
    }
}

void UPerf_QualityScaler::InitializeQualitySettings()
{
    // Low Quality Settings
    QualitySettings.Add(EPerf_QualityLevel::Low, FPerf_QualitySettings{
        0, // ViewDistance
        0, // AntiAliasing
        0, // PostProcess
        0, // Shadows
        0, // GlobalIllumination
        0, // Reflections
        0, // Textures
        0, // Effects
        0, // Foliage
        0  // Shading
    });
    
    // Medium Quality Settings
    QualitySettings.Add(EPerf_QualityLevel::Medium, FPerf_QualitySettings{
        1, // ViewDistance
        1, // AntiAliasing
        1, // PostProcess
        1, // Shadows
        1, // GlobalIllumination
        1, // Reflections
        1, // Textures
        1, // Effects
        1, // Foliage
        1  // Shading
    });
    
    // High Quality Settings
    QualitySettings.Add(EPerf_QualityLevel::High, FPerf_QualitySettings{
        2, // ViewDistance
        2, // AntiAliasing
        2, // PostProcess
        2, // Shadows
        2, // GlobalIllumination
        2, // Reflections
        2, // Textures
        2, // Effects
        2, // Foliage
        2  // Shading
    });
    
    // Ultra Quality Settings
    QualitySettings.Add(EPerf_QualityLevel::Ultra, FPerf_QualitySettings{
        3, // ViewDistance
        3, // AntiAliasing
        3, // PostProcess
        3, // Shadows
        3, // GlobalIllumination
        3, // Reflections
        3, // Textures
        3, // Effects
        3, // Foliage
        3  // Shading
    });
}

void UPerf_QualityScaler::UpdateFrameRateTracking(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        float CurrentFPS = 1.0f / DeltaTime;
        
        // Add to history
        FrameTimeHistory.Add(CurrentFPS);
        
        // Keep only recent history
        if (FrameTimeHistory.Num() > 60)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average FPS
        if (FrameTimeHistory.Num() > 0)
        {
            float Sum = 0.0f;
            for (float FPS : FrameTimeHistory)
            {
                Sum += FPS;
            }
            AverageFrameRate = Sum / FrameTimeHistory.Num();
        }
    }
}

bool UPerf_QualityScaler::ShouldAdjustQuality() const
{
    if (FrameTimeHistory.Num() < 30) // Need enough samples
    {
        return false;
    }
    
    // Check if performance is consistently below target
    if (AverageFrameRate < (TargetFrameRate - FrameRateBuffer))
    {
        return CurrentQualityLevel > EPerf_QualityLevel::Low;
    }
    
    // Check if performance is consistently above target (can increase quality)
    if (AverageFrameRate > (TargetFrameRate + FrameRateBuffer * 2.0f))
    {
        return CurrentQualityLevel < EPerf_QualityLevel::Ultra;
    }
    
    return false;
}

void UPerf_QualityScaler::AdjustQualityLevel()
{
    EPerf_QualityLevel NewQualityLevel = CurrentQualityLevel;
    
    if (AverageFrameRate < (TargetFrameRate - FrameRateBuffer))
    {
        // Decrease quality
        switch (CurrentQualityLevel)
        {
            case EPerf_QualityLevel::Ultra:
                NewQualityLevel = EPerf_QualityLevel::High;
                break;
            case EPerf_QualityLevel::High:
                NewQualityLevel = EPerf_QualityLevel::Medium;
                break;
            case EPerf_QualityLevel::Medium:
                NewQualityLevel = EPerf_QualityLevel::Low;
                break;
            case EPerf_QualityLevel::Low:
                // Already at lowest
                break;
        }
    }
    else if (AverageFrameRate > (TargetFrameRate + FrameRateBuffer * 2.0f))
    {
        // Increase quality
        switch (CurrentQualityLevel)
        {
            case EPerf_QualityLevel::Low:
                NewQualityLevel = EPerf_QualityLevel::Medium;
                break;
            case EPerf_QualityLevel::Medium:
                NewQualityLevel = EPerf_QualityLevel::High;
                break;
            case EPerf_QualityLevel::High:
                NewQualityLevel = EPerf_QualityLevel::Ultra;
                break;
            case EPerf_QualityLevel::Ultra:
                // Already at highest
                break;
        }
    }
    
    if (NewQualityLevel != CurrentQualityLevel)
    {
        SetQualityLevel(NewQualityLevel);
    }
}

void UPerf_QualityScaler::SetQualityLevel(EPerf_QualityLevel NewLevel)
{
    if (NewLevel == CurrentQualityLevel)
    {
        return;
    }
    
    CurrentQualityLevel = NewLevel;
    
    UE_LOG(LogPerfQualityScaler, Log, TEXT("Adjusting quality level to: %d (Average FPS: %.1f)"), 
           static_cast<int32>(NewLevel), AverageFrameRate);
    
    // Apply quality settings
    ApplyQualitySettings(NewLevel);
    
    // Clear frame history to avoid oscillation
    FrameTimeHistory.Empty();
    
    // Broadcast quality change event
    OnQualityLevelChanged.Broadcast(NewLevel, AverageFrameRate);
}

void UPerf_QualityScaler::ApplyQualitySettings(EPerf_QualityLevel QualityLevel)
{
    if (!QualitySettings.Contains(QualityLevel))
    {
        UE_LOG(LogPerfQualityScaler, Warning, TEXT("Quality settings not found for level: %d"), static_cast<int32>(QualityLevel));
        return;
    }
    
    const FPerf_QualitySettings& Settings = QualitySettings[QualityLevel];
    
    // Apply settings through Game User Settings
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        UserSettings->SetViewDistanceQuality(Settings.ViewDistanceQuality);
        UserSettings->SetAntiAliasingQuality(Settings.AntiAliasingQuality);
        UserSettings->SetPostProcessingQuality(Settings.PostProcessQuality);
        UserSettings->SetShadowQuality(Settings.ShadowQuality);
        UserSettings->SetGlobalIlluminationQuality(Settings.GlobalIlluminationQuality);
        UserSettings->SetReflectionQuality(Settings.ReflectionQuality);
        UserSettings->SetTextureQuality(Settings.TextureQuality);
        UserSettings->SetVisualEffectQuality(Settings.EffectsQuality);
        UserSettings->SetFoliageQuality(Settings.FoliageQuality);
        UserSettings->SetShadingQuality(Settings.ShadingQuality);
        
        // Apply changes
        UserSettings->ApplySettings(false);
    }
}

void UPerf_QualityScaler::SetAutoScaling(bool bEnabled)
{
    bAutoScalingEnabled = bEnabled;
    UE_LOG(LogPerfQualityScaler, Log, TEXT("Auto scaling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_QualityScaler::SetTargetFrameRate(float NewTargetFPS)
{
    TargetFrameRate = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogPerfQualityScaler, Log, TEXT("Target frame rate set to: %.1f"), TargetFrameRate);
}

void UPerf_QualityScaler::ForceQualityUpdate()
{
    bForceUpdate = true;
    FrameTimeHistory.Empty(); // Clear history for immediate response
}

float UPerf_QualityScaler::GetCurrentFrameRate() const
{
    return AverageFrameRate;
}

EPerf_QualityLevel UPerf_QualityScaler::GetCurrentQualityLevel() const
{
    return CurrentQualityLevel;
}

bool UPerf_QualityScaler::IsAutoScalingEnabled() const
{
    return bAutoScalingEnabled;
}

FString UPerf_QualityScaler::GetQualityLevelName(EPerf_QualityLevel Level) const
{
    switch (Level)
    {
        case EPerf_QualityLevel::Low: return TEXT("Low");
        case EPerf_QualityLevel::Medium: return TEXT("Medium");
        case EPerf_QualityLevel::High: return TEXT("High");
        case EPerf_QualityLevel::Ultra: return TEXT("Ultra");
        default: return TEXT("Unknown");
    }
}