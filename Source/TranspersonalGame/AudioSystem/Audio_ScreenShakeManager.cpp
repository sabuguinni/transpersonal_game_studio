#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Math/UnrealMathUtility.h"

// Static instance
AAudio_ScreenShakeManager* AAudio_ScreenShakeManager::Instance = nullptr;

AAudio_ScreenShakeManager::AAudio_ScreenShakeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize shake presets
    // T-Rex footstep - massive, long duration
    TRexFootstepShake.Duration = 2.0f;
    TRexFootstepShake.Amplitude = 3.0f;
    TRexFootstepShake.Frequency = 8.0f;
    TRexFootstepShake.bFadeIn = true;
    TRexFootstepShake.bFadeOut = true;

    // Brachiosaurus footstep - very heavy, slower frequency
    BrachiosaurusFootstepShake.Duration = 2.5f;
    BrachiosaurusFootstepShake.Amplitude = 4.0f;
    BrachiosaurusFootstepShake.Frequency = 6.0f;
    BrachiosaurusFootstepShake.bFadeIn = true;
    BrachiosaurusFootstepShake.bFadeOut = true;

    // Raptor landing - quick, sharp
    RaptorLandingShake.Duration = 0.3f;
    RaptorLandingShake.Amplitude = 1.5f;
    RaptorLandingShake.Frequency = 15.0f;
    RaptorLandingShake.bFadeIn = false;
    RaptorLandingShake.bFadeOut = true;

    // Player damage - medium intensity, quick
    PlayerDamageShake.Duration = 0.5f;
    PlayerDamageShake.Amplitude = 2.0f;
    PlayerDamageShake.Frequency = 20.0f;
    PlayerDamageShake.bFadeIn = false;
    PlayerDamageShake.bFadeOut = true;

    // Seismic rumble - long, rolling
    SeismicRumbleShake.Duration = 5.0f;
    SeismicRumbleShake.Amplitude = 1.0f;
    SeismicRumbleShake.Frequency = 4.0f;
    SeismicRumbleShake.bFadeIn = true;
    SeismicRumbleShake.bFadeOut = true;

    // Set as singleton instance
    Instance = this;
}

void AAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure this is the active instance
    Instance = this;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Initialized and ready for shake events"));
}

void AAudio_ScreenShakeManager::TriggerDinosaurFootstep(const FString& DinosaurType, const FVector& Location)
{
    FAudio_ShakeData ShakeToUse;
    
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Tyrannosaurus")))
    {
        ShakeToUse = TRexFootstepShake;
    }
    else if (DinosaurType.Contains(TEXT("Brachiosaurus")) || DinosaurType.Contains(TEXT("Sauropod")))
    {
        ShakeToUse = BrachiosaurusFootstepShake;
    }
    else if (DinosaurType.Contains(TEXT("Raptor")) || DinosaurType.Contains(TEXT("Velociraptor")))
    {
        ShakeToUse = RaptorLandingShake;
    }
    else
    {
        // Default medium shake for unknown dinosaurs
        ShakeToUse = PlayerDamageShake;
    }

    // Calculate distance-based intensity
    float DistanceMultiplier = CalculateShakeIntensityByDistance(Location);
    
    // Apply the shake
    ApplyShakeToCamera(ShakeToUse, DistanceMultiplier);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Triggered %s footstep shake at distance multiplier %.2f"), 
           *DinosaurType, DistanceMultiplier);
}

void AAudio_ScreenShakeManager::TriggerPlayerDamage(float DamageAmount)
{
    // Scale shake intensity based on damage amount
    float IntensityMultiplier = FMath::Clamp(DamageAmount / 50.0f, 0.2f, 2.0f);
    
    ApplyShakeToCamera(PlayerDamageShake, IntensityMultiplier);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Triggered damage shake with intensity %.2f"), 
           IntensityMultiplier);
}

void AAudio_ScreenShakeManager::TriggerSeismicEvent(float Intensity, float Duration)
{
    FAudio_ShakeData CustomSeismic = SeismicRumbleShake;
    CustomSeismic.Duration = Duration;
    CustomSeismic.Amplitude *= Intensity;
    
    ApplyShakeToCamera(CustomSeismic);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Triggered seismic event - Intensity: %.2f, Duration: %.2f"), 
           Intensity, Duration);
}

void AAudio_ScreenShakeManager::TriggerCustomShake(const FAudio_ShakeData& ShakeData)
{
    ApplyShakeToCamera(ShakeData);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Triggered custom shake - Duration: %.2f, Amplitude: %.2f"), 
           ShakeData.Duration, ShakeData.Amplitude);
}

float AAudio_ScreenShakeManager::CalculateShakeIntensityByDistance(const FVector& SourceLocation, float MaxDistance)
{
    UCameraComponent* PlayerCamera = GetPlayerCamera();
    if (!PlayerCamera)
    {
        return 0.0f;
    }

    FVector CameraLocation = PlayerCamera->GetComponentLocation();
    float Distance = FVector::Dist(SourceLocation, CameraLocation);
    
    // Linear falloff from 1.0 at distance 0 to 0.0 at MaxDistance
    float Intensity = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
    
    return Intensity;
}

UCameraComponent* AAudio_ScreenShakeManager::GetPlayerCamera()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return nullptr;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    // Try to find camera component
    UCameraComponent* CameraComponent = PlayerPawn->FindComponentByClass<UCameraComponent>();
    return CameraComponent;
}

AAudio_ScreenShakeManager* AAudio_ScreenShakeManager::GetInstance()
{
    return Instance;
}

void AAudio_ScreenShakeManager::ApplyShakeToCamera(const FAudio_ShakeData& ShakeData, float IntensityMultiplier)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_ScreenShakeManager: No world context for camera shake"));
        return;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_ScreenShakeManager: No player controller found"));
        return;
    }

    // For now, we'll use a simple camera shake simulation
    // In a full implementation, you would create a proper CameraShake class
    
    // Log the shake for debugging
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Applying camera shake - Duration: %.2f, Amplitude: %.2f, Intensity: %.2f"), 
           ShakeData.Duration, ShakeData.Amplitude * IntensityMultiplier, IntensityMultiplier);
    
    // TODO: Implement actual camera shake using UE5's camera shake system
    // This would involve creating a UCameraShakeBase subclass and calling:
    // PlayerController->ClientStartCameraShake(CameraShakeClass, IntensityMultiplier);
}