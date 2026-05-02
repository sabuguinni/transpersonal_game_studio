#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundSource.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio components
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    RootComponent = AmbienceAudioComponent;
    AmbienceAudioComponent->bAutoActivate = false;
    
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;
    
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);
    NarrativeAudioComponent->bAutoActivate = false;
    
    // Initialize default values
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    MusicVolume = 0.5f;
    SFXVolume = 0.8f;
    
    AudioUpdateTimer = 0.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    
    // Initialize default soundscape
    CurrentSoundscape = FAudio_SoundscapeParameters();
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: BeginPlay started"));
    
    InitializeAudioComponents();
    InitializeDinosaurAudioDatabase();
    
    // Start with forest ambience
    SetBiome(EAudio_BiomeType::Forest);
    SetThreatLevel(EAudio_ThreatLevel::Safe);
    SetTimeOfDay(EAudio_TimeOfDay::Day);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initialization complete"));
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    AudioUpdateTimer += DeltaTime;
    
    if (AudioUpdateTimer >= AudioUpdateInterval)
    {
        ProcessAudioParameters();
        UpdateAdaptiveMusic();
        UpdateAmbienceAudio();
        UpdateMetaSoundParameters();
        
        AudioUpdateTimer = 0.0f;
    }
    
    // Handle crossfade transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
        }
    }
}

void AAudio_MetaSoundManager::InitializeAudioComponents()
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
        AmbienceAudioComponent->SetPitchMultiplier(1.0f);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
        MusicAudioComponent->SetPitchMultiplier(1.0f);
    }
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(SFXVolume * MasterVolume);
        NarrativeAudioComponent->SetPitchMultiplier(1.0f);
    }
}

void AAudio_MetaSoundManager::InitializeDinosaurAudioDatabase()
{
    // Initialize dinosaur audio data for common species
    FAudio_DinosaurAudioData TRexData;
    TRexData.DinosaurType = TEXT("TRex");
    TRexData.VolumeMultiplier = 1.5f;
    TRexData.PitchVariation = 0.2f;
    DinosaurAudioDatabase.Add(TRexData);
    
    FAudio_DinosaurAudioData RaptorData;
    RaptorData.DinosaurType = TEXT("Raptor");
    RaptorData.VolumeMultiplier = 0.8f;
    RaptorData.PitchVariation = 0.3f;
    DinosaurAudioDatabase.Add(RaptorData);
    
    FAudio_DinosaurAudioData BrachiosaurusData;
    BrachiosaurusData.DinosaurType = TEXT("Brachiosaurus");
    BrachiosaurusData.VolumeMultiplier = 2.0f;
    BrachiosaurusData.PitchVariation = 0.1f;
    DinosaurAudioDatabase.Add(BrachiosaurusData);
    
    FAudio_DinosaurAudioData TriceratopsData;
    TriceratopsData.DinosaurType = TEXT("Triceratops");
    TriceratopsData.VolumeMultiplier = 1.2f;
    TriceratopsData.PitchVariation = 0.15f;
    DinosaurAudioDatabase.Add(TriceratopsData);
    
    FAudio_DinosaurAudioData ParasaurolophusData;
    ParasaurolophusData.DinosaurType = TEXT("Parasaurolophus");
    ParasaurolophusData.VolumeMultiplier = 1.0f;
    ParasaurolophusData.PitchVariation = 0.25f;
    DinosaurAudioDatabase.Add(ParasaurolophusData);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Dinosaur audio database initialized with %d entries"), DinosaurAudioDatabase.Num());
}

void AAudio_MetaSoundManager::UpdateSoundscapeParameters(const FAudio_SoundscapeParameters& NewParameters)
{
    bool bBiomeChanged = (CurrentSoundscape.CurrentBiome != NewParameters.CurrentBiome);
    bool bThreatChanged = (CurrentSoundscape.ThreatLevel != NewParameters.ThreatLevel);
    
    CurrentSoundscape = NewParameters;
    
    if (bBiomeChanged)
    {
        CrossfadeToNewBiome(NewParameters.CurrentBiome);
    }
    
    if (bThreatChanged)
    {
        TransitionThreatLevel(NewParameters.ThreatLevel);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Soundscape updated - Biome: %d, Threat: %d, Time: %d"), 
           (int32)NewParameters.CurrentBiome, (int32)NewParameters.ThreatLevel, (int32)NewParameters.TimeOfDay);
}

void AAudio_MetaSoundManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentSoundscape.CurrentBiome != NewBiome)
    {
        CurrentSoundscape.CurrentBiome = NewBiome;
        CrossfadeToNewBiome(NewBiome);
    }
}

