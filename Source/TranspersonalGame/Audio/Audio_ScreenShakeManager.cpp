#include "Audio_ScreenShakeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShake.h"

UAudio_ScreenShakeManager::UAudio_ScreenShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    GlobalShakeMultiplier = 1.0f;
    bEnableScreenShake = true;
    MaxShakeDistance = 10000.0f;
}

void UAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeShakeProfiles();
}

void UAudio_ScreenShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAudio_ScreenShakeManager::InitializeShakeProfiles()
{
    // Light shake - small dinosaurs, distant impacts
    FAudio_ShakeProfile LightProfile;
    LightProfile.Duration = 0.3f;
    LightProfile.Amplitude = 0.2f;
    LightProfile.Frequency = 15.0f;
    LightProfile.FalloffRadius = 2000.0f;
    LightProfile.bUseDistanceAttenuation = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightProfile);

    // Medium shake - medium dinosaurs, player impacts
    FAudio_ShakeProfile MediumProfile;
    MediumProfile.Duration = 0.6f;
    MediumProfile.Amplitude = 0.5f;
    MediumProfile.Frequency = 12.0f;
    MediumProfile.FalloffRadius = 4000.0f;
    MediumProfile.bUseDistanceAttenuation = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumProfile);

    // Heavy shake - T-Rex footsteps, large impacts
    FAudio_ShakeProfile HeavyProfile;
    HeavyProfile.Duration = 1.2f;
    HeavyProfile.Amplitude = 1.0f;
    HeavyProfile.Frequency = 8.0f;
    HeavyProfile.FalloffRadius = 6000.0f;
    HeavyProfile.bUseDistanceAttenuation = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyProfile);

    // Extreme shake - T-Rex attacks, explosions
    FAudio_ShakeProfile ExtremeProfile;
    ExtremeProfile.Duration = 2.0f;
    ExtremeProfile.Amplitude = 1.5f;
    ExtremeProfile.Frequency = 5.0f;
    ExtremeProfile.FalloffRadius = 8000.0f;
    ExtremeProfile.bUseDistanceAttenuation = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeProfile);
}

void UAudio_ScreenShakeManager::TriggerShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation)
{
    if (!bEnableScreenShake)
    {
        return;
    }

    const FAudio_ShakeProfile* Profile = ShakeProfiles.Find(Intensity);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: No shake profile found for intensity"));
        return;
    }

    TriggerCustomShake(*Profile, SourceLocation);
}

void UAudio_ScreenShakeManager::TriggerCustomShake(const FAudio_ShakeProfile& Profile, FVector SourceLocation)
{
    if (!bEnableScreenShake)
    {
        return;
    }

    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Get player location for distance calculation
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);

    // Check if within max shake distance
    if (Distance > MaxShakeDistance)
    {
        return;
    }

    // Calculate distance attenuation
    float Attenuation = 1.0f;
    if (Profile.bUseDistanceAttenuation)
    {
        Attenuation = CalculateDistanceAttenuation(SourceLocation, PlayerLocation, Profile.FalloffRadius);
    }

    // Apply global multiplier and attenuation
    float FinalAmplitude = Profile.Amplitude * GlobalShakeMultiplier * Attenuation;

    if (FinalAmplitude > 0.01f) // Only trigger if amplitude is meaningful
    {
        // Create a simple camera shake effect
        // Note: In UE5, we would typically use a camera shake class, but for now we'll use a simple approach
        if (PlayerController->PlayerCameraManager)
        {
            PlayerController->PlayerCameraManager->StartCameraShake(
                nullptr, // Camera shake class - would need to be set up in Blueprint
                FinalAmplitude,
                ECameraShakePlaySpace::CameraLocal
            );
        }

        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Triggered shake - Amplitude: %f, Distance: %f, Attenuation: %f"), 
               FinalAmplitude, Distance, Attenuation);
    }
}

void UAudio_ScreenShakeManager::TriggerDinosaurFootstep(const FString& DinosaurType, FVector FootstepLocation)
{
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;

    // Determine shake intensity based on dinosaur type
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("T-Rex")))
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DinosaurType.Contains(TEXT("Brachiosaurus")) || DinosaurType.Contains(TEXT("Triceratops")))
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    else if (DinosaurType.Contains(TEXT("Velociraptor")) || DinosaurType.Contains(TEXT("Raptor")))
    {
        Intensity = EAudio_ShakeIntensity::Light;
    }
    else
    {
        // Default for unknown dinosaurs
        Intensity = EAudio_ShakeIntensity::Light;
    }

    TriggerShake(Intensity, FootstepLocation);
}

void UAudio_ScreenShakeManager::SetGlobalShakeMultiplier(float NewMultiplier)
{
    GlobalShakeMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Global shake multiplier set to %f"), GlobalShakeMultiplier);
}

void UAudio_ScreenShakeManager::EnableScreenShake(bool bEnable)
{
    bEnableScreenShake = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Screen shake %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UAudio_ScreenShakeManager::CalculateDistanceAttenuation(FVector SourceLocation, FVector PlayerLocation, float FalloffRadius)
{
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    
    if (Distance >= FalloffRadius)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / FalloffRadius);
}

APlayerController* UAudio_ScreenShakeManager::GetPlayerController()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World->GetFirstPlayerController();
}