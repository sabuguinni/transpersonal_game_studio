#include "Audio_SoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_SoundManager::AAudio_SoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);
    MasterAudioComponent->bAutoActivate = true;

    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    AmbienceAudioComponent->SetupAttachment(RootComponent);
    AmbienceAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savana;
    bIsNightTime = false;
    bIsWeatherActive = false;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 5.0f;

    // Initialize volume settings
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    MusicVolume = 0.5f;
    SFXVolume = 0.8f;

    // Initialize default dinosaur sound profiles
    FAudio_SoundProfile TRexProfile;
    TRexProfile.VolumeMultiplier = 1.5f;
    TRexProfile.PitchMultiplier = 0.8f;
    TRexProfile.AttenuationRadius = 10000.0f;
    DinosaurSoundProfiles.Add(EAudio_DinosaurType::TRex, TRexProfile);

    FAudio_SoundProfile VelociraptorProfile;
    VelociraptorProfile.VolumeMultiplier = 1.0f;
    VelociraptorProfile.PitchMultiplier = 1.2f;
    VelociraptorProfile.AttenuationRadius = 3000.0f;
    DinosaurSoundProfiles.Add(EAudio_DinosaurType::Velociraptor, VelociraptorProfile);

    FAudio_SoundProfile TriceratopsProfile;
    TriceratopsProfile.VolumeMultiplier = 1.3f;
    TriceratopsProfile.PitchMultiplier = 0.9f;
    TriceratopsProfile.AttenuationRadius = 7000.0f;
    DinosaurSoundProfiles.Add(EAudio_DinosaurType::Triceratops, TriceratopsProfile);

    FAudio_SoundProfile BrachiosaurusProfile;
    BrachiosaurusProfile.VolumeMultiplier = 2.0f;
    BrachiosaurusProfile.PitchMultiplier = 0.6f;
    BrachiosaurusProfile.AttenuationRadius = 15000.0f;
    DinosaurSoundProfiles.Add(EAudio_DinosaurType::Brachiosaurus, BrachiosaurusProfile);

    FAudio_SoundProfile AnkylosaurusProfile;
    AnkylosaurusProfile.VolumeMultiplier = 1.1f;
    AnkylosaurusProfile.PitchMultiplier = 0.85f;
    AnkylosaurusProfile.AttenuationRadius = 5000.0f;
    DinosaurSoundProfiles.Add(EAudio_DinosaurType::Ankylosaurus, AnkylosaurusProfile);

    // Initialize default biome ambience settings
    FAudio_BiomeAmbience SavanaAmbience;
    SavanaAmbience.BiomeType = EAudio_BiomeType::Savana;
    SavanaAmbience.TransitionDuration = 5.0f;
    BiomeAmbienceSettings.Add(SavanaAmbience);

    FAudio_BiomeAmbience PantanoAmbience;
    PantanoAmbience.BiomeType = EAudio_BiomeType::Pantano;
    PantanoAmbience.TransitionDuration = 6.0f;
    BiomeAmbienceSettings.Add(PantanoAmbience);

    FAudio_BiomeAmbience FlorestaAmbience;
    FlorestaAmbience.BiomeType = EAudio_BiomeType::Floresta;
    FlorestaAmbience.TransitionDuration = 4.0f;
    BiomeAmbienceSettings.Add(FlorestaAmbience);

    FAudio_BiomeAmbience DesertoAmbience;
    DesertoAmbience.BiomeType = EAudio_BiomeType::Deserto;
    DesertoAmbience.TransitionDuration = 7.0f;
    BiomeAmbienceSettings.Add(DesertoAmbience);

    FAudio_BiomeAmbience MontanhaAmbience;
    MontanhaAmbience.BiomeType = EAudio_BiomeType::Montanha;
    MontanhaAmbience.TransitionDuration = 5.5f;
    BiomeAmbienceSettings.Add(MontanhaAmbience);
}

void AAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with default biome ambience
    PlayAmbienceForBiome(CurrentBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: BeginPlay completed"));
}

void AAudio_SoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle audio transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

        if (AmbienceAudioComponent && MusicAudioComponent)
        {
            // Crossfade between previous and target ambience
            float PreviousVolume = (1.0f - TransitionProgress) * AmbienceVolume;
            float TargetVolume = TransitionProgress * AmbienceVolume;

            AmbienceAudioComponent->SetVolumeMultiplier(PreviousVolume);
            MusicAudioComponent->SetVolumeMultiplier(TargetVolume);
        }

        if (TransitionProgress >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            
            // Complete the transition
            if (AmbienceAudioComponent && TargetAmbience)
            {
                AmbienceAudioComponent->SetSound(TargetAmbience);
                AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume);
                AmbienceAudioComponent->Play();
            }
            
            if (MusicAudioComponent)
            {
                MusicAudioComponent->Stop();
            }
        }
    }
}

void AAudio_SoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        PlayAmbienceForBiome(NewBiome);
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Changed to biome %d"), (int32)NewBiome);
    }
}

