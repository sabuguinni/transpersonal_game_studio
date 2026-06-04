#include "Audio_EffectSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AAudio_EffectSystem::AAudio_EffectSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    EffectAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectAudio"));

    // Set default values
    AmbientVolumeMultiplier = 0.7f;
    EffectVolumeMultiplier = 1.0f;

    // Configure ambient component
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolumeMultiplier);
    }

    // Configure effect component
    if (EffectAudioComponent)
    {
        EffectAudioComponent->bAutoActivate = false;
        EffectAudioComponent->SetVolumeMultiplier(EffectVolumeMultiplier);
    }
}

void AAudio_EffectSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeAudioComponents();
    
    // Auto-start forest ambient
    StartForestAmbient();
}

void AAudio_EffectSystem::InitializeAudioComponents()
{
    if (AmbientAudioComponent && ForestAmbientSound)
    {
        AmbientAudioComponent->SetSound(ForestAmbientSound);
    }
}

void AAudio_EffectSystem::PlayTRexFootstep(FVector Location, float VolumeScale)
{
    if (TRexFootstepSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            TRexFootstepSound,
            Location,
            VolumeScale * EffectVolumeMultiplier,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation override
            nullptr, // Concurrency settings
            GetOwner() // Owner
        );
    }
}

void AAudio_EffectSystem::PlayDinosaurRoar(FVector Location, float VolumeScale)
{
    if (DinosaurRoarSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DinosaurRoarSound,
            Location,
            VolumeScale * EffectVolumeMultiplier,
            FMath::RandRange(0.9f, 1.1f), // Random pitch variation
            0.0f,
            nullptr,
            nullptr,
            GetOwner()
        );
    }
}

void AAudio_EffectSystem::StartForestAmbient()
{
    if (AmbientAudioComponent && ForestAmbientSound)
    {
        AmbientAudioComponent->SetSound(ForestAmbientSound);
        AmbientAudioComponent->Play();
    }
}

void AAudio_EffectSystem::StopForestAmbient()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
}

void AAudio_EffectSystem::SetAmbientVolume(float NewVolume)
{
    AmbientVolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolumeMultiplier);
    }
}

void AAudio_EffectSystem::PlaySpatialEffect(USoundCue* SoundToPlay, FVector Location, float Volume, float Pitch)
{
    if (SoundToPlay && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            Location,
            Volume * EffectVolumeMultiplier,
            Pitch,
            0.0f,
            nullptr,
            nullptr,
            GetOwner()
        );
    }
}