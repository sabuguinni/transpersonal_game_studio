// AudioZoneManager.cpp — Audio Agent #16
// Manages spatial audio zones for the prehistoric survival world.
// Each zone blends ambient sounds and music intensity based on player proximity.

#include "AudioZoneManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->VolumeMultiplier = 0.8f;

    // Default zone data
    ZoneData.ZoneType = EAudio_ZoneType::OpenPlains;
    ZoneData.BlendRadius = 500.0f;
    ZoneData.AmbientVolume = 0.8f;
    ZoneData.MusicIntensity = 0.5f;
    ZoneData.bDangerousZone = false;
}

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Start ambient sound if assigned
    if (AmbientSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
        AmbientAudioComponent->SetVolumeMultiplier(ZoneData.AmbientVolume);
        AmbientAudioComponent->Play();
    }
}

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick down danger cooldown
    if (bDangerActive && DangerCooldown > 0.0f)
    {
        DangerCooldown -= DeltaTime;
        if (DangerCooldown <= 0.0f)
        {
            bDangerActive = false;
        }
    }
}

void AAudioZoneManager::SetZoneType(EAudio_ZoneType NewZoneType)
{
    ZoneData.ZoneType = NewZoneType;

    // Adjust music intensity based on zone danger level
    switch (NewZoneType)
    {
        case EAudio_ZoneType::OpenPlains:
            ZoneData.MusicIntensity = 0.3f;
            ZoneData.bDangerousZone = false;
            break;
        case EAudio_ZoneType::DenseForest:
            ZoneData.MusicIntensity = 0.5f;
            ZoneData.bDangerousZone = false;
            break;
        case EAudio_ZoneType::RiverBank:
            ZoneData.MusicIntensity = 0.4f;
            ZoneData.bDangerousZone = false;
            break;
        case EAudio_ZoneType::CaveEntrance:
            ZoneData.MusicIntensity = 0.7f;
            ZoneData.bDangerousZone = true;
            break;
        case EAudio_ZoneType::CampSite:
            ZoneData.MusicIntensity = 0.2f;
            ZoneData.bDangerousZone = false;
            break;
        case EAudio_ZoneType::DangerZone:
            ZoneData.MusicIntensity = 1.0f;
            ZoneData.bDangerousZone = true;
            break;
        default:
            ZoneData.MusicIntensity = 0.5f;
            break;
    }
}

void AAudioZoneManager::SetAmbientVolume(float Volume)
{
    ZoneData.AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(ZoneData.AmbientVolume);
    }
}

void AAudioZoneManager::TriggerDangerAlert()
{
    if (!bDangerActive)
    {
        bDangerActive = true;
        DangerCooldown = 10.0f;

        // Play danger stinger if assigned
        if (DangerStinger)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DangerStinger, GetActorLocation());
        }
    }
}

void AAudioZoneManager::ClearDangerAlert()
{
    bDangerActive = false;
    DangerCooldown = 0.0f;
}