void AAudio_SoundManager::SetTimeOfDay(bool bNightTime)
{
    if (bIsNightTime != bNightTime)
    {
        bIsNightTime = bNightTime;
        UpdateAmbienceAudio();
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Time of day changed to %s"), bNightTime ? TEXT("Night") : TEXT("Day"));
    }
}

void AAudio_SoundManager::SetWeatherState(bool bWeatherActive)
{
    if (bIsWeatherActive != bWeatherActive)
    {
        bIsWeatherActive = bWeatherActive;
        UpdateAmbienceAudio();
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Weather state changed to %s"), bWeatherActive ? TEXT("Active") : TEXT("Inactive"));
    }
}

void AAudio_SoundManager::PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, bool bIsFootstep)
{
    FAudio_SoundProfile SoundProfile = GetDinosaurSoundProfile(DinosaurType);
    
    USoundCue* SoundToPlay = nullptr;
    if (bIsFootstep && SoundProfile.FootstepSound.LoadSynchronous())
    {
        SoundToPlay = SoundProfile.FootstepSound.LoadSynchronous();
    }
    else if (!bIsFootstep && SoundProfile.VocalizationSound.LoadSynchronous())
    {
        SoundToPlay = SoundProfile.VocalizationSound.LoadSynchronous();
    }

    if (SoundToPlay)
    {
        float FinalVolume = SFXVolume * SoundProfile.VolumeMultiplier * MasterVolume;
        float FinalPitch = SoundProfile.PitchMultiplier;

        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            Location,
            FinalVolume,
            FinalPitch,
            0.0f,
            nullptr,
            nullptr,
            true
        );

        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Played dinosaur sound for type %d at location %s"), 
               (int32)DinosaurType, *Location.ToString());
    }
}

void AAudio_SoundManager::PlayAmbienceForBiome(EAudio_BiomeType BiomeType)
{
    FAudio_BiomeAmbience BiomeSettings = GetBiomeAmbienceSettings(BiomeType);
    
    USoundCue* AmbienceToPlay = nullptr;
    if (bIsNightTime && BiomeSettings.NightAmbience.LoadSynchronous())
    {
        AmbienceToPlay = BiomeSettings.NightAmbience.LoadSynchronous();
    }
    else if (bIsWeatherActive && BiomeSettings.WeatherAmbience.LoadSynchronous())
    {
        AmbienceToPlay = BiomeSettings.WeatherAmbience.LoadSynchronous();
    }
    else if (BiomeSettings.DayAmbience.LoadSynchronous())
    {
        AmbienceToPlay = BiomeSettings.DayAmbience.LoadSynchronous();
    }

    if (AmbienceToPlay && AmbienceAudioComponent)
    {
        CrossfadeToNewAmbience(AmbienceToPlay);
    }
}

void AAudio_SoundManager::StopAllAmbience()
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->Stop();
    }
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
    bIsTransitioning = false;
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Stopped all ambience"));
}

void AAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAmbienceAudio();
}

void AAudio_SoundManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
}

void AAudio_SoundManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void AAudio_SoundManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void AAudio_SoundManager::UpdateAmbienceAudio()
{
    PlayAmbienceForBiome(CurrentBiome);
}

void AAudio_SoundManager::CrossfadeToNewAmbience(USoundCue* NewAmbience)
{
    if (!NewAmbience || !AmbienceAudioComponent)
    {
        return;
    }

    TargetAmbience = NewAmbience;
    
    if (AmbienceAudioComponent->IsPlaying())
    {
        // Start crossfade transition
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
        TransitionDuration = GetBiomeAmbienceSettings(CurrentBiome).TransitionDuration;
        
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetSound(NewAmbience);
            MusicAudioComponent->SetVolumeMultiplier(0.0f);
            MusicAudioComponent->Play();
        }
    }
    else
    {
        // Direct play if nothing is currently playing
        AmbienceAudioComponent->SetSound(NewAmbience);
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
        AmbienceAudioComponent->Play();
    }
}

FAudio_SoundProfile AAudio_SoundManager::GetDinosaurSoundProfile(EAudio_DinosaurType DinosaurType)
{
    if (DinosaurSoundProfiles.Contains(DinosaurType))
    {
        return DinosaurSoundProfiles[DinosaurType];
    }
    
    // Return default profile if not found
    FAudio_SoundProfile DefaultProfile;
    return DefaultProfile;
}

FAudio_BiomeAmbience AAudio_SoundManager::GetBiomeAmbienceSettings(EAudio_BiomeType BiomeType)
{
    for (const FAudio_BiomeAmbience& Settings : BiomeAmbienceSettings)
    {
        if (Settings.BiomeType == BiomeType)
        {
            return Settings;
        }
    }
    
    // Return default settings if not found
    FAudio_BiomeAmbience DefaultSettings;
    DefaultSettings.BiomeType = BiomeType;
    return DefaultSettings;
}