void AAudio_MetaSoundManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentSoundscape.ThreatLevel != NewThreatLevel)
    {
        CurrentSoundscape.ThreatLevel = NewThreatLevel;
        TransitionThreatLevel(NewThreatLevel);
    }
}

void AAudio_MetaSoundManager::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    CurrentSoundscape.TimeOfDay = NewTimeOfDay;
    
    // Adjust ambience based on time of day
    float TimeMultiplier = 1.0f;
    switch (NewTimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:
            TimeMultiplier = 0.6f;
            break;
        case EAudio_TimeOfDay::Day:
            TimeMultiplier = 1.0f;
            break;
        case EAudio_TimeOfDay::Dusk:
            TimeMultiplier = 0.8f;
            break;
        case EAudio_TimeOfDay::Night:
            TimeMultiplier = 0.4f;
            break;
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume * TimeMultiplier);
    }
}

void AAudio_MetaSoundManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    // Find dinosaur audio data
    FAudio_DinosaurAudioData* DinosaurData = DinosaurAudioDatabase.FindByPredicate([&DinosaurType](const FAudio_DinosaurAudioData& Data)
    {
        return Data.DinosaurType == DinosaurType;
    });
    
    if (!DinosaurData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Dinosaur type '%s' not found in database"), *DinosaurType);
        return;
    }
    
    USoundBase* SoundToPlay = nullptr;
    
    if (SoundType == TEXT("Idle"))
    {
        SoundToPlay = DinosaurData->IdleSound.LoadSynchronous();
    }
    else if (SoundType == TEXT("Alert"))
    {
        SoundToPlay = DinosaurData->AlertSound.LoadSynchronous();
    }
    else if (SoundType == TEXT("Attack"))
    {
        SoundToPlay = DinosaurData->AttackSound.LoadSynchronous();
    }
    else if (SoundType == TEXT("Footstep"))
    {
        SoundToPlay = DinosaurData->FootstepSound.LoadSynchronous();
    }
    
    if (SoundToPlay)
    {
        float Volume = DinosaurData->VolumeMultiplier * SFXVolume * MasterVolume;
        float Pitch = 1.0f + FMath::RandRange(-DinosaurData->PitchVariation, DinosaurData->PitchVariation);
        
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Location, Volume, Pitch);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Played %s sound for %s at location %s"), 
               *SoundType, *DinosaurType, *Location.ToString());
    }
}

void AAudio_MetaSoundManager::RegisterDinosaurProximity(const FString& DinosaurType, float Distance)
{
    // Update proximity for threat level calculation
    float ProximityFactor = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 1.0f);
    CurrentSoundscape.DinosaurProximity = FMath::Max(CurrentSoundscape.DinosaurProximity, ProximityFactor);
    
    // Adjust threat level based on proximity and dinosaur type
    if (DinosaurType == TEXT("TRex") && Distance < 500.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Panic);
    }
    else if (DinosaurType == TEXT("Raptor") && Distance < 300.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }
    else if (Distance < 200.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Cautious);
    }
}

void AAudio_MetaSoundManager::PlayNarrativeClip(USoundBase* AudioClip)
{
    if (NarrativeAudioComponent && AudioClip)
    {
        NarrativeAudioComponent->SetSound(AudioClip);
        NarrativeAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing narrative audio clip"));
    }
}

void AAudio_MetaSoundManager::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
}

void AAudio_MetaSoundManager::TriggerWeatherAudio(float Intensity)
{
    CurrentSoundscape.WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Weather affects ambience volume and characteristics
    if (AmbienceAudioComponent)
    {
        float WeatherVolume = 1.0f + (Intensity * 0.5f);
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume * WeatherVolume);
    }
}

void AAudio_MetaSoundManager::PlayFootstepAudio(FVector Location, const FString& SurfaceType)
{
    // Simple footstep audio based on surface type
    float Volume = 0.3f * SFXVolume * MasterVolume;
    float Pitch = 1.0f + FMath::RandRange(-0.1f, 0.1f);
    
    // Note: In a full implementation, this would use different sounds for different surface types
    // For now, we log the footstep event
    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_MetaSoundManager: Footstep on %s at %s"), 
           *SurfaceType, *Location.ToString());
}

void AAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all audio components
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(SFXVolume * MasterVolume);
    }
}

void AAudio_MetaSoundManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
}

