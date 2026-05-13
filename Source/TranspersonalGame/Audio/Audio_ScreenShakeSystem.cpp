#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

UAudio_ScreenShakeSystem::UAudio_ScreenShakeSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    MinDinosaurMassForShake = 1000.0f;
    MaxShakeDistance = 3000.0f;
}

void UAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeShakeProfiles();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void UAudio_ScreenShakeSystem::InitializeShakeProfiles()
{
    // Light shake profile
    FAudio_ShakeProfile LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 2.0f;
    LightShake.Frequency = 15.0f;
    LightShake.FalloffDistance = 1000.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake profile
    FAudio_ShakeProfile MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 5.0f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FalloffDistance = 1500.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake profile
    FAudio_ShakeProfile HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 8.0f;
    HeavyShake.Frequency = 8.0f;
    HeavyShake.FalloffDistance = 2000.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake profile
    FAudio_ShakeProfile ExtremeShake;
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Amplitude = 12.0f;
    ExtremeShake.Frequency = 6.0f;
    ExtremeShake.FalloffDistance = 2500.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);
}

void UAudio_ScreenShakeSystem::TriggerShake(EAudio_ShakeIntensity Intensity, FVector ShakeOrigin)
{
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!CachedPlayerController) return;
    }

    const FAudio_ShakeProfile* ShakeProfile = ShakeProfiles.Find(Intensity);
    if (!ShakeProfile) return;

    TriggerCustomShake(*ShakeProfile, ShakeOrigin);
}

void UAudio_ScreenShakeSystem::TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile, FVector ShakeOrigin)
{
    if (!CachedPlayerController) return;

    APawn* PlayerPawn = CachedPlayerController->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ShakeOrigin);

    if (ShakeProfile.bUseDistanceFalloff && Distance > ShakeProfile.FalloffDistance)
    {
        return; // Too far away
    }

    float ShakeIntensity = ShakeProfile.Amplitude;
    if (ShakeProfile.bUseDistanceFalloff)
    {
        ShakeIntensity = CalculateShakeIntensity(Distance, ShakeProfile.FalloffDistance, ShakeProfile.Amplitude);
    }

    // Create a simple camera shake effect
    if (GEngine)
    {
        FString ShakeMessage = FString::Printf(TEXT("Screen Shake: Intensity %.1f, Duration %.1f"), 
                                             ShakeIntensity, ShakeProfile.Duration);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, ShakeMessage);
    }

    // TODO: Implement actual camera shake when UE5 camera shake classes are available
    // For now, we'll use a simple screen flash effect via the player controller
    if (CachedPlayerController)
    {
        // Trigger a brief controller vibration if available
        CachedPlayerController->ClientPlayForceFeedback(nullptr, false, false, NAME_None);
    }
}

void UAudio_ScreenShakeSystem::TriggerDinosaurFootstep(float DinosaurMass, FVector FootstepLocation)
{
    if (DinosaurMass < MinDinosaurMassForShake) return;

    // Calculate shake intensity based on dinosaur mass
    EAudio_ShakeIntensity ShakeIntensity = EAudio_ShakeIntensity::Light;
    
    if (DinosaurMass >= 10000.0f) // 10+ tons (T-Rex, Brachiosaurus)
    {
        ShakeIntensity = EAudio_ShakeIntensity::Extreme;
    }
    else if (DinosaurMass >= 5000.0f) // 5-10 tons (Large predators)
    {
        ShakeIntensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DinosaurMass >= 2000.0f) // 2-5 tons (Medium dinosaurs)
    {
        ShakeIntensity = EAudio_ShakeIntensity::Medium;
    }
    else // 1-2 tons (Raptors, smaller dinosaurs)
    {
        ShakeIntensity = EAudio_ShakeIntensity::Light;
    }

    TriggerShake(ShakeIntensity, FootstepLocation);
}

void UAudio_ScreenShakeSystem::StopAllShakes()
{
    if (CachedPlayerController)
    {
        // Stop any ongoing camera shakes
        CachedPlayerController->ClientStopCameraShake(nullptr, true);
    }
}

float UAudio_ScreenShakeSystem::CalculateShakeIntensity(float Distance, float MaxDistance, float BaseMagnitude)
{
    if (Distance >= MaxDistance) return 0.0f;
    
    float DistanceRatio = Distance / MaxDistance;
    float Falloff = 1.0f - (DistanceRatio * DistanceRatio); // Quadratic falloff
    
    return BaseMagnitude * FMath::Clamp(Falloff, 0.0f, 1.0f);
}