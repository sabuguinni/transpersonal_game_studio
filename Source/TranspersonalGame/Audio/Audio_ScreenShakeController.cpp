#include "Audio_ScreenShakeController.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CachedPlayerController = nullptr;
    
    // Initialize default shake settings
    InitializeShakePresets();
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    CachePlayerController();
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Ensure we have a valid player controller
    if (!CachedPlayerController)
    {
        CachePlayerController();
    }
}

void UAudio_ScreenShakeController::TriggerTRexFootstepShake(float Distance)
{
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: No player controller found"));
        return;
    }

    // Calculate shake intensity based on distance
    float ShakeIntensity = CalculateShakeIntensityFromDistance(Distance, TRexMaxShakeDistance);
    
    if (ShakeIntensity <= 0.0f)
    {
        return; // Too far away
    }

    // Create custom shake settings for T-Rex footstep
    FAudio_ShakeSettings TRexShake;
    TRexShake.Duration = 0.8f;
    TRexShake.Amplitude = ShakeIntensity * TRexShakeIntensityMultiplier;
    TRexShake.Frequency = 8.0f;
    TRexShake.bFadeIn = true;
    TRexShake.bFadeOut = true;

    TriggerShakeFromSettings(TRexShake);

    UE_LOG(LogTemp, Log, TEXT("T-Rex footstep shake triggered - Distance: %.1f, Intensity: %.2f"), Distance, ShakeIntensity);
}

void UAudio_ScreenShakeController::TriggerCustomShake(EAudio_ShakeIntensity Intensity, float Duration)
{
    FAudio_ShakeSettings* SettingsToUse = nullptr;

    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
            SettingsToUse = &LightShakeSettings;
            break;
        case EAudio_ShakeIntensity::Medium:
            SettingsToUse = &MediumShakeSettings;
            break;
        case EAudio_ShakeIntensity::Heavy:
            SettingsToUse = &HeavyShakeSettings;
            break;
        case EAudio_ShakeIntensity::Extreme:
            SettingsToUse = &ExtremeShakeSettings;
            break;
        default:
            SettingsToUse = &MediumShakeSettings;
            break;
    }

    if (SettingsToUse)
    {
        FAudio_ShakeSettings CustomSettings = *SettingsToUse;
        CustomSettings.Duration = Duration;
        TriggerShakeFromSettings(CustomSettings);
    }
}

void UAudio_ScreenShakeController::TriggerShakeFromSettings(const FAudio_ShakeSettings& Settings)
{
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Cannot trigger shake - no player controller"));
        return;
    }

    // Create a simple camera shake using the legacy system for compatibility
    if (CachedPlayerController->PlayerCameraManager)
    {
        // Use the simplified shake function
        CachedPlayerController->PlayerCameraManager->StartCameraShake(
            DefaultShakeClass,
            Settings.Amplitude,
            ECameraShakePlaySpace::CameraLocal,
            FRotator::ZeroRotator
        );

        UE_LOG(LogTemp, Log, TEXT("Camera shake triggered - Amplitude: %.2f, Duration: %.2f"), 
               Settings.Amplitude, Settings.Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: No camera manager found"));
    }
}

void UAudio_ScreenShakeController::StopAllShakes()
{
    if (CachedPlayerController && CachedPlayerController->PlayerCameraManager)
    {
        CachedPlayerController->PlayerCameraManager->StopAllCameraShakes(false);
        UE_LOG(LogTemp, Log, TEXT("All camera shakes stopped"));
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensityFromDistance(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    if (Distance <= TRexMinShakeDistance)
    {
        return 1.0f;
    }

    // Linear falloff from max intensity at min distance to 0 at max distance
    float NormalizedDistance = (Distance - TRexMinShakeDistance) / (MaxDistance - TRexMinShakeDistance);
    return FMath::Clamp(1.0f - NormalizedDistance, 0.0f, 1.0f);
}

void UAudio_ScreenShakeController::InitializeShakePresets()
{
    // Light shake (small creatures, distant impacts)
    LightShakeSettings.Duration = 0.3f;
    LightShakeSettings.Amplitude = 0.2f;
    LightShakeSettings.Frequency = 15.0f;
    LightShakeSettings.bFadeIn = true;
    LightShakeSettings.bFadeOut = true;

    // Medium shake (medium dinosaurs, nearby impacts)
    MediumShakeSettings.Duration = 0.6f;
    MediumShakeSettings.Amplitude = 0.5f;
    MediumShakeSettings.Frequency = 12.0f;
    MediumShakeSettings.bFadeIn = true;
    MediumShakeSettings.bFadeOut = true;

    // Heavy shake (large dinosaurs, close impacts)
    HeavyShakeSettings.Duration = 1.0f;
    HeavyShakeSettings.Amplitude = 0.8f;
    HeavyShakeSettings.Frequency = 10.0f;
    HeavyShakeSettings.bFadeIn = true;
    HeavyShakeSettings.bFadeOut = true;

    // Extreme shake (T-Rex, massive impacts, earthquakes)
    ExtremeShakeSettings.Duration = 1.5f;
    ExtremeShakeSettings.Amplitude = 1.2f;
    ExtremeShakeSettings.Frequency = 8.0f;
    ExtremeShakeSettings.bFadeIn = true;
    ExtremeShakeSettings.bFadeOut = true;

    // T-Rex specific settings
    TRexMaxShakeDistance = 2000.0f;  // 20 meters
    TRexMinShakeDistance = 100.0f;   // 1 meter
    TRexShakeIntensityMultiplier = 2.0f;
}

void UAudio_ScreenShakeController::CachePlayerController()
{
    if (GetWorld())
    {
        CachedPlayerController = GetWorld()->GetFirstPlayerController();
        
        if (CachedPlayerController)
        {
            UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Player controller cached successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Failed to find player controller"));
        }
    }
}

UCameraShakeBase* UAudio_ScreenShakeController::CreateShakeInstance(const FAudio_ShakeSettings& Settings)
{
    // For now, return nullptr as we're using the simplified shake system
    // This method can be expanded later for custom shake classes
    return nullptr;
}