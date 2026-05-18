#include "Audio_ScreenShakeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"

AAudio_ScreenShakeSystem::AAudio_ScreenShakeSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize shake presets
    InitializeShakePresets();
}

void AAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: System initialized and ready"));
}

void AAudio_ScreenShakeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ScreenShakeSystem::InitializeShakePresets()
{
    // Light shake preset
    FAudio_ShakeData LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.5f;
    LightShake.Frequency = 8.0f;
    LightShake.FalloffRadius = 1000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake preset
    FAudio_ShakeData MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 1.0f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FalloffRadius = 1500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake preset
    FAudio_ShakeData HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 2.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.FalloffRadius = 2000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake preset
    FAudio_ShakeData ExtremeShake;
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Amplitude = 3.0f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.FalloffRadius = 2500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);

    // T-Rex specific shakes
    TRexFootstepShake.Duration = 0.8f;
    TRexFootstepShake.Amplitude = 1.5f;
    TRexFootstepShake.Frequency = 10.0f;
    TRexFootstepShake.FalloffRadius = 3000.0f;

    TRexRoarShake.Duration = 2.0f;
    TRexRoarShake.Amplitude = 2.5f;
    TRexRoarShake.Frequency = 8.0f;
    TRexRoarShake.FalloffRadius = 4000.0f;

    // Combat shakes
    DamageShake.Duration = 0.2f;
    DamageShake.Amplitude = 0.8f;
    DamageShake.Frequency = 25.0f;
    DamageShake.FalloffRadius = 0.0f; // Always full intensity for damage

    DeathShake.Duration = 1.2f;
    DeathShake.Amplitude = 2.0f;
    DeathShake.Frequency = 5.0f;
    DeathShake.FalloffRadius = 0.0f; // Always full intensity for death
}

void AAudio_ScreenShakeSystem::TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector ShakeLocation)
{
    if (ShakePresets.Contains(Intensity))
    {
        const FAudio_ShakeData& ShakeData = ShakePresets[Intensity];
        ApplyScreenShake(ShakeData, ShakeLocation);
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: Triggered %s shake at location %s"), 
               *UEnum::GetValueAsString(Intensity), *ShakeLocation.ToString());
    }
}

void AAudio_ScreenShakeSystem::TriggerTRexFootstep(FVector FootstepLocation)
{
    ApplyScreenShake(TRexFootstepShake, FootstepLocation);
    
    if (TRexFootstepShake.ShakeSound)
    {
        PlayShakeAudio(TRexFootstepShake.ShakeSound, FootstepLocation, 0.8f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: T-Rex footstep shake at %s"), *FootstepLocation.ToString());
}

void AAudio_ScreenShakeSystem::TriggerTRexRoar(FVector RoarLocation)
{
    ApplyScreenShake(TRexRoarShake, RoarLocation);
    
    if (TRexRoarShake.ShakeSound)
    {
        PlayShakeAudio(TRexRoarShake.ShakeSound, RoarLocation, 1.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: T-Rex roar shake at %s"), *RoarLocation.ToString());
}

void AAudio_ScreenShakeSystem::TriggerDamageShake()
{
    ApplyScreenShake(DamageShake, FVector::ZeroVector);
    
    if (DamageShake.ShakeSound)
    {
        PlayShakeAudio(DamageShake.ShakeSound, GetActorLocation(), 1.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: Damage shake triggered"));
}

void AAudio_ScreenShakeSystem::TriggerDeathShake()
{
    ApplyScreenShake(DeathShake, FVector::ZeroVector);
    
    if (DeathShake.ShakeSound)
    {
        PlayShakeAudio(DeathShake.ShakeSound, GetActorLocation(), 1.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: Death shake triggered"));
}

float AAudio_ScreenShakeSystem::CalculateShakeIntensityByDistance(FVector ShakeLocation, float MaxDistance)
{
    APlayerController* PC = GetPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 0.0f;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ShakeLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    float IntensityMultiplier = 1.0f - (Distance / MaxDistance);
    return FMath::Clamp(IntensityMultiplier, 0.0f, 1.0f);
}

void AAudio_ScreenShakeSystem::PlayShakeAudio(USoundBase* Sound, FVector Location, float VolumeMultiplier)
{
    if (!Sound)
    {
        return;
    }

    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        return;
    }

    // Calculate volume based on distance
    float Distance = FVector::Dist(GetActorLocation(), Location);
    float VolumeScale = CalculateShakeIntensityByDistance(Location, 2000.0f);
    float FinalVolume = VolumeMultiplier * VolumeScale;

    if (FinalVolume > 0.1f)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FinalVolume);
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Playing shake audio at volume %f"), FinalVolume);
    }
}

void AAudio_ScreenShakeSystem::ApplyScreenShake(const FAudio_ShakeData& ShakeData, FVector ShakeLocation)
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem: No PlayerController found"));
        return;
    }

    // Calculate intensity based on distance if location is specified
    float IntensityMultiplier = 1.0f;
    if (ShakeLocation != FVector::ZeroVector && ShakeData.FalloffRadius > 0.0f)
    {
        IntensityMultiplier = CalculateShakeIntensityByDistance(ShakeLocation, ShakeData.FalloffRadius);
        
        if (IntensityMultiplier <= 0.0f)
        {
            return; // Too far away
        }
    }

    // Apply the screen shake using client-side camera shake
    // Note: In a real implementation, you would use a proper CameraShake class
    // For now, we'll use a simple approach with PlayerController
    
    float FinalAmplitude = ShakeData.Amplitude * IntensityMultiplier;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Applying shake - Amplitude: %f, Duration: %f, Frequency: %f"), 
           FinalAmplitude, ShakeData.Duration, ShakeData.Frequency);
    
    // This would typically trigger a proper camera shake blueprint or C++ class
    // For now, we log the shake parameters for verification
}

APlayerController* AAudio_ScreenShakeSystem::GetPlayerController()
{
    if (UWorld* World = GetWorld())
    {
        return World->GetFirstPlayerController();
    }
    return nullptr;
}