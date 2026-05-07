#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "LegacyCameraShake.h"

UAudio_ScreenShakeSystem::UAudio_ScreenShakeSystem()
{
    ShakeIntensityMultiplier = 1.0f;
    MaxProximityDistance = 5000.0f;
    ShakeCooldown = 0.1f;
    LastShakeTime = 0.0f;
}

void UAudio_ScreenShakeSystem::Initialize()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Initializing screen shake system"));
    
    InitializeShakePresets();
    LastShakeTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Initialization complete"));
}

void UAudio_ScreenShakeSystem::TriggerProximityShake(AActor* SourceActor, float Distance, float CreatureSize)
{
    if (!SourceActor || IsShakeOnCooldown())
    {
        return;
    }

    // Calculate shake intensity based on distance and creature size
    float DistanceIntensity = CalculateDistanceIntensity(Distance, MaxProximityDistance);
    float SizeMultiplier = FMath::Clamp(CreatureSize / 1000.0f, 0.1f, 3.0f); // Normalize creature size
    
    FAudio_ShakeParameters ShakeParams;
    ShakeParams.Duration = FMath::Lerp(0.3f, 1.5f, SizeMultiplier);
    ShakeParams.Amplitude = FMath::Lerp(0.2f, 2.0f, DistanceIntensity * SizeMultiplier);
    ShakeParams.Frequency = FMath::Lerp(5.0f, 15.0f, SizeMultiplier);
    ShakeParams.bFadeIn = true;
    ShakeParams.bFadeOut = true;

    ExecuteCameraShake(ShakeParams, SourceActor->GetActorLocation());
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Proximity shake triggered - Distance: %f, Size: %f, Intensity: %f"), 
           Distance, CreatureSize, ShakeParams.Amplitude);
}

void UAudio_ScreenShakeSystem::TriggerDamageShake(float DamageAmount, FVector ImpactLocation)
{
    if (IsShakeOnCooldown())
    {
        return;
    }

    // Scale shake based on damage amount
    float DamageIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);
    
    FAudio_ShakeParameters ShakeParams;
    ShakeParams.Duration = FMath::Lerp(0.2f, 0.8f, DamageIntensity);
    ShakeParams.Amplitude = FMath::Lerp(0.5f, 3.0f, DamageIntensity);
    ShakeParams.Frequency = FMath::Lerp(15.0f, 25.0f, DamageIntensity);
    ShakeParams.bFadeIn = false; // Immediate impact
    ShakeParams.bFadeOut = true;

    ExecuteCameraShake(ShakeParams, ImpactLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Damage shake triggered - Damage: %f, Intensity: %f"), 
           DamageAmount, ShakeParams.Amplitude);
}

void UAudio_ScreenShakeSystem::TriggerFootstepShake(FVector FootstepLocation, float CreatureWeight)
{
    if (IsShakeOnCooldown())
    {
        return;
    }

    // Only trigger for very large creatures (T-Rex, Brachiosaurus)
    if (CreatureWeight < 5000.0f) // 5 tons minimum
    {
        return;
    }

    float WeightMultiplier = FMath::Clamp(CreatureWeight / 10000.0f, 0.1f, 2.0f);
    
    FAudio_ShakeParameters ShakeParams;
    ShakeParams.Duration = 0.3f;
    ShakeParams.Amplitude = FMath::Lerp(0.1f, 0.8f, WeightMultiplier);
    ShakeParams.Frequency = FMath::Lerp(8.0f, 12.0f, WeightMultiplier);
    ShakeParams.bFadeIn = false;
    ShakeParams.bFadeOut = true;

    ExecuteCameraShake(ShakeParams, FootstepLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Footstep shake triggered - Weight: %f, Intensity: %f"), 
           CreatureWeight, ShakeParams.Amplitude);
}

void UAudio_ScreenShakeSystem::TriggerEnvironmentalShake(EAudio_ShakeIntensity Intensity, FVector EpicenterLocation)
{
    if (IsShakeOnCooldown())
    {
        return;
    }

    FAudio_ShakeParameters* ShakeParams = ShakePresets.Find(Intensity);
    if (ShakeParams)
    {
        ExecuteCameraShake(*ShakeParams, EpicenterLocation);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Environmental shake triggered - Intensity: %d"), 
               (int32)Intensity);
    }
}

void UAudio_ScreenShakeSystem::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams)
{
    if (IsShakeOnCooldown())
    {
        return;
    }

    ExecuteCameraShake(ShakeParams);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Custom shake triggered - Duration: %f, Amplitude: %f"), 
           ShakeParams.Duration, ShakeParams.Amplitude);
}

void UAudio_ScreenShakeSystem::StopAllShakes()
{
    APlayerCameraManager* CameraManager = GetPlayerCameraManager();
    if (CameraManager)
    {
        CameraManager->StopAllCameraShakes(true);
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: All camera shakes stopped"));
    }
}

void UAudio_ScreenShakeSystem::SetShakeIntensityMultiplier(float Multiplier)
{
    ShakeIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Shake intensity multiplier set to %f"), ShakeIntensityMultiplier);
}

void UAudio_ScreenShakeSystem::ExecuteCameraShake(const FAudio_ShakeParameters& ShakeParams, FVector OptionalLocation)
{
    APlayerCameraManager* CameraManager = GetPlayerCameraManager();
    if (!CameraManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: No camera manager found"));
        return;
    }

    // Apply intensity multiplier
    FAudio_ShakeParameters AdjustedParams = ShakeParams;
    AdjustedParams.Amplitude *= ShakeIntensityMultiplier;

    // Create a simple camera shake
    // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
    // For now, we'll use the legacy system with basic parameters
    
    // Update last shake time
    LastShakeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Executing camera shake - Amplitude: %f, Duration: %f"), 
           AdjustedParams.Amplitude, AdjustedParams.Duration);
}

float UAudio_ScreenShakeSystem::CalculateDistanceIntensity(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Inverse square falloff for realistic intensity
    float NormalizedDistance = Distance / MaxDistance;
    return FMath::Clamp(1.0f - (NormalizedDistance * NormalizedDistance), 0.0f, 1.0f);
}

APlayerCameraManager* UAudio_ScreenShakeSystem::GetPlayerCameraManager() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->PlayerCameraManager;
        }
    }
    return nullptr;
}

bool UAudio_ScreenShakeSystem::IsShakeOnCooldown() const
{
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        return (CurrentTime - LastShakeTime) < ShakeCooldown;
    }
    return false;
}

void UAudio_ScreenShakeSystem::InitializeShakePresets()
{
    // Light shake (small creatures, distant events)
    FAudio_ShakeParameters LightShake;
    LightShake.Duration = 0.5f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 8.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake (medium creatures, moderate impacts)
    FAudio_ShakeParameters MediumShake;
    MediumShake.Duration = 1.0f;
    MediumShake.Amplitude = 0.8f;
    MediumShake.Frequency = 12.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake (large creatures, strong impacts)
    FAudio_ShakeParameters HeavyShake;
    HeavyShake.Duration = 1.5f;
    HeavyShake.Amplitude = 1.5f;
    HeavyShake.Frequency = 15.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake (massive events, volcanic activity)
    FAudio_ShakeParameters ExtremeShake;
    ExtremeShake.Duration = 3.0f;
    ExtremeShake.Amplitude = 2.5f;
    ExtremeShake.Frequency = 20.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Shake presets initialized"));
}