void AAudio_MetaSoundManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void AAudio_MetaSoundManager::ProcessAudioParameters()
{
    // Decay proximity over time
    CurrentSoundscape.DinosaurProximity = FMath::Max(0.0f, CurrentSoundscape.DinosaurProximity - 0.1f);
    
    // Calculate stress level based on threat and proximity
    float StressFromThreat = static_cast<float>(CurrentSoundscape.ThreatLevel) / 3.0f;
    float StressFromProximity = CurrentSoundscape.DinosaurProximity;
    CurrentSoundscape.PlayerStressLevel = FMath::Clamp(StressFromThreat + StressFromProximity, 0.0f, 1.0f);
}

void AAudio_MetaSoundManager::UpdateAdaptiveMusic()
{
    if (!MusicAudioComponent)
    {
        return;
    }
    
    // Adjust music based on stress level and threat
    float MusicIntensity = CurrentSoundscape.PlayerStressLevel;
    float TargetVolume = MusicVolume * MasterVolume * (0.3f + MusicIntensity * 0.7f);
    
    MusicAudioComponent->SetVolumeMultiplier(TargetVolume);
    
    // Adjust pitch slightly based on threat level
    float ThreatPitch = 1.0f + (static_cast<float>(CurrentSoundscape.ThreatLevel) * 0.05f);
    MusicAudioComponent->SetPitchMultiplier(ThreatPitch);
}

void AAudio_MetaSoundManager::UpdateAmbienceAudio()
{
    if (!AmbienceAudioComponent)
    {
        return;
    }
    
    // Adjust ambience based on biome and time of day
    float BiomeVolume = 1.0f;
    switch (CurrentSoundscape.CurrentBiome)
    {
        case EAudio_BiomeType::Forest:
            BiomeVolume = 0.8f;
            break;
        case EAudio_BiomeType::Swamp:
            BiomeVolume = 1.0f;
            break;
        case EAudio_BiomeType::Savanna:
            BiomeVolume = 0.6f;
            break;
        case EAudio_BiomeType::Desert:
            BiomeVolume = 0.4f;
            break;
        case EAudio_BiomeType::SnowMountain:
            BiomeVolume = 0.3f;
            break;
    }
    
    float TimeVolume = 1.0f;
    switch (CurrentSoundscape.TimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:
            TimeVolume = 0.6f;
            break;
        case EAudio_TimeOfDay::Day:
            TimeVolume = 1.0f;
            break;
        case EAudio_TimeOfDay::Dusk:
            TimeVolume = 0.8f;
            break;
        case EAudio_TimeOfDay::Night:
            TimeVolume = 0.4f;
            break;
    }
    
    float FinalVolume = AmbienceVolume * MasterVolume * BiomeVolume * TimeVolume;
    AmbienceAudioComponent->SetVolumeMultiplier(FinalVolume);
}

void AAudio_MetaSoundManager::UpdateMetaSoundParameters()
{
    // Update MetaSound parameters if MetaSound assets are loaded
    if (AdaptiveMusicMetaSound.IsValid() && MusicAudioComponent)
    {
        SetMetaSoundParameter(MusicAudioComponent, TEXT("ThreatLevel"), static_cast<float>(CurrentSoundscape.ThreatLevel));
        SetMetaSoundParameter(MusicAudioComponent, TEXT("StressLevel"), CurrentSoundscape.PlayerStressLevel);
    }
    
    if (AmbienceMetaSound.IsValid() && AmbienceAudioComponent)
    {
        SetMetaSoundParameter(AmbienceAudioComponent, TEXT("BiomeType"), static_cast<float>(CurrentSoundscape.CurrentBiome));
        SetMetaSoundParameter(AmbienceAudioComponent, TEXT("TimeOfDay"), static_cast<float>(CurrentSoundscape.TimeOfDay));
        SetMetaSoundParameter(AmbienceAudioComponent, TEXT("WeatherIntensity"), CurrentSoundscape.WeatherIntensity);
    }
}

void AAudio_MetaSoundManager::SetMetaSoundParameter(UAudioComponent* AudioComp, const FName& ParameterName, float Value)
{
    if (AudioComp && AudioComp->IsPlaying())
    {
        // Note: This would require proper MetaSound parameter setting in a full implementation
        // For now, we log the parameter change
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_MetaSoundManager: Setting MetaSound parameter %s to %f"), 
               *ParameterName.ToString(), Value);
    }
}

void AAudio_MetaSoundManager::CrossfadeToNewBiome(EAudio_BiomeType NewBiome)
{
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Crossfading to biome %d"), static_cast<int32>(NewBiome));
}

void AAudio_MetaSoundManager::TransitionThreatLevel(EAudio_ThreatLevel NewLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Transitioning to threat level %d"), static_cast<int32>(NewLevel));
    
    // Immediate response to threat level changes
    UpdateAdaptiveMusic();
}