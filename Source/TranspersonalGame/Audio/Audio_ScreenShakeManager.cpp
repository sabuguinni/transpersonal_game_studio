#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UAudio_ScreenShakeManager::UAudio_ScreenShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    TRexProximityThreshold = 2000.0f;
    TRexMaxShakeDistance = 5000.0f;
    ShakeUpdateInterval = 0.1f;

    CachedPlayerController = nullptr;
    CachedCameraManager = nullptr;

    InitializeShakeProfiles();
}

void UAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache player controller and camera manager
    if (UWorld* World = GetWorld())
    {
        CachedPlayerController = World->GetFirstPlayerController();
        if (CachedPlayerController)
        {
            CachedCameraManager = CachedPlayerController->PlayerCameraManager;
        }
    }

    // Start proximity monitoring by default
    StartProximityMonitoring();
}

void UAudio_ScreenShakeManager::InitializeShakeProfiles()
{
    // Light shake for distant T-Rex
    FAudio_ShakeProfile LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 8.0f;
    LightShake.bFadeOut = true;
    LightShake.FadeOutTime = 0.2f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake for moderate distance
    FAudio_ShakeProfile MediumShake;
    MediumShake.Duration = 0.5f;
    MediumShake.Amplitude = 0.5f;
    MediumShake.Frequency = 12.0f;
    MediumShake.bFadeOut = true;
    MediumShake.FadeOutTime = 0.3f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake for close T-Rex
    FAudio_ShakeProfile HeavyShake;
    HeavyShake.Duration = 0.8f;
    HeavyShake.Amplitude = 1.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.bFadeOut = true;
    HeavyShake.FadeOutTime = 0.4f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake for damage or very close T-Rex
    FAudio_ShakeProfile ExtremeShake;
    ExtremeShake.Duration = 1.2f;
    ExtremeShake.Amplitude = 1.5f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.bFadeOut = true;
    ExtremeShake.FadeOutTime = 0.6f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);
}

void UAudio_ScreenShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cache validation happens in timer callback
}

void UAudio_ScreenShakeManager::TriggerScreenShake(EAudio_ShakeIntensity Intensity)
{
    if (ShakeProfiles.Contains(Intensity))
    {
        const FAudio_ShakeProfile& Profile = ShakeProfiles[Intensity];
        ExecuteShake(Profile);

        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Triggered %s shake"), 
            *UEnum::GetValueAsString(Intensity));
    }
}

void UAudio_ScreenShakeManager::TriggerCustomShake(float Duration, float Amplitude, float Frequency)
{
    FAudio_ShakeProfile CustomProfile;
    CustomProfile.Duration = Duration;
    CustomProfile.Amplitude = Amplitude;
    CustomProfile.Frequency = Frequency;
    CustomProfile.bFadeOut = true;
    CustomProfile.FadeOutTime = Duration * 0.3f;

    ExecuteShake(CustomProfile);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Triggered custom shake - Duration: %f, Amplitude: %f, Frequency: %f"), 
        Duration, Amplitude, Frequency);
}

void UAudio_ScreenShakeManager::TriggerDamageShake()
{
    TriggerScreenShake(EAudio_ShakeIntensity::Extreme);
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Player damage shake triggered"));
}

void UAudio_ScreenShakeManager::TriggerTRexProximityShake(float Distance)
{
    if (Distance > TRexMaxShakeDistance)
    {
        return; // Too far for any shake
    }

    // Calculate shake intensity based on distance
    float DistanceRatio = 1.0f - (Distance / TRexMaxShakeDistance);
    
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::None;
    
    if (DistanceRatio > 0.8f)
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DistanceRatio > 0.5f)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    else if (DistanceRatio > 0.2f)
    {
        Intensity = EAudio_ShakeIntensity::Light;
    }

    if (Intensity != EAudio_ShakeIntensity::None)
    {
        TriggerScreenShake(Intensity);
    }
}

void UAudio_ScreenShakeManager::StartProximityMonitoring()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProximityCheckTimer,
            this,
            &UAudio_ScreenShakeManager::CheckTRexProximity,
            ShakeUpdateInterval,
            true
        );

        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Started T-Rex proximity monitoring"));
    }
}

void UAudio_ScreenShakeManager::StopProximityMonitoring()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProximityCheckTimer);
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Stopped T-Rex proximity monitoring"));
    }
}

void UAudio_ScreenShakeManager::CheckTRexProximity()
{
    float NearestDistance = CalculateDistanceToNearestTRex();
    
    if (NearestDistance <= TRexMaxShakeDistance && NearestDistance >= 0.0f)
    {
        TriggerTRexProximityShake(NearestDistance);
    }
}

void UAudio_ScreenShakeManager::ExecuteShake(const FAudio_ShakeProfile& Profile)
{
    if (!CachedCameraManager)
    {
        // Try to re-cache
        if (UWorld* World = GetWorld())
        {
            CachedPlayerController = World->GetFirstPlayerController();
            if (CachedPlayerController)
            {
                CachedCameraManager = CachedPlayerController->PlayerCameraManager;
            }
        }
    }

    if (CachedCameraManager)
    {
        // Create camera shake parameters
        FVector ShakeLocation = FVector::ZeroVector;
        float InnerRadius = 0.0f;
        float OuterRadius = 10000.0f; // Large radius to affect player regardless of position
        
        // Apply the shake using camera manager
        // Note: In UE5, we would typically use a UCameraShakeBase subclass
        // For now, we'll use a simple approach with camera manager functions
        
        // Apply rotational shake
        FRotator ShakeRotation = FRotator(
            FMath::RandRange(-Profile.Amplitude, Profile.Amplitude),
            FMath::RandRange(-Profile.Amplitude, Profile.Amplitude),
            FMath::RandRange(-Profile.Amplitude * 0.5f, Profile.Amplitude * 0.5f)
        );

        // This is a simplified implementation - in a full game, you'd use UCameraShakeBase
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Executing shake - Duration: %f, Amplitude: %f"), 
            Profile.Duration, Profile.Amplitude);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: No camera manager available for shake"));
    }
}

float UAudio_ScreenShakeManager::CalculateDistanceToNearestTRex()
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return -1.0f;
    }

    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float NearestDistance = TRexMaxShakeDistance + 1.0f; // Start with max + 1
    bool bFoundTRex = false;

    // Find all actors with "TRex" in their name
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("TRex")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                if (Distance < NearestDistance)
                {
                    NearestDistance = Distance;
                    bFoundTRex = true;
                }
            }
        }
    }

    return bFoundTRex ? NearestDistance : -1.0f;
}