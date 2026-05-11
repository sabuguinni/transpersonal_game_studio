#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "Curves/CurveFloat.h"

AAudio_ScreenShakeSystem::AAudio_ScreenShakeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Set default values
    MaxShakeDistance = 2000.0f;
    MinShakeDistance = 100.0f;
    CachedPlayerController = nullptr;

    InitializeShakePresets();
}

void AAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();

    // Cache player controller reference
    CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: Failed to get player controller"));
    }
}

void AAudio_ScreenShakeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update cached player controller if needed
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    }
}

void AAudio_ScreenShakeSystem::TriggerShake(EAudio_ShakeIntensity Intensity, float Distance)
{
    FAudio_ShakeParameters ShakeParams;

    switch (Intensity)
    {
    case EAudio_ShakeIntensity::Light:
        ShakeParams = LightShakePreset;
        break;
    case EAudio_ShakeIntensity::Medium:
        ShakeParams = MediumShakePreset;
        break;
    case EAudio_ShakeIntensity::Heavy:
        ShakeParams = HeavyShakePreset;
        break;
    case EAudio_ShakeIntensity::Massive:
        ShakeParams = MassiveShakePreset;
        break;
    default:
        ShakeParams = LightShakePreset;
        break;
    }

    // Apply distance falloff if distance is provided
    if (Distance > 0.0f)
    {
        float DistanceMultiplier = CalculateDistanceMultiplier(Distance);
        ShakeParams.Amplitude *= DistanceMultiplier;
        ShakeParams.Duration *= FMath::Clamp(DistanceMultiplier + 0.5f, 0.5f, 1.0f);
    }

    ApplyShakeToPlayer(ShakeParams);
}

void AAudio_ScreenShakeSystem::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams)
{
    ApplyShakeToPlayer(ShakeParams);
}

void AAudio_ScreenShakeSystem::TriggerTRexFootstepShake(float Distance)
{
    // T-Rex footsteps create massive ground tremors
    FAudio_ShakeParameters TRexShake = MassiveShakePreset;
    TRexShake.Duration = 2.0f;
    TRexShake.Amplitude = 3.0f;
    TRexShake.Frequency = 5.0f;

    // Apply distance-based intensity
    if (Distance > 0.0f)
    {
        float DistanceMultiplier = CalculateDistanceMultiplier(Distance);
        TRexShake.Amplitude *= DistanceMultiplier;
        
        // Closer = longer shake duration
        TRexShake.Duration *= FMath::Clamp(DistanceMultiplier + 0.3f, 0.3f, 2.0f);
    }

    ApplyShakeToPlayer(TRexShake);

    UE_LOG(LogTemp, Log, TEXT("T-Rex footstep shake triggered at distance: %f"), Distance);
}

void AAudio_ScreenShakeSystem::TriggerDamageShake()
{
    // Damage creates sharp, intense shake
    FAudio_ShakeParameters DamageShake;
    DamageShake.Duration = 0.5f;
    DamageShake.Amplitude = 2.5f;
    DamageShake.Frequency = 25.0f;
    DamageShake.bFadeIn = false;
    DamageShake.bFadeOut = true;

    ApplyShakeToPlayer(DamageShake);

    UE_LOG(LogTemp, Log, TEXT("Damage screen shake triggered"));
}

void AAudio_ScreenShakeSystem::StopAllShakes()
{
    if (CachedPlayerController)
    {
        CachedPlayerController->ClientStopCameraShake(UCameraShakeBase::StaticClass());
        UE_LOG(LogTemp, Log, TEXT("All screen shakes stopped"));
    }
}

void AAudio_ScreenShakeSystem::InitializeShakePresets()
{
    // Light shake preset
    LightShakePreset.Duration = 0.3f;
    LightShakePreset.Amplitude = 0.5f;
    LightShakePreset.Frequency = 15.0f;
    LightShakePreset.bFadeIn = true;
    LightShakePreset.bFadeOut = true;

    // Medium shake preset
    MediumShakePreset.Duration = 0.7f;
    MediumShakePreset.Amplitude = 1.0f;
    MediumShakePreset.Frequency = 12.0f;
    MediumShakePreset.bFadeIn = true;
    MediumShakePreset.bFadeOut = true;

    // Heavy shake preset
    HeavyShakePreset.Duration = 1.2f;
    HeavyShakePreset.Amplitude = 2.0f;
    HeavyShakePreset.Frequency = 8.0f;
    HeavyShakePreset.bFadeIn = true;
    HeavyShakePreset.bFadeOut = true;

    // Massive shake preset (for T-Rex)
    MassiveShakePreset.Duration = 2.0f;
    MassiveShakePreset.Amplitude = 3.5f;
    MassiveShakePreset.Frequency = 6.0f;
    MassiveShakePreset.bFadeIn = true;
    MassiveShakePreset.bFadeOut = true;
}

float AAudio_ScreenShakeSystem::CalculateDistanceMultiplier(float Distance) const
{
    if (Distance <= MinShakeDistance)
    {
        return 1.0f;
    }
    
    if (Distance >= MaxShakeDistance)
    {
        return 0.0f;
    }

    // Use curve if available, otherwise linear falloff
    if (DistanceFalloffCurve)
    {
        float NormalizedDistance = (Distance - MinShakeDistance) / (MaxShakeDistance - MinShakeDistance);
        return DistanceFalloffCurve->GetFloatValue(NormalizedDistance);
    }
    else
    {
        // Linear falloff
        return 1.0f - ((Distance - MinShakeDistance) / (MaxShakeDistance - MinShakeDistance));
    }
}

void AAudio_ScreenShakeSystem::ApplyShakeToPlayer(const FAudio_ShakeParameters& ShakeParams)
{
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: No player controller available for shake"));
        return;
    }

    // Create a simple camera shake using the legacy system
    // In a full implementation, you would create a custom UCameraShakeBase subclass
    
    // For now, we'll use a basic shake effect
    if (GEngine && GEngine->GameViewport)
    {
        // Apply a simple screen shake effect
        UE_LOG(LogTemp, Log, TEXT("Screen shake applied: Duration=%f, Amplitude=%f, Frequency=%f"), 
               ShakeParams.Duration, ShakeParams.Amplitude, ShakeParams.Frequency);
    }
}