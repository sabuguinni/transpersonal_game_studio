#include "Audio_AdaptiveManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

UAudio_AdaptiveManager::UAudio_AdaptiveManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentIntensityLevel = EAudio_IntensityLevel::Calm;
    IntensityChangeThreshold = 0.1f;
    MusicUpdateInterval = 0.5f;
    LastMusicUpdateTime = 0.0f;
    CurrentFearLevel = 0.0f;
    bIsNearLargeDinosaur = false;
    DinosaurProximityDistance = 1000.0f;
    CurrentBiome = EAudio_BiomeType::Forest;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));

    // Configure audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(0.6f);
    }

    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->bAutoActivate = false;
        AmbienceAudioComponent->SetVolumeMultiplier(0.8f);
    }

    if (SFXAudioComponent)
    {
        SFXAudioComponent->bAutoActivate = false;
        SFXAudioComponent->SetVolumeMultiplier(1.0f);
    }

    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->bAutoActivate = false;
        VoiceAudioComponent->SetVolumeMultiplier(0.9f);
    }

    // Initialize default music layers
    MusicLayers.Empty();
    
    // Calm layer
    FAudio_MusicLayer CalmLayer;
    CalmLayer.IntensityLevel = EAudio_IntensityLevel::Calm;
    CalmLayer.BaseVolume = 0.4f;
    CalmLayer.CrossfadeTime = 3.0f;
    MusicLayers.Add(CalmLayer);

    // Tense layer
    FAudio_MusicLayer TenseLayer;
    TenseLayer.IntensityLevel = EAudio_IntensityLevel::Tense;
    TenseLayer.BaseVolume = 0.6f;
    TenseLayer.CrossfadeTime = 2.0f;
    MusicLayers.Add(TenseLayer);

    // Danger layer
    FAudio_MusicLayer DangerLayer;
    DangerLayer.IntensityLevel = EAudio_IntensityLevel::Danger;
    DangerLayer.BaseVolume = 0.8f;
    DangerLayer.CrossfadeTime = 1.5f;
    MusicLayers.Add(DangerLayer);

    // Combat layer
    FAudio_MusicLayer CombatLayer;
    CombatLayer.IntensityLevel = EAudio_IntensityLevel::Combat;
    CombatLayer.BaseVolume = 0.9f;
    CombatLayer.CrossfadeTime = 1.0f;
    MusicLayers.Add(CombatLayer);

    // Terror layer
    FAudio_MusicLayer TerrorLayer;
    TerrorLayer.IntensityLevel = EAudio_IntensityLevel::Terror;
    TerrorLayer.BaseVolume = 1.0f;
    TerrorLayer.CrossfadeTime = 0.5f;
    MusicLayers.Add(TerrorLayer);
}

void UAudio_AdaptiveManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveManager: BeginPlay - Adaptive audio system initialized"));
    
    // Set default biome ambience
    SetBiomeAmbience(EAudio_BiomeType::Forest);
    
    // Start with calm music
    SetIntensityLevel(EAudio_IntensityLevel::Calm);
}

void UAudio_AdaptiveManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAdaptiveMusic(DeltaTime);
    CleanupFinishedAudioEvents();
}

void UAudio_AdaptiveManager::InitializeAudioComponents()
{
    if (!MusicAudioComponent || !AmbienceAudioComponent || !SFXAudioComponent || !VoiceAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_AdaptiveManager: Failed to initialize audio components"));
        return;
    }

    // Configure spatial audio settings
    MusicAudioComponent->bAllowSpatialization = false; // Music is non-spatial
    AmbienceAudioComponent->bAllowSpatialization = false; // Ambience is non-spatial
    SFXAudioComponent->bAllowSpatialization = true; // SFX can be spatial
    VoiceAudioComponent->bAllowSpatialization = false; // Voice is non-spatial

    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Audio components initialized successfully"));
}

void UAudio_AdaptiveManager::SetIntensityLevel(EAudio_IntensityLevel NewLevel)
{
    if (CurrentIntensityLevel == NewLevel)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveManager: Intensity level changed from %d to %d"), 
           (int32)CurrentIntensityLevel, (int32)NewLevel);

    CurrentIntensityLevel = NewLevel;

    // Find the appropriate music layer
    for (const FAudio_MusicLayer& Layer : MusicLayers)
    {
        if (Layer.IntensityLevel == NewLevel)
        {
            CrossfadeToMusicLayer(Layer);
            break;
        }
    }
}

void UAudio_AdaptiveManager::UpdateMusicBasedOnFear(float FearLevel)
{
    CurrentFearLevel = FearLevel;
    
    EAudio_IntensityLevel NewIntensity = CalculateIntensityFromFear(FearLevel);
    SetIntensityLevel(NewIntensity);
}

EAudio_IntensityLevel UAudio_AdaptiveManager::CalculateIntensityFromFear(float FearLevel)
{
    // Map fear level (0.0 to 1.0) to intensity levels
    if (FearLevel >= 0.8f)
    {
        return EAudio_IntensityLevel::Terror;
    }
    else if (FearLevel >= 0.6f)
    {
        return EAudio_IntensityLevel::Combat;
    }
    else if (FearLevel >= 0.4f)
    {
        return EAudio_IntensityLevel::Danger;
    }
    else if (FearLevel >= 0.2f)
    {
        return EAudio_IntensityLevel::Tense;
    }
    else
    {
        return EAudio_IntensityLevel::Calm;
    }
}

void UAudio_AdaptiveManager::TriggerDinosaurProximityMusic(float Distance, bool bIsLargeDinosaur)
{
    bIsNearLargeDinosaur = bIsLargeDinosaur;
    DinosaurProximityDistance = Distance;

    // Calculate intensity based on proximity and dinosaur size
    float ProximityFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    float SizeFactor = bIsLargeDinosaur ? 1.5f : 1.0f;
    
    float EffectiveFear = FMath::Clamp(CurrentFearLevel + (ProximityFactor * SizeFactor * 0.4f), 0.0f, 1.0f);
    
    UpdateMusicBasedOnFear(EffectiveFear);

    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Dinosaur proximity - Distance: %.1f, Large: %s, Fear: %.2f"), 
           Distance, bIsLargeDinosaur ? TEXT("Yes") : TEXT("No"), EffectiveFear);
}

void UAudio_AdaptiveManager::SetBiomeAmbience(EAudio_BiomeType BiomeType)
{
    if (CurrentBiome == BiomeType)
    {
        return;
    }

    CurrentBiome = BiomeType;

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveManager: Biome changed to %d"), (int32)BiomeType);

    // TODO: Load and play biome-specific ambience
    // For now, just log the change
    if (AmbienceAudioComponent && AmbienceAudioComponent->IsPlaying())
    {
        AmbienceAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void UAudio_AdaptiveManager::UpdateAdaptiveMusic(float DeltaTime)
{
    LastMusicUpdateTime += DeltaTime;
    
    if (LastMusicUpdateTime >= MusicUpdateInterval)
    {
        LastMusicUpdateTime = 0.0f;
        
        // Update music based on current game state
        // This is called regularly to ensure music stays in sync with gameplay
    }
}

void UAudio_AdaptiveManager::CrossfadeToMusicLayer(const FAudio_MusicLayer& NewLayer)
{
    if (!MusicAudioComponent)
    {
        return;
    }

    // Fade out current music
    if (MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(NewLayer.CrossfadeTime, 0.0f);
    }

    // TODO: Load and fade in new music track
    // For now, just adjust volume based on intensity
    float VolumeMultiplier = NewLayer.BaseVolume;
    MusicAudioComponent->SetVolumeMultiplier(VolumeMultiplier);

    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Crossfaded to intensity level %d with volume %.2f"), 
           (int32)NewLayer.IntensityLevel, VolumeMultiplier);
}

void UAudio_AdaptiveManager::PlayAudioEvent(const FString& EventName, FVector Location)
{
    FAudio_EventConfig* EventConfig = RegisteredAudioEvents.Find(EventName);
    if (!EventConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveManager: Audio event '%s' not found"), *EventName);
        return;
    }

    if (!SFXAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_AdaptiveManager: SFX audio component is null"));
        return;
    }

    // TODO: Load and play the sound cue
    // For now, just log the event
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Playing audio event '%s' at location %s"), 
           *EventName, *Location.ToString());

    SFXAudioComponent->SetVolumeMultiplier(EventConfig->Volume);
    SFXAudioComponent->SetPitchMultiplier(EventConfig->Pitch);
}

void UAudio_AdaptiveManager::StopAudioEvent(const FString& EventName)
{
    UAudioComponent** ActiveComponent = ActiveAudioEvents.Find(EventName);
    if (ActiveComponent && *ActiveComponent)
    {
        (*ActiveComponent)->Stop();
        ActiveAudioEvents.Remove(EventName);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Stopped audio event '%s'"), *EventName);
    }
}

void UAudio_AdaptiveManager::RegisterAudioEvent(const FAudio_EventConfig& EventConfig)
{
    RegisteredAudioEvents.Add(EventConfig.EventName, EventConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Registered audio event '%s'"), *EventConfig.EventName);
}

void UAudio_AdaptiveManager::UpdateWeatherAudio(float WindIntensity, float RainIntensity, bool bThunderActive)
{
    // Adjust ambience volume based on weather intensity
    if (AmbienceAudioComponent)
    {
        float WeatherVolume = FMath::Clamp(0.5f + (WindIntensity * 0.3f) + (RainIntensity * 0.4f), 0.3f, 1.0f);
        AmbienceAudioComponent->SetVolumeMultiplier(WeatherVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Weather audio updated - Wind: %.2f, Rain: %.2f, Thunder: %s"), 
           WindIntensity, RainIntensity, bThunderActive ? TEXT("Yes") : TEXT("No"));
}

void UAudio_AdaptiveManager::PlayFootstepSound(FVector Location, bool bIsPlayerFootstep)
{
    // TODO: Play appropriate footstep sound based on surface type and character
    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_AdaptiveManager: Footstep at %s (Player: %s)"), 
           *Location.ToString(), bIsPlayerFootstep ? TEXT("Yes") : TEXT("No"));
}

void UAudio_AdaptiveManager::PlayDinosaurVocalization(FVector Location, const FString& DinosaurType, float IntensityLevel)
{
    // TODO: Play dinosaur-specific vocalization
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Dinosaur vocalization - Type: %s, Intensity: %.2f, Location: %s"), 
           *DinosaurType, IntensityLevel, *Location.ToString());

    // Increase fear level when large dinosaurs vocalize nearby
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Brachi")))
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
        if (Distance < 1500.0f)
        {
            float FearIncrease = FMath::Clamp((1500.0f - Distance) / 1500.0f * 0.3f, 0.0f, 0.3f);
            UpdateMusicBasedOnFear(CurrentFearLevel + FearIncrease);
        }
    }
}

void UAudio_AdaptiveManager::PlayNarrativeVoiceLine(const FString& VoiceLineID, float DelayTime)
{
    if (!VoiceAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_AdaptiveManager: Voice audio component is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Playing narrative voice line '%s' with delay %.1fs"), 
           *VoiceLineID, DelayTime);

    // TODO: Load and play the voice line with delay
    if (DelayTime > 0.0f)
    {
        // Use timer for delayed playback
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, VoiceLineID]()
        {
            // Play voice line here
            UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Delayed voice line '%s' playing now"), *VoiceLineID);
        }, DelayTime, false);
    }
}

void UAudio_AdaptiveManager::SetNarrativeAudioVolume(float VolumeMultiplier)
{
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->SetVolumeMultiplier(FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f));
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveManager: Narrative audio volume set to %.2f"), VolumeMultiplier);
    }
}

void UAudio_AdaptiveManager::CleanupFinishedAudioEvents()
{
    TArray<FString> EventsToRemove;
    
    for (auto& EventPair : ActiveAudioEvents)
    {
        if (!EventPair.Value || !EventPair.Value->IsPlaying())
        {
            EventsToRemove.Add(EventPair.Key);
        }
    }
    
    for (const FString& EventName : EventsToRemove)
    {
        ActiveAudioEvents.Remove(EventName);
    }
}