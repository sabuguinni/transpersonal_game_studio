#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "UObject/ConstructorHelpers.h"

AAudio_AudioManager::AAudio_AudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    RootComponent = MusicComponent;
    MusicComponent->bAutoActivate = false;

    AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
    AmbientComponent->SetupAttachment(RootComponent);
    AmbientComponent->bAutoActivate = false;

    DialogueComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueComponent"));
    DialogueComponent->SetupAttachment(RootComponent);
    DialogueComponent->bAutoActivate = false;

    SFXComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXComponent"));
    SFXComponent->SetupAttachment(RootComponent);
    SFXComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EEng_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentMusicType = EAudio_MusicType::Ambient;
    CurrentWeather = EEng_WeatherType::Clear;
    CurrentTimeOfDay = EEng_TimeOfDay::Day;

    // Volume settings
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 0.8f;
    AmbientVolume = 0.6f;

    // Transition state
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 2.0f;
}

void AAudio_AudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioSystem();
}

void AAudio_AudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= TransitionDuration)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
        
        UpdateAudioMixing();
    }
}

void AAudio_AudioManager::InitializeAudioSystem()
{
    InitializeBiomeConfigs();
    InitializeThreatConfigs();
    
    // Set initial biome to Forest
    SetCurrentBiome(EEng_BiomeType::Forest);
    SetThreatLevel(EAudio_ThreatLevel::Safe);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Audio system initialized for prehistoric survival"));
}

void AAudio_AudioManager::SetCurrentBiome(EEng_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Biome changed from %d to %d"), (int32)CurrentBiome, (int32)NewBiome);
        
        EEng_BiomeType PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        TransitionToNewBiome(NewBiome);
    }
}

void AAudio_AudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Threat level changed from %d to %d"), (int32)CurrentThreatLevel, (int32)NewThreatLevel);
        
        CurrentThreatLevel = NewThreatLevel;
        TransitionToNewThreat(NewThreatLevel);
    }
}

void AAudio_AudioManager::PlayMusicType(EAudio_MusicType MusicType, float FadeTime)
{
    CurrentMusicType = MusicType;
    TransitionDuration = FadeTime;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing music type %d"), (int32)MusicType);
}

void AAudio_AudioManager::PlaySpatialSound(const FString& SoundPath, const FVector& Location, float Volume)
{
    if (!SoundPath.IsEmpty())
    {
        SpawnSpatialAudioComponent(SoundPath, Location, Volume * SFXVolume * MasterVolume);
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing spatial sound at location %s"), *Location.ToString());
    }
}

void AAudio_AudioManager::PlayDinosaurSound(EEng_DinosaurSpecies Species, const FVector& Location, float Distance)
{
    FString SoundPath;
    float VolumeMultiplier = 1.0f;
    
    switch (Species)
    {
        case EEng_DinosaurSpecies::TRex:
            SoundPath = "/Game/Audio/Dinosaurs/TRex_Roar";
            VolumeMultiplier = 1.5f; // Louder for larger dinosaurs
            break;
        case EEng_DinosaurSpecies::Raptor:
            SoundPath = "/Game/Audio/Dinosaurs/Raptor_Growl";
            VolumeMultiplier = 0.8f;
            break;
        case EEng_DinosaurSpecies::Triceratops:
            SoundPath = "/Game/Audio/Dinosaurs/Triceratops_Call";
            VolumeMultiplier = 1.2f;
            break;
        case EEng_DinosaurSpecies::Brachiosaurus:
            SoundPath = "/Game/Audio/Dinosaurs/Brachiosaurus_Call";
            VolumeMultiplier = 1.3f;
            break;
        case EEng_DinosaurSpecies::Stegosaurus:
            SoundPath = "/Game/Audio/Dinosaurs/Stegosaurus_Grunt";
            VolumeMultiplier = 1.0f;
            break;
        default:
            SoundPath = "/Game/Audio/Dinosaurs/Generic_Dinosaur";
            break;
    }
    
    // Distance-based volume attenuation
    float DistanceAttenuation = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.1f, 1.0f);
    float FinalVolume = VolumeMultiplier * DistanceAttenuation;
    
    PlaySpatialSound(SoundPath, Location, FinalVolume);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing %d dinosaur sound at distance %f"), (int32)Species, Distance);
}

