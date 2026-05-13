#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AAudio_ScreenShakeSystem::AAudio_ScreenShakeSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize shake profiles for different intensities
    FAudio_ShakeProfile LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.5f;
    LightShake.Frequency = 8.0f;
    LightShake.FadeInTime = 0.05f;
    LightShake.FadeOutTime = 0.2f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightShake);

    FAudio_ShakeProfile MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 1.0f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FadeInTime = 0.1f;
    MediumShake.FadeOutTime = 0.3f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    FAudio_ShakeProfile HeavyShake;
    HeavyShake.Duration = 1.2f;
    HeavyShake.Amplitude = 2.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.FadeInTime = 0.15f;
    HeavyShake.FadeOutTime = 0.5f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    FAudio_ShakeProfile ExtremeShake;
    ExtremeShake.Duration = 2.0f;
    ExtremeShake.Amplitude = 3.5f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.FadeInTime = 0.2f;
    ExtremeShake.FadeOutTime = 0.8f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);

    MaxShakeDistance = 5000.0f;
    MinShakeDistance = 100.0f;
}

void AAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
}

void AAudio_ScreenShakeSystem::TriggerFootstepShake(FVector FootstepLocation, float CreatureWeight)
{
    if (CreatureWeight <= 0.0f)
    {
        return;
    }

    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;

    // Determine shake intensity based on creature weight (in tons)
    if (CreatureWeight >= 50.0f) // Titanosaur range
    {
        Intensity = EAudio_ShakeIntensity::Extreme;
    }
    else if (CreatureWeight >= 10.0f) // T-Rex range
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (CreatureWeight >= 3.0f) // Large herbivores
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    else if (CreatureWeight >= 0.5f) // Medium predators
    {
        Intensity = EAudio_ShakeIntensity::Light;
    }
    else
    {
        return; // Too light to cause noticeable shake
    }

    TriggerCustomShake(Intensity, FootstepLocation);
}

void AAudio_ScreenShakeSystem::TriggerCustomShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation)
{
    if (!ShakeProfiles.Contains(Intensity))
    {
        return;
    }

    const FAudio_ShakeProfile& Profile = ShakeProfiles[Intensity];
    float DistanceMultiplier = CalculateDistanceMultiplier(SourceLocation);

    if (DistanceMultiplier > 0.0f)
    {
        ApplyScreenShake(Profile, DistanceMultiplier);
    }
}

void AAudio_ScreenShakeSystem::TriggerDamageFlash(float DamageAmount)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC)
    {
        return;
    }

    // Create damage flash effect based on damage amount
    EAudio_ShakeIntensity FlashIntensity = EAudio_ShakeIntensity::Light;
    
    if (DamageAmount >= 75.0f)
    {
        FlashIntensity = EAudio_ShakeIntensity::Extreme;
    }
    else if (DamageAmount >= 50.0f)
    {
        FlashIntensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DamageAmount >= 25.0f)
    {
        FlashIntensity = EAudio_ShakeIntensity::Medium;
    }

    // Apply screen shake for damage feedback
    if (ShakeProfiles.Contains(FlashIntensity))
    {
        const FAudio_ShakeProfile& Profile = ShakeProfiles[FlashIntensity];
        ApplyScreenShake(Profile, 1.0f); // Full intensity for damage
    }
}

void AAudio_ScreenShakeSystem::ApplyScreenShake(const FAudio_ShakeProfile& Profile, float DistanceMultiplier)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC)
    {
        return;
    }

    // Create a simple camera shake effect
    // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
    // For now, we'll use the basic shake functionality
    
    float AdjustedAmplitude = Profile.Amplitude * DistanceMultiplier;
    float AdjustedDuration = Profile.Duration;

    // Apply camera shake using UE5's built-in system
    if (AdjustedAmplitude > 0.1f)
    {
        // Simple implementation - in production you'd use a proper camera shake class
        PC->ClientStartCameraShake(nullptr, AdjustedAmplitude);
    }
}

float AAudio_ScreenShakeSystem::CalculateDistanceMultiplier(FVector SourceLocation)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 0.0f;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);

    if (Distance >= MaxShakeDistance)
    {
        return 0.0f;
    }

    if (Distance <= MinShakeDistance)
    {
        return 1.0f;
    }

    // Linear falloff between min and max distance
    float DistanceRatio = (MaxShakeDistance - Distance) / (MaxShakeDistance - MinShakeDistance);
    return FMath::Clamp(DistanceRatio, 0.0f, 1.0f);
}

APlayerController* AAudio_ScreenShakeSystem::GetLocalPlayerController()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World->GetFirstPlayerController();
}