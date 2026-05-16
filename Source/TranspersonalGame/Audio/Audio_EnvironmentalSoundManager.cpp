#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_EnvironmentalSoundManager::UAudio_EnvironmentalSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savanna;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentTimeOfDay = 0.5f; // Noon
    CurrentWeatherIntensity = 0.0f; // Clear weather

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    CreatureAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CreatureAudio"));
    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudio"));

    // Configure audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->VolumeMultiplier = 0.7f;
    }

    if (WindAudioComponent)
    {
        WindAudioComponent->bAutoActivate = false;
        WindAudioComponent->VolumeMultiplier = 0.5f;
    }

    if (CreatureAudioComponent)
    {
        CreatureAudioComponent->bAutoActivate = false;
        CreatureAudioComponent->VolumeMultiplier = 0.6f;
    }

    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->bAutoActivate = false;
        ThreatAudioComponent->VolumeMultiplier = 1.0f;
    }
}

void UAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeBiomeProfiles();
    InitializeThreatSounds();

    // Start periodic audio updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(AudioUpdateTimer, this, 
            &UAudio_EnvironmentalSoundManager::PeriodicAudioUpdate, 2.0f, true);
    }

    // Initialize with current biome
    SetCurrentBiome(CurrentBiome);
}

void UAudio_EnvironmentalSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update volume based on current threat level
    UpdateVolumeBasedOnThreat();
}

void UAudio_EnvironmentalSoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }

    CurrentBiome = NewBiome;
    UpdateAmbientAudio();

    UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Switched to biome %d"), (int32)NewBiome);
}

void UAudio_EnvironmentalSoundManager::UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
    {
        return;
    }

    EAudio_ThreatLevel PreviousThreatLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewThreatLevel;

    // Log threat level change
    UE_LOG(LogTemp, Warning, TEXT("Environmental Audio: Threat level changed from %d to %d"), 
           (int32)PreviousThreatLevel, (int32)NewThreatLevel);

    // Immediate volume adjustment
    UpdateVolumeBasedOnThreat();
}

void UAudio_EnvironmentalSoundManager::PlayThreatSound(const FString& ThreatType, FVector Location)
{
    if (FAudio_ThreatAudioData* ThreatData = ThreatSounds.Find(ThreatType))
    {
        if (ThreatAudioComponent && ThreatData->ThreatSound.IsValid())
        {
            // Stop current threat sound if playing
            if (ThreatAudioComponent->IsPlaying())
            {
                ThreatAudioComponent->Stop();
            }

            // Load and play the threat sound
            if (USoundCue* SoundCue = ThreatData->ThreatSound.LoadSynchronous())
            {
                ThreatAudioComponent->SetSound(SoundCue);
                ThreatAudioComponent->SetVolumeMultiplier(ThreatData->Volume);
                ThreatAudioComponent->SetWorldLocation(Location);
                ThreatAudioComponent->Play();

                UE_LOG(LogTemp, Warning, TEXT("Environmental Audio: Playing threat sound %s at location %s"), 
                       *ThreatType, *Location.ToString());
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Environmental Audio: Threat sound %s not found"), *ThreatType);
    }
}

void UAudio_EnvironmentalSoundManager::StopAllThreatSounds()
{
    if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
    {
        ThreatAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Stopped all threat sounds"));
    }
}

void UAudio_EnvironmentalSoundManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);

    // Adjust ambient volumes based on time of day
    float NightVolume = 1.0f;
    if (CurrentTimeOfDay > 0.25f && CurrentTimeOfDay < 0.75f) // Day time
    {
        NightVolume = 0.6f;
    }
    else // Night time
    {
        NightVolume = 1.2f; // Night sounds are more prominent
    }

    if (CreatureAudioComponent)
    {
        CreatureAudioComponent->SetVolumeMultiplier(0.6f * NightVolume);
    }
}

void UAudio_EnvironmentalSoundManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Increase wind volume during storms
    if (WindAudioComponent)
    {
        float WindVolume = 0.5f + (CurrentWeatherIntensity * 0.8f);
        WindAudioComponent->SetVolumeMultiplier(WindVolume);
    }
}