void AAudio_AudioManager::PlayFootstepSound(const FVector& Location, bool bIsHeavy)
{
    FString SoundPath = bIsHeavy ? "/Game/Audio/Footsteps/Heavy_Footstep" : "/Game/Audio/Footsteps/Light_Footstep";
    float Volume = bIsHeavy ? 1.2f : 0.8f;
    
    PlaySpatialSound(SoundPath, Location, Volume);
}

void AAudio_AudioManager::PlaySeismicRumble(const FVector& EpicenterLocation, float Magnitude)
{
    FString SoundPath = "/Game/Audio/Environmental/Seismic_Rumble";
    float Volume = FMath::Clamp(Magnitude / 10.0f, 0.3f, 2.0f);
    
    PlaySpatialSound(SoundPath, EpicenterLocation, Volume);
    
    // Add screen shake effect through audio
    if (Magnitude > 5.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Major seismic event - magnitude %f"), Magnitude);
    }
}

void AAudio_AudioManager::UpdateWeatherAudio(EEng_WeatherType WeatherType)
{
    CurrentWeather = WeatherType;
    
    switch (WeatherType)
    {
        case EEng_WeatherType::Rain:
            SetRainIntensity(0.7f);
            SetWindIntensity(0.5f);
            break;
        case EEng_WeatherType::Storm:
            SetRainIntensity(1.0f);
            SetWindIntensity(0.9f);
            break;
        case EEng_WeatherType::Fog:
            SetWindIntensity(0.3f);
            SetRainIntensity(0.0f);
            break;
        case EEng_WeatherType::Clear:
        default:
            SetRainIntensity(0.0f);
            SetWindIntensity(0.2f);
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Weather changed to %d"), (int32)WeatherType);
}

void AAudio_AudioManager::UpdateTimeOfDayAudio(EEng_TimeOfDay TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    
    // Adjust ambient volume based on time of day
    float TimeVolumeMultiplier = 1.0f;
    
    switch (TimeOfDay)
    {
        case EEng_TimeOfDay::Dawn:
            TimeVolumeMultiplier = 0.8f;
            break;
        case EEng_TimeOfDay::Day:
            TimeVolumeMultiplier = 1.0f;
            break;
        case EEng_TimeOfDay::Dusk:
            TimeVolumeMultiplier = 0.7f;
            break;
        case EEng_TimeOfDay::Night:
            TimeVolumeMultiplier = 0.5f;
            break;
    }
    
    if (AmbientComponent)
    {
        AmbientComponent->SetVolumeMultiplier(AmbientVolume * TimeVolumeMultiplier * MasterVolume);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Time of day changed to %d"), (int32)TimeOfDay);
}

void AAudio_AudioManager::SetWindIntensity(float Intensity)
{
    // Implementation for wind audio intensity
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Wind intensity set to %f"), Intensity);
}

void AAudio_AudioManager::SetRainIntensity(float Intensity)
{
    // Implementation for rain audio intensity
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Rain intensity set to %f"), Intensity);
}

void AAudio_AudioManager::PlayDialogueAudio(const FString& AudioPath, float Volume)
{
    if (DialogueComponent && !AudioPath.IsEmpty())
    {
        DialogueComponent->SetVolumeMultiplier(Volume * MasterVolume);
        // Load and play audio file
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing dialogue audio: %s"), *AudioPath);
    }
}

void AAudio_AudioManager::StopDialogueAudio()
{
    if (DialogueComponent)
    {
        DialogueComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Stopped dialogue audio"));
    }
}

bool AAudio_AudioManager::IsDialoguePlaying() const
{
    return DialogueComponent && DialogueComponent->IsPlaying();
}

void AAudio_AudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioMixing();
}

void AAudio_AudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void AAudio_AudioManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (SFXComponent)
    {
        SFXComponent->SetVolumeMultiplier(SFXVolume * MasterVolume);
    }
}

