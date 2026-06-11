#include "AudioZoneManager.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger volume component
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Set default zone settings
    ZoneSettings.ZoneType = EArch_AudioZoneType::Plains;
    ZoneSettings.Volume = 0.5f;
    ZoneSettings.Pitch = 1.0f;
    ZoneSettings.FadeInTime = 2.0f;
    ZoneSettings.FadeOutTime = 2.0f;
    ZoneSettings.bUseReverb = true;
    ZoneSettings.ReverbIntensity = 1.0f;

    bIsActive = true;
    Priority = 1;
}

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAudioZoneManager::OnTriggerBeginOverlap);
        TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AAudioZoneManager::OnTriggerEndOverlap);
    }

    // Initialize audio settings
    UpdateAudioSettings();
}

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle fading
    if (bIsFading && AudioComponent)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float FadeProgress = (CurrentTime - FadeStartTime) / FadeDuration;
        
        if (FadeProgress >= 1.0f)
        {
            // Fade complete
            bIsFading = false;
            AudioComponent->SetVolumeMultiplier(FadeTargetVolume);
            
            if (FadeTargetVolume <= 0.0f)
            {
                AudioComponent->Stop();
            }
        }
        else
        {
            // Interpolate volume
            float CurrentVolume = FMath::Lerp(FadeStartVolume, FadeTargetVolume, FadeProgress);
            AudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
    }
}

void AAudioZoneManager::ActivateZone()
{
    if (!bIsActive)
    {
        bIsActive = true;
        UpdateAudioSettings();
        
        // If there are actors in the zone, start playing
        if (ActorsInZone.Num() > 0)
        {
            FadeIn(ZoneSettings.FadeInTime);
        }
    }
}

void AAudioZoneManager::DeactivateZone()
{
    if (bIsActive)
    {
        bIsActive = false;
        FadeOut(ZoneSettings.FadeOutTime);
    }
}

void AAudioZoneManager::SetZoneSettings(const FArch_AudioZoneSettings& NewSettings)
{
    ZoneSettings = NewSettings;
    UpdateAudioSettings();
}

void AAudioZoneManager::FadeIn(float Duration)
{
    if (!AudioComponent || !bIsActive)
    {
        return;
    }

    if (!AudioComponent->IsPlaying() && ZoneSettings.AmbientSound.IsValid())
    {
        AudioComponent->SetSound(ZoneSettings.AmbientSound.LoadSynchronous());
        AudioComponent->Play();
    }

    bIsFading = true;
    FadeStartTime = GetWorld()->GetTimeSeconds();
    FadeDuration = Duration;
    FadeStartVolume = AudioComponent->VolumeMultiplier;
    FadeTargetVolume = ZoneSettings.Volume;
}

void AAudioZoneManager::FadeOut(float Duration)
{
    if (!AudioComponent)
    {
        return;
    }

    bIsFading = true;
    FadeStartTime = GetWorld()->GetTimeSeconds();
    FadeDuration = Duration;
    FadeStartVolume = AudioComponent->VolumeMultiplier;
    FadeTargetVolume = 0.0f;
}

void AAudioZoneManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                             bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || !OtherActor)
    {
        return;
    }

    // Check if it's a character (player or NPC)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        ActorsInZone.AddUnique(OtherActor);
        
        // If this is the first actor entering, start audio
        if (ActorsInZone.Num() == 1)
        {
            FadeIn(ZoneSettings.FadeInTime);
        }

        // Trigger Blueprint event
        OnPlayerEnterZone(OtherActor);
    }
}

void AAudioZoneManager::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    ActorsInZone.Remove(OtherActor);
    
    // If no more actors in zone, fade out audio
    if (ActorsInZone.Num() == 0)
    {
        FadeOut(ZoneSettings.FadeOutTime);
    }

    // Trigger Blueprint event
    OnPlayerExitZone(OtherActor);
}

void AAudioZoneManager::UpdateAudioSettings()
{
    if (!AudioComponent)
    {
        return;
    }

    // Set basic audio properties
    AudioComponent->SetVolumeMultiplier(ZoneSettings.Volume);
    AudioComponent->SetPitchMultiplier(ZoneSettings.Pitch);

    // Load and set sound cue if available
    if (ZoneSettings.AmbientSound.IsValid())
    {
        USoundCue* SoundCue = ZoneSettings.AmbientSound.LoadSynchronous();
        AudioComponent->SetSound(SoundCue);
    }

    // Apply reverb settings if enabled
    if (ZoneSettings.bUseReverb)
    {
        ApplyReverbSettings();
    }
}

void AAudioZoneManager::ApplyReverbSettings()
{
    // Apply reverb based on zone type
    switch (ZoneSettings.ZoneType)
    {
        case EArch_AudioZoneType::Cave:
            // Cave reverb: long decay, high reflectivity
            if (AudioComponent)
            {
                // Set reverb parameters for cave acoustics
                // This would typically involve setting up reverb zones or audio volumes
            }
            break;
            
        case EArch_AudioZoneType::Forest:
            // Forest reverb: medium decay, natural absorption
            break;
            
        case EArch_AudioZoneType::Plains:
            // Plains reverb: short decay, open space
            break;
            
        case EArch_AudioZoneType::Structure:
            // Structure reverb: variable based on architecture
            break;
            
        case EArch_AudioZoneType::Water:
            // Water reverb: unique reflection properties
            break;
            
        case EArch_AudioZoneType::Underground:
            // Underground reverb: enclosed, echoing
            break;
    }
}