void UAudio_EnvironmentalSoundManager::InitializeBiomeProfiles()
{
    // Initialize Savanna profile
    FAudio_BiomeAudioProfile SavannaProfile;
    SavannaProfile.BaseVolume = 0.7f;
    SavannaProfile.ThreatVolumeMultiplier = 1.3f;
    BiomeProfiles.Add(EAudio_BiomeType::Savanna, SavannaProfile);

    // Initialize Swamp profile
    FAudio_BiomeAudioProfile SwampProfile;
    SwampProfile.BaseVolume = 0.8f;
    SwampProfile.ThreatVolumeMultiplier = 1.5f;
    BiomeProfiles.Add(EAudio_BiomeType::Swamp, SwampProfile);

    // Initialize Forest profile
    FAudio_BiomeAudioProfile ForestProfile;
    ForestProfile.BaseVolume = 0.9f;
    ForestProfile.ThreatVolumeMultiplier = 1.4f;
    BiomeProfiles.Add(EAudio_BiomeType::Forest, ForestProfile);

    // Initialize Desert profile
    FAudio_BiomeAudioProfile DesertProfile;
    DesertProfile.BaseVolume = 0.5f;
    DesertProfile.ThreatVolumeMultiplier = 1.6f;
    BiomeProfiles.Add(EAudio_BiomeType::Desert, DesertProfile);

    // Initialize Mountain profile
    FAudio_BiomeAudioProfile MountainProfile;
    MountainProfile.BaseVolume = 0.6f;
    MountainProfile.ThreatVolumeMultiplier = 1.7f;
    BiomeProfiles.Add(EAudio_BiomeType::Mountain, MountainProfile);

    UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Initialized %d biome profiles"), BiomeProfiles.Num());
}

void UAudio_EnvironmentalSoundManager::InitializeThreatSounds()
{
    // T-Rex threat
    FAudio_ThreatAudioData TRexThreat;
    TRexThreat.TriggerDistance = 3000.0f;
    TRexThreat.Volume = 1.2f;
    TRexThreat.bLooping = false;
    ThreatSounds.Add(TEXT("TRex"), TRexThreat);

    // Raptor pack threat
    FAudio_ThreatAudioData RaptorThreat;
    RaptorThreat.TriggerDistance = 2000.0f;
    RaptorThreat.Volume = 1.0f;
    RaptorThreat.bLooping = true;
    ThreatSounds.Add(TEXT("RaptorPack"), RaptorThreat);

    // Herbivore stampede
    FAudio_ThreatAudioData StampedeThreat;
    StampedeThreat.TriggerDistance = 4000.0f;
    StampedeThreat.Volume = 1.5f;
    StampedeThreat.bLooping = false;
    ThreatSounds.Add(TEXT("Stampede"), StampedeThreat);

    // Environmental danger (storm, earthquake)
    FAudio_ThreatAudioData EnvironmentalThreat;
    EnvironmentalThreat.TriggerDistance = 5000.0f;
    EnvironmentalThreat.Volume = 0.8f;
    EnvironmentalThreat.bLooping = true;
    ThreatSounds.Add(TEXT("Environmental"), EnvironmentalThreat);

    UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Initialized %d threat sound types"), ThreatSounds.Num());
}

void UAudio_EnvironmentalSoundManager::UpdateAmbientAudio()
{
    if (FAudio_BiomeAudioProfile* Profile = BiomeProfiles.Find(CurrentBiome))
    {
        // Update ambient audio component
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(Profile->BaseVolume * CalculateVolumeMultiplier());
        }

        UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Updated ambient audio for biome %d"), (int32)CurrentBiome);
    }
}

void UAudio_EnvironmentalSoundManager::UpdateVolumeBasedOnThreat()
{
    float ThreatMultiplier = CalculateVolumeMultiplier();

    if (FAudio_BiomeAudioProfile* Profile = BiomeProfiles.Find(CurrentBiome))
    {
        float FinalVolume = Profile->BaseVolume * ThreatMultiplier;

        // Apply to all audio components
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
        }

        if (WindAudioComponent)
        {
            WindAudioComponent->SetVolumeMultiplier(FinalVolume * 0.8f);
        }

        if (CreatureAudioComponent)
        {
            CreatureAudioComponent->SetVolumeMultiplier(FinalVolume * 0.9f);
        }
    }
}

float UAudio_EnvironmentalSoundManager::CalculateVolumeMultiplier() const
{
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            return 1.0f;
        case EAudio_ThreatLevel::Caution:
            return 1.2f;
        case EAudio_ThreatLevel::Danger:
            return 1.5f;
        case EAudio_ThreatLevel::Extreme:
            return 2.0f;
        default:
            return 1.0f;
    }
}

void UAudio_EnvironmentalSoundManager::PeriodicAudioUpdate()
{
    // Periodic check for audio state consistency
    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        // Restart ambient audio if it stopped unexpectedly
        UpdateAmbientAudio();
    }

    // Log current audio state for debugging
    UE_LOG(LogTemp, VeryVerbose, TEXT("Environmental Audio: Biome=%d, Threat=%d, Time=%.2f, Weather=%.2f"), 
           (int32)CurrentBiome, (int32)CurrentThreatLevel, CurrentTimeOfDay, CurrentWeatherIntensity);
}