void AAudio_AudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientComponent)
    {
        AmbientComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void AAudio_AudioManager::InitializeBiomeConfigs()
{
    BiomeConfigs.Empty();
    
    // Forest biome
    FAudio_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EEng_BiomeType::Forest;
    ForestConfig.AmbientSoundPath = "/Game/Audio/Ambient/Forest_Ambience";
    ForestConfig.MusicPath = "/Game/Audio/Music/Forest_Exploration";
    ForestConfig.BaseVolume = 0.8f;
    ForestConfig.ReverbIntensity = 0.7f;
    ForestConfig.bHasWind = true;
    ForestConfig.bHasWater = false;
    BiomeConfigs.Add(ForestConfig);
    
    // Swamp biome
    FAudio_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EEng_BiomeType::Swamp;
    SwampConfig.AmbientSoundPath = "/Game/Audio/Ambient/Swamp_Ambience";
    SwampConfig.MusicPath = "/Game/Audio/Music/Swamp_Mystery";
    SwampConfig.BaseVolume = 0.7f;
    SwampConfig.ReverbIntensity = 0.9f;
    SwampConfig.bHasWind = false;
    SwampConfig.bHasWater = true;
    BiomeConfigs.Add(SwampConfig);
    
    // Savanna biome
    FAudio_BiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EEng_BiomeType::Savanna;
    SavannaConfig.AmbientSoundPath = "/Game/Audio/Ambient/Savanna_Ambience";
    SavannaConfig.MusicPath = "/Game/Audio/Music/Savanna_Open";
    SavannaConfig.BaseVolume = 0.6f;
    SavannaConfig.ReverbIntensity = 0.3f;
    SavannaConfig.bHasWind = true;
    SavannaConfig.bHasWater = false;
    BiomeConfigs.Add(SavannaConfig);
    
    // Desert biome
    FAudio_BiomeConfig DesertConfig;
    DesertConfig.BiomeType = EEng_BiomeType::Desert;
    DesertConfig.AmbientSoundPath = "/Game/Audio/Ambient/Desert_Ambience";
    DesertConfig.MusicPath = "/Game/Audio/Music/Desert_Harsh";
    DesertConfig.BaseVolume = 0.5f;
    DesertConfig.ReverbIntensity = 0.2f;
    DesertConfig.bHasWind = true;
    DesertConfig.bHasWater = false;
    BiomeConfigs.Add(DesertConfig);
    
    // Mountain biome
    FAudio_BiomeConfig MountainConfig;
    MountainConfig.BiomeType = EEng_BiomeType::Mountain;
    MountainConfig.AmbientSoundPath = "/Game/Audio/Ambient/Mountain_Ambience";
    MountainConfig.MusicPath = "/Game/Audio/Music/Mountain_Epic";
    MountainConfig.BaseVolume = 0.9f;
    MountainConfig.ReverbIntensity = 1.0f;
    MountainConfig.bHasWind = true;
    MountainConfig.bHasWater = false;
    BiomeConfigs.Add(MountainConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initialized %d biome configurations"), BiomeConfigs.Num());
}

void AAudio_AudioManager::InitializeThreatConfigs()
{
    ThreatConfigs.Empty();
    
    // Safe level
    FAudio_ThreatConfig SafeConfig;
    SafeConfig.ThreatLevel = EAudio_ThreatLevel::Safe;
    SafeConfig.MusicPath = "/Game/Audio/Music/Peaceful_Exploration";
    SafeConfig.MusicVolume = 0.5f;
    SafeConfig.HeartbeatIntensity = 0.0f;
    SafeConfig.TransitionSpeed = 3.0f;
    SafeConfig.bEnableStingers = false;
    ThreatConfigs.Add(SafeConfig);
    
    // Caution level
    FAudio_ThreatConfig CautionConfig;
    CautionConfig.ThreatLevel = EAudio_ThreatLevel::Caution;
    CautionConfig.MusicPath = "/Game/Audio/Music/Tension_Building";
    CautionConfig.MusicVolume = 0.6f;
    CautionConfig.HeartbeatIntensity = 0.2f;
    CautionConfig.TransitionSpeed = 2.0f;
    CautionConfig.bEnableStingers = true;
    ThreatConfigs.Add(CautionConfig);
    
    // Danger level
    FAudio_ThreatConfig DangerConfig;
    DangerConfig.ThreatLevel = EAudio_ThreatLevel::Danger;
    DangerConfig.MusicPath = "/Game/Audio/Music/High_Tension";
    DangerConfig.MusicVolume = 0.8f;
    DangerConfig.HeartbeatIntensity = 0.5f;
    DangerConfig.TransitionSpeed = 1.5f;
    DangerConfig.bEnableStingers = true;
    ThreatConfigs.Add(DangerConfig);
    
    // Critical level
    FAudio_ThreatConfig CriticalConfig;
    CriticalConfig.ThreatLevel = EAudio_ThreatLevel::Critical;
    CriticalConfig.MusicPath = "/Game/Audio/Music/Combat_Intense";
    CriticalConfig.MusicVolume = 1.0f;
    CriticalConfig.HeartbeatIntensity = 1.0f;
    CriticalConfig.TransitionSpeed = 0.5f;
    CriticalConfig.bEnableStingers = true;
    ThreatConfigs.Add(CriticalConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initialized %d threat configurations"), ThreatConfigs.Num());
}

void AAudio_AudioManager::TransitionToNewBiome(EEng_BiomeType NewBiome)
{
    FAudio_BiomeConfig Config = GetBiomeConfig(NewBiome);
    
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionDuration = 3.0f;
    
    // Start ambient audio for new biome
    if (AmbientComponent && !Config.AmbientSoundPath.IsEmpty())
    {
        AmbientComponent->SetVolumeMultiplier(Config.BaseVolume * AmbientVolume * MasterVolume);
    }
}

void AAudio_AudioManager::TransitionToNewThreat(EAudio_ThreatLevel NewThreat)
{
    FAudio_ThreatConfig Config = GetThreatConfig(NewThreat);
    
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionDuration = Config.TransitionSpeed;
    
    // Start threat music
    if (MusicComponent && !Config.MusicPath.IsEmpty())
    {
        MusicComponent->SetVolumeMultiplier(Config.MusicVolume * MusicVolume * MasterVolume);
    }
}

void AAudio_AudioManager::UpdateAudioMixing()
{
    // Update volume mixing based on current state
    float TransitionProgress = bIsTransitioning ? (TransitionTimer / TransitionDuration) : 1.0f;
    
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * TransitionProgress);
    }
    
    if (AmbientComponent)
    {
        AmbientComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume * TransitionProgress);
    }
}

FAudio_BiomeConfig AAudio_AudioManager::GetBiomeConfig(EEng_BiomeType BiomeType)
{
    for (const FAudio_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }
    
    // Return default forest config if not found
    FAudio_BiomeConfig DefaultConfig;
    return DefaultConfig;
}

FAudio_ThreatConfig AAudio_AudioManager::GetThreatConfig(EAudio_ThreatLevel ThreatLevel)
{
    for (const FAudio_ThreatConfig& Config : ThreatConfigs)
    {
        if (Config.ThreatLevel == ThreatLevel)
        {
            return Config;
        }
    }
    
    // Return default safe config if not found
    FAudio_ThreatConfig DefaultConfig;
    return DefaultConfig;
}

void AAudio_AudioManager::SpawnSpatialAudioComponent(const FString& SoundPath, const FVector& Location, float Volume)
{
    if (GetWorld())
    {
        // Create temporary audio component for spatial sound
        UAudioComponent* SpatialAudio = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            nullptr, // Sound cue will be loaded dynamically
            Location,
            FRotator::ZeroRotator,
            Volume
        );
        
        if (SpatialAudio)
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioManager: Spawned spatial audio at %s"), *Location.ToString());
        }
    }
}