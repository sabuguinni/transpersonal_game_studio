#include "AudioEffectsManager.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"

AAudioEffectsManager::AAudioEffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create proximity audio component
    ProximityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximityAudio"));
    RootComponent = ProximityAudioComponent;

    // Create damage audio component
    DamageAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DamageAudio"));
    DamageAudioComponent->SetupAttachment(RootComponent);

    // Create footstep dust particle component
    FootstepDustComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootstepDust"));
    FootstepDustComponent->SetupAttachment(RootComponent);
    FootstepDustComponent->SetAutoActivate(false);

    // Initialize effect settings
    TRexShakeSettings.Intensity = 2.0f;
    TRexShakeSettings.Duration = 3.0f;
    TRexShakeSettings.Range = 2000.0f;
    TRexShakeSettings.bEnabled = true;

    DamageFlashSettings.Intensity = 1.5f;
    DamageFlashSettings.Duration = 1.0f;
    DamageFlashSettings.Range = 0.0f; // Screen effect, no range
    DamageFlashSettings.bEnabled = true;

    FootstepDustSettings.Intensity = 1.0f;
    FootstepDustSettings.Duration = 2.0f;
    FootstepDustSettings.Range = 500.0f;
    FootstepDustSettings.bEnabled = true;
}

void AAudioEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio components
    if (ProximityAudioComponent)
    {
        ProximityAudioComponent->SetVolumeMultiplier(0.8f);
        ProximityAudioComponent->SetPitchMultiplier(0.9f);
    }

    if (DamageAudioComponent)
    {
        DamageAudioComponent->SetVolumeMultiplier(1.0f);
        DamageAudioComponent->SetPitchMultiplier(1.2f);
    }
}

void AAudioEffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update damage flash timer
    if (bDamageFlashActive)
    {
        DamageFlashTimer -= DeltaTime;
        if (DamageFlashTimer <= 0.0f)
        {
            bDamageFlashActive = false;
        }
    }

    // Update day/night cycle
    CurrentTimeOfDay += DeltaTime * 0.1f; // Slow time progression
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    RotateSunLight(CurrentTimeOfDay);
}

void AAudioEffectsManager::TriggerTRexProximityEffect(FVector TRexLocation, float Distance)
{
    if (!TRexShakeSettings.bEnabled || Distance > TRexShakeSettings.Range)
    {
        return;
    }

    // Calculate intensity based on distance
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / TRexShakeSettings.Range), 0.0f, 1.0f);
    float EffectIntensity = TRexShakeSettings.Intensity * DistanceRatio;

    // Trigger screen shake
    UpdateScreenShakeIntensity(Distance);

    // Play proximity warning sound
    PlayProximityWarningSound(Distance);

    // Create ground rumble particles
    CreateFootstepDustParticles(TRexLocation, 5.0f); // Large creature size
}

void AAudioEffectsManager::TriggerDamageFlashEffect(float DamageAmount)
{
    if (!DamageFlashSettings.bEnabled)
    {
        return;
    }

    bDamageFlashActive = true;
    DamageFlashTimer = DamageFlashSettings.Duration;

    // Play damage sound
    if (DamageAudioComponent)
    {
        float DamageIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);
        DamageAudioComponent->SetVolumeMultiplier(DamageIntensity);
        DamageAudioComponent->Play();
    }
}

void AAudioEffectsManager::TriggerFootstepDustEffect(FVector FootstepLocation, float CreatureSize)
{
    if (!FootstepDustSettings.bEnabled)
    {
        return;
    }

    CreateFootstepDustParticles(FootstepLocation, CreatureSize);
}

void AAudioEffectsManager::UpdateDayNightCycle(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Fmod(TimeOfDay, 24.0f);
    RotateSunLight(CurrentTimeOfDay);
}

void AAudioEffectsManager::SetEffectEnabled(EAudio_EffectType EffectType, bool bEnabled)
{
    switch (EffectType)
    {
        case EAudio_EffectType::ScreenShake:
            TRexShakeSettings.bEnabled = bEnabled;
            break;
        case EAudio_EffectType::DamageFlash:
            DamageFlashSettings.bEnabled = bEnabled;
            break;
        case EAudio_EffectType::FootstepDust:
            FootstepDustSettings.bEnabled = bEnabled;
            break;
        case EAudio_EffectType::ProximityRumble:
            TRexShakeSettings.bEnabled = bEnabled; // Same as screen shake
            break;
    }
}

void AAudioEffectsManager::UpdateScreenShakeIntensity(float Distance)
{
    if (!TRexCameraShakeClass)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    // Calculate shake intensity based on distance
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / TRexShakeSettings.Range), 0.0f, 1.0f);
    float ShakeScale = TRexShakeSettings.Intensity * DistanceRatio;

    // Start camera shake
    PC->PlayerCameraManager->StartCameraShake(TRexCameraShakeClass, ShakeScale);
}

void AAudioEffectsManager::PlayProximityWarningSound(float Distance)
{
    if (!ProximityAudioComponent)
    {
        return;
    }

    // Adjust volume and pitch based on distance
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / TRexShakeSettings.Range), 0.0f, 1.0f);
    float Volume = 0.3f + (0.7f * DistanceRatio);
    float Pitch = 0.8f + (0.4f * DistanceRatio);

    ProximityAudioComponent->SetVolumeMultiplier(Volume);
    ProximityAudioComponent->SetPitchMultiplier(Pitch);
    ProximityAudioComponent->Play();
}

void AAudioEffectsManager::CreateFootstepDustParticles(FVector Location, float Size)
{
    if (!FootstepDustComponent)
    {
        return;
    }

    // Set particle location
    FootstepDustComponent->SetWorldLocation(Location);
    
    // Scale particles based on creature size
    float ParticleScale = FMath::Clamp(Size / 2.0f, 0.5f, 3.0f);
    FootstepDustComponent->SetRelativeScale3D(FVector(ParticleScale));

    // Activate particle system
    FootstepDustComponent->Activate(true);
}

void AAudioEffectsManager::RotateSunLight(float TimeOfDay)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);

    if (DirectionalLights.Num() > 0)
    {
        ADirectionalLight* SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        if (SunLight)
        {
            // Calculate sun rotation based on time of day
            float SunAngle = (TimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
            FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
            SunLight->SetActorRotation(SunRotation);

            // Adjust light intensity based on time
            float LightIntensity = 1.0f;
            if (TimeOfDay < 6.0f || TimeOfDay > 18.0f) // Night time
            {
                LightIntensity = 0.1f;
            }
            else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f) // Dawn/dusk
            {
                float DawnDuskFactor = (TimeOfDay < 8.0f) ? (TimeOfDay - 6.0f) / 2.0f : (18.0f - TimeOfDay) / 2.0f;
                LightIntensity = 0.1f + (0.9f * DawnDuskFactor);
            }

            SunLight->GetLightComponent()->SetIntensity(LightIntensity);
        }
    }
}