#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UAudio_ScreenShakeSystem::UAudio_ScreenShakeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default shake parameters
    InitializeShakeParameters();
}

void UAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller reference
    if (UWorld* World = GetWorld())
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(World, 0);
        if (!CachedPlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: PlayerController not found"));
        }
    }
}

void UAudio_ScreenShakeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Continuous monitoring could be added here if needed
}

void UAudio_ScreenShakeSystem::TriggerTRexProximityShake(float Distance)
{
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: No PlayerController for T-Rex shake"));
        return;
    }

    FAudio_ShakeParameters ShakeToUse;
    
    if (Distance <= TRexCloseDistance)
    {
        ShakeToUse = TRexCloseShake;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Close Proximity Shake triggered - Distance: %f"), Distance);
    }
    else if (Distance <= TRexMediumDistance)
    {
        ShakeToUse = TRexMediumShake;
        // Scale intensity based on distance
        float IntensityScale = CalculateShakeIntensity(Distance, TRexMediumDistance);
        ShakeToUse.Amplitude *= IntensityScale;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Medium Proximity Shake triggered - Distance: %f, Scale: %f"), Distance, IntensityScale);
    }
    else if (Distance <= TRexFarDistance)
    {
        ShakeToUse = TRexFarShake;
        // Scale intensity based on distance
        float IntensityScale = CalculateShakeIntensity(Distance, TRexFarDistance);
        ShakeToUse.Amplitude *= IntensityScale;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Far Proximity Shake triggered - Distance: %f, Scale: %f"), Distance, IntensityScale);
    }
    else
    {
        // Too far for shake effect
        return;
    }

    ApplyScreenShake(ShakeToUse);
}

void UAudio_ScreenShakeSystem::TriggerFootstepShake(const FVector& FootstepLocation)
{
    if (!CachedPlayerController)
    {
        return;
    }

    // Calculate distance from player to footstep
    if (APawn* PlayerPawn = CachedPlayerController->GetPawn())
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), FootstepLocation);
        
        // Only trigger if within reasonable distance
        if (Distance <= 2000.0f)
        {
            FAudio_ShakeParameters ScaledFootstepShake = FootstepShake;
            float IntensityScale = CalculateShakeIntensity(Distance, 2000.0f);
            ScaledFootstepShake.Amplitude *= IntensityScale;
            
            ApplyScreenShake(ScaledFootstepShake);
            UE_LOG(LogTemp, Log, TEXT("Footstep Shake triggered - Distance: %f, Scale: %f"), Distance, IntensityScale);
        }
    }
}

void UAudio_ScreenShakeSystem::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams)
{
    if (!CachedPlayerController)
    {
        return;
    }

    ApplyScreenShake(ShakeParams);
    UE_LOG(LogTemp, Log, TEXT("Custom Shake triggered - Duration: %f, Amplitude: %f"), ShakeParams.Duration, ShakeParams.Amplitude);
}

void UAudio_ScreenShakeSystem::StopAllShakes()
{
    if (CachedPlayerController)
    {
        CachedPlayerController->ClientStopCameraShake(CameraShakeClass);
        UE_LOG(LogTemp, Log, TEXT("All screen shakes stopped"));
    }
}

void UAudio_ScreenShakeSystem::InitializeShakeParameters()
{
    // T-Rex Close Shake (very intense)
    TRexCloseShake.Duration = 2.0f;
    TRexCloseShake.Amplitude = 8.0f;
    TRexCloseShake.Frequency = 15.0f;
    TRexCloseShake.Intensity = EAudio_ShakeIntensity::Extreme;

    // T-Rex Medium Shake (moderate)
    TRexMediumShake.Duration = 1.5f;
    TRexMediumShake.Amplitude = 4.0f;
    TRexMediumShake.Frequency = 12.0f;
    TRexMediumShake.Intensity = EAudio_ShakeIntensity::Heavy;

    // T-Rex Far Shake (subtle)
    TRexFarShake.Duration = 1.0f;
    TRexFarShake.Amplitude = 2.0f;
    TRexFarShake.Frequency = 8.0f;
    TRexFarShake.Intensity = EAudio_ShakeIntensity::Medium;

    // Footstep Shake (brief and sharp)
    FootstepShake.Duration = 0.3f;
    FootstepShake.Amplitude = 1.5f;
    FootstepShake.Frequency = 20.0f;
    FootstepShake.Intensity = EAudio_ShakeIntensity::Light;
}

void UAudio_ScreenShakeSystem::ApplyScreenShake(const FAudio_ShakeParameters& ShakeParams)
{
    if (!CachedPlayerController)
    {
        return;
    }

    // Create a simple camera shake using the legacy system
    // In a full implementation, you would create a custom UCameraShakeBase subclass
    if (UWorld* World = GetWorld())
    {
        // For now, we'll use a simple approach with the player controller
        // This would need to be expanded with actual camera shake assets
        
        // Log the shake for debugging
        UE_LOG(LogTemp, Log, TEXT("Applying Screen Shake - Duration: %f, Amplitude: %f, Frequency: %f"), 
               ShakeParams.Duration, ShakeParams.Amplitude, ShakeParams.Frequency);
        
        // In a production environment, you would:
        // 1. Create a UCameraShakeBase-derived class
        // 2. Set up the shake parameters
        // 3. Call CachedPlayerController->ClientStartCameraShake(ShakeClass)
        
        // For now, we simulate the effect by logging
        FString IntensityStr;
        switch (ShakeParams.Intensity)
        {
            case EAudio_ShakeIntensity::Light: IntensityStr = TEXT("Light"); break;
            case EAudio_ShakeIntensity::Medium: IntensityStr = TEXT("Medium"); break;
            case EAudio_ShakeIntensity::Heavy: IntensityStr = TEXT("Heavy"); break;
            case EAudio_ShakeIntensity::Extreme: IntensityStr = TEXT("Extreme"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("SCREEN SHAKE APPLIED: %s intensity for %f seconds"), *IntensityStr, ShakeParams.Duration);
    }
}

float UAudio_ScreenShakeSystem::CalculateShakeIntensity(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Inverse distance falloff with minimum intensity
    float NormalizedDistance = Distance / MaxDistance;
    float Intensity = FMath::Clamp(1.0f - NormalizedDistance, 0.1f, 1.0f);
    
    // Apply exponential falloff for more realistic feel
    return FMath::Pow(Intensity, 1.5f);
}