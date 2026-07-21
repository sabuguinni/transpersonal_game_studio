#include "MetaSoundAudioManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AMetaSoundAudioManager::AMetaSoundAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    SFXAudioComponent->SetupAttachment(RootComponent);
    SFXAudioComponent->bAutoActivate = false;

    // Create proximity detection sphere
    ProximityDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityDetectionSphere"));
    ProximityDetectionSphere->SetupAttachment(RootComponent);
    ProximityDetectionSphere->SetSphereRadius(1500.0f);
    ProximityDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProximityDetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    ProximityDetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    ProximityDetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize default values
    MasterVolume = 1.0f;
    ProximityRadius = 1500.0f;
    bEnableProximityAudio = true;
    bEnableDynamicMixing = true;
    CurrentTensionLevel = 0.0f;
    CurrentExplorationLevel = 1.0f;
    bIsTransitioning = false;

    // Setup default audio zones
    FAudio_SoundZoneData ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.VolumeMultiplier = 0.7f;
    ForestZone.FadeInTime = 3.0f;
    ForestZone.FadeOutTime = 2.0f;
    AudioZones.Add(ForestZone);

    FAudio_SoundZoneData PlainsZone;
    PlainsZone.ZoneName = TEXT("Plains");
    PlainsZone.VolumeMultiplier = 0.5f;
    PlainsZone.FadeInTime = 4.0f;
    PlainsZone.FadeOutTime = 3.0f;
    AudioZones.Add(PlainsZone);

    FAudio_SoundZoneData CaveZone;
    CaveZone.ZoneName = TEXT("Cave");
    CaveZone.VolumeMultiplier = 0.3f;
    CaveZone.FadeInTime = 2.0f;
    CaveZone.FadeOutTime = 1.5f;
    AudioZones.Add(CaveZone);
}

void AMetaSoundAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    SetupProximityDetection();
    
    // Start with forest ambient by default
    PlayAmbientSound(TEXT("Forest"));
}

void AMetaSoundAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicMixing && !bIsTransitioning)
    {
        // Update dynamic audio mixing based on game state
        UpdateDynamicMix(CurrentTensionLevel, CurrentExplorationLevel);
    }
}

void AMetaSoundAudioManager::InitializeAudioComponents()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * 0.7f);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * 0.5f);
    }

    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void AMetaSoundAudioManager::SetupProximityDetection()
{
    if (ProximityDetectionSphere && bEnableProximityAudio)
    {
        ProximityDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMetaSoundAudioManager::OnProximityEnter);
        ProximityDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AMetaSoundAudioManager::OnProximityExit);
        ProximityDetectionSphere->SetSphereRadius(ProximityRadius);
    }
}

void AMetaSoundAudioManager::PlayAmbientSound(const FString& ZoneName)
{
    FAudio_SoundZoneData* ZoneData = FindAudioZone(ZoneName);
    if (ZoneData && AmbientAudioComponent)
    {
        CurrentZoneData = *ZoneData;
        
        // Fade out current sound if playing
        if (AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(ZoneData->FadeOutTime, 0.0f);
        }

        // Set new volume and fade in
        float TargetVolume = MasterVolume * ZoneData->VolumeMultiplier;
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
        AmbientAudioComponent->Play();
        AmbientAudioComponent->FadeIn(ZoneData->FadeInTime, TargetVolume);

        UE_LOG(LogTemp, Log, TEXT("MetaSoundAudioManager: Playing ambient sound for zone %s"), *ZoneName);
    }
}

void AMetaSoundAudioManager::StopAmbientSound()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(CurrentZoneData.FadeOutTime, 0.0f);
    }
}

void AMetaSoundAudioManager::PlayFootstepSound(EAudio_SurfaceType SurfaceType, float Volume)
{
    if (SFXAudioComponent)
    {
        // Different footstep sounds based on surface type
        float VolumeMultiplier = Volume * MasterVolume;
        
        switch (SurfaceType)
        {
            case EAudio_SurfaceType::Dirt:
                VolumeMultiplier *= 0.8f;
                break;
            case EAudio_SurfaceType::Grass:
                VolumeMultiplier *= 0.6f;
                break;
            case EAudio_SurfaceType::Rock:
                VolumeMultiplier *= 1.2f;
                break;
            case EAudio_SurfaceType::Water:
                VolumeMultiplier *= 0.9f;
                break;
            case EAudio_SurfaceType::Sand:
                VolumeMultiplier *= 0.5f;
                break;
            case EAudio_SurfaceType::Wood:
                VolumeMultiplier *= 1.0f;
                break;
        }

        SFXAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        SFXAudioComponent->Play();
    }
}

void AMetaSoundAudioManager::PlayProximityWarning(const FString& ThreatType)
{
    if (SFXAudioComponent)
    {
        // Play warning sound based on threat type
        float WarningVolume = MasterVolume * 0.9f;
        
        if (ThreatType == TEXT("TRex"))
        {
            WarningVolume *= 1.2f; // Louder for T-Rex
        }
        else if (ThreatType == TEXT("Raptor"))
        {
            WarningVolume *= 0.8f; // Quieter for smaller predators
        }

        SFXAudioComponent->SetVolumeMultiplier(WarningVolume);
        SFXAudioComponent->Play();

        UE_LOG(LogTemp, Warning, TEXT("MetaSoundAudioManager: Playing proximity warning for %s"), *ThreatType);
    }
}

void AMetaSoundAudioManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all audio component volumes
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * CurrentZoneData.VolumeMultiplier);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * 0.5f);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void AMetaSoundAudioManager::FadeToZone(const FString& ZoneName, float FadeTime)
{
    FAudio_SoundZoneData* NewZoneData = FindAudioZone(ZoneName);
    if (NewZoneData && !bIsTransitioning)
    {
        bIsTransitioning = true;
        
        // Fade out current zone
        if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(FadeTime * 0.5f, 0.0f);
        }

        // Set timer to start new zone
        GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, [this, ZoneName]()
        {
            PlayAmbientSound(ZoneName);
            bIsTransitioning = false;
        }, FadeTime * 0.6f, false);
    }
}

void AMetaSoundAudioManager::UpdateDynamicMix(float TensionLevel, float ExplorationLevel)
{
    CurrentTensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    CurrentExplorationLevel = FMath::Clamp(ExplorationLevel, 0.0f, 1.0f);

    // Adjust ambient volume based on tension
    if (AmbientAudioComponent)
    {
        float AmbientVolume = MasterVolume * CurrentZoneData.VolumeMultiplier * (1.0f - CurrentTensionLevel * 0.3f);
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
    }

    // Adjust music volume based on exploration
    if (MusicAudioComponent)
    {
        float MusicVolume = MasterVolume * 0.5f * CurrentExplorationLevel;
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
    }
}

void AMetaSoundAudioManager::OnProximityEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                            UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                            bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->GetName().Contains(TEXT("TRex")))
    {
        PlayProximityWarning(TEXT("TRex"));
        UpdateDynamicMix(1.0f, CurrentExplorationLevel); // Max tension
    }
    else if (OtherActor && OtherActor->GetName().Contains(TEXT("Raptor")))
    {
        PlayProximityWarning(TEXT("Raptor"));
        UpdateDynamicMix(0.7f, CurrentExplorationLevel); // High tension
    }
}

void AMetaSoundAudioManager::OnProximityExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                           UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor && (OtherActor->GetName().Contains(TEXT("TRex")) || OtherActor->GetName().Contains(TEXT("Raptor"))))
    {
        // Gradually reduce tension when threat moves away
        UpdateDynamicMix(0.2f, CurrentExplorationLevel);
    }
}

FAudio_SoundZoneData* AMetaSoundAudioManager::FindAudioZone(const FString& ZoneName)
{
    for (FAudio_SoundZoneData& Zone : AudioZones)
    {
        if (Zone.ZoneName.Equals(ZoneName, ESearchCase::IgnoreCase))
        {
            return &Zone;
        }
    }
    return nullptr;
}