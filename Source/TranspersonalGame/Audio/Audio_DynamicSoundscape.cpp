#include "Audio_DynamicSoundscape.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AAudio_DynamicSoundscape::AAudio_DynamicSoundscape()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize state
    CurrentZone = EAudio_SoundscapeZone::OpenValley;
    TargetZone = EAudio_SoundscapeZone::OpenValley;
    CurrentDangerLevel = 0.0f;
    TimeOfDayRatio = 0.25f; // Start at noon
    bInTransition = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 5.0f;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize audio components
    InitializeAudioComponents();

    // Load default zone configurations
    LoadDefaultZoneConfigurations();

    // Set up generated narration audio paths
    NarrationAudioPaths.Add(TEXT("AncientNarrator"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410658814_AncientNarrator.mp3"));
    NarrationAudioPaths.Add(TEXT("SurvivalGuide"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410663107_SurvivalGuide.mp3"));
    NarrationAudioPaths.Add(TEXT("ElderThok"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410658814_ElderThok.mp3"));
    NarrationAudioPaths.Add(TEXT("TribalScout"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410663107_TribalScout.mp3"));
}

void AAudio_DynamicSoundscape::BeginPlay()
{
    Super::BeginPlay();

    // Start with the default zone
    SetCurrentZone(CurrentZone, true);
}

void AAudio_DynamicSoundscape::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update zone transitions
    if (bInTransition)
    {
        UpdateZoneTransition(DeltaTime);
    }

    // Apply dynamic modulations
    ApplyDangerLevelModulation();
    ApplyTimeOfDayModulation();
    UpdateAudioComponentVolumes();
}

void AAudio_DynamicSoundscape::InitializeAudioComponents()
{
    // Create 6 ambient audio components for layered soundscape
    for (int32 i = 0; i < 6; i++)
    {
        FString ComponentName = FString::Printf(TEXT("AmbientAudio_%d"), i);
        UAudioComponent* AmbientComp = CreateDefaultSubobject<UAudioComponent>(*ComponentName);
        if (AmbientComp)
        {
            AmbientComp->SetupAttachment(RootComponent);
            AmbientComp->bAutoActivate = false;
            AmbientComp->SetVolumeMultiplier(0.0f);
            AmbientAudioComponents.Add(AmbientComp);
        }
    }

    // Create 4 music audio components for musical layers
    for (int32 i = 0; i < 4; i++)
    {
        FString ComponentName = FString::Printf(TEXT("MusicAudio_%d"), i);
        UAudioComponent* MusicComp = CreateDefaultSubobject<UAudioComponent>(*ComponentName);
        if (MusicComp)
        {
            MusicComp->SetupAttachment(RootComponent);
            MusicComp->bAutoActivate = false;
            MusicComp->SetVolumeMultiplier(0.0f);
            MusicAudioComponents.Add(MusicComp);
        }
    }

    // Create narration audio component
    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->SetupAttachment(RootComponent);
        NarrationAudioComponent->bAutoActivate = false;
        NarrationAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void AAudio_DynamicSoundscape::LoadDefaultZoneConfigurations()
{
    // Open Valley Configuration
    FAudio_ZoneConfiguration OpenValleyConfig;
    OpenValleyConfig.ZoneType = EAudio_SoundscapeZone::OpenValley;
    OpenValleyConfig.DangerLevel = 0.2f;
    OpenValleyConfig.TransitionRadius = 1500.0f;

    // Dense Forest Configuration
    FAudio_ZoneConfiguration DenseForestConfig;
    DenseForestConfig.ZoneType = EAudio_SoundscapeZone::DenseForest;
    DenseForestConfig.DangerLevel = 0.4f;
    DenseForestConfig.TransitionRadius = 800.0f;

    // River Side Configuration
    FAudio_ZoneConfiguration RiverSideConfig;
    RiverSideConfig.ZoneType = EAudio_SoundscapeZone::RiverSide;
    RiverSideConfig.DangerLevel = 0.1f;
    RiverSideConfig.TransitionRadius = 600.0f;

    // Cave Entrance Configuration
    FAudio_ZoneConfiguration CaveEntranceConfig;
    CaveEntranceConfig.ZoneType = EAudio_SoundscapeZone::CaveEntrance;
    CaveEntranceConfig.DangerLevel = 0.6f;
    CaveEntranceConfig.TransitionRadius = 400.0f;

    // Predator Territory Configuration
    FAudio_ZoneConfiguration PredatorTerritoryConfig;
    PredatorTerritoryConfig.ZoneType = EAudio_SoundscapeZone::PredatorTerritory;
    PredatorTerritoryConfig.DangerLevel = 0.9f;
    PredatorTerritoryConfig.TransitionRadius = 2000.0f;

    // Safe Zone Configuration
    FAudio_ZoneConfiguration SafeZoneConfig;
    SafeZoneConfig.ZoneType = EAudio_SoundscapeZone::SafeZone;
    SafeZoneConfig.DangerLevel = 0.0f;
    SafeZoneConfig.TransitionRadius = 500.0f;

    // Register all configurations
    ZoneConfigurations.Add(EAudio_SoundscapeZone::OpenValley, OpenValleyConfig);
    ZoneConfigurations.Add(EAudio_SoundscapeZone::DenseForest, DenseForestConfig);
    ZoneConfigurations.Add(EAudio_SoundscapeZone::RiverSide, RiverSideConfig);
    ZoneConfigurations.Add(EAudio_SoundscapeZone::CaveEntrance, CaveEntranceConfig);
    ZoneConfigurations.Add(EAudio_SoundscapeZone::PredatorTerritory, PredatorTerritoryConfig);
    ZoneConfigurations.Add(EAudio_SoundscapeZone::SafeZone, SafeZoneConfig);
}

void AAudio_DynamicSoundscape::SetCurrentZone(EAudio_SoundscapeZone NewZone, bool bForceImmediate)
{
    if (NewZone == CurrentZone && !bForceImmediate)
    {
        return;
    }

    if (bForceImmediate)
    {
        CurrentZone = NewZone;
        TargetZone = NewZone;
        bInTransition = false;
        TransitionProgress = 1.0f;
        
        // Apply zone configuration immediately
        if (ZoneConfigurations.Contains(NewZone))
        {
            CurrentDangerLevel = ZoneConfigurations[NewZone].DangerLevel;
        }
    }
    else
    {
        CrossfadeToZone(NewZone, 5.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Zone changed to %d"), (int32)NewZone);
}

void AAudio_DynamicSoundscape::UpdateDangerLevel(float NewDangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(NewDangerLevel, 0.0f, 1.0f);
}

void AAudio_DynamicSoundscape::TriggerStoryMoment(const FString& StoryEventName, float IntensityLevel)
{
    UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Story moment triggered - %s (Intensity: %f)"), *StoryEventName, IntensityLevel);

    // Modulate current soundscape based on story intensity
    float StoryDangerMod = IntensityLevel * 0.3f;
    UpdateDangerLevel(FMath::Min(CurrentDangerLevel + StoryDangerMod, 1.0f));

    // Trigger specific audio cues based on story event
    if (StoryEventName.Contains(TEXT("Predator")))
    {
        SetCurrentZone(EAudio_SoundscapeZone::PredatorTerritory);
    }
    else if (StoryEventName.Contains(TEXT("Safe")))
    {
        SetCurrentZone(EAudio_SoundscapeZone::SafeZone);
    }
    else if (StoryEventName.Contains(TEXT("Cave")))
    {
        SetCurrentZone(EAudio_SoundscapeZone::CaveEntrance);
    }
}

void AAudio_DynamicSoundscape::SetTimeOfDay(float TimeRatio)
{
    TimeOfDayRatio = FMath::Fmod(TimeRatio, 1.0f);
    if (TimeOfDayRatio < 0.0f)
    {
        TimeOfDayRatio += 1.0f;
    }
}

EAudio_SoundscapeZone AAudio_DynamicSoundscape::DetectPlayerZone(const FVector& PlayerLocation)
{
    // Simple zone detection based on location
    // This would be enhanced with proper zone volumes in a full implementation
    
    float DistanceFromOrigin = PlayerLocation.Size();
    
    if (DistanceFromOrigin < 500.0f)
    {
        return EAudio_SoundscapeZone::SafeZone;
    }
    else if (DistanceFromOrigin > 3000.0f)
    {
        return EAudio_SoundscapeZone::PredatorTerritory;
    }
    else if (PlayerLocation.Z < -200.0f)
    {
        return EAudio_SoundscapeZone::CaveEntrance;
    }
    else if (FMath::Abs(PlayerLocation.Y) > 2000.0f)
    {
        return EAudio_SoundscapeZone::RiverSide;
    }
    else if (DistanceFromOrigin > 1500.0f)
    {
        return EAudio_SoundscapeZone::DenseForest;
    }
    
    return EAudio_SoundscapeZone::OpenValley;
}

void AAudio_DynamicSoundscape::RegisterZoneConfiguration(const FAudio_ZoneConfiguration& ZoneConfig)
{
    ZoneConfigurations.Add(ZoneConfig.ZoneType, ZoneConfig);
}

void AAudio_DynamicSoundscape::FadeInLayer(int32 LayerIndex, float FadeTime)
{
    if (AmbientAudioComponents.IsValidIndex(LayerIndex))
    {
        UAudioComponent* AudioComp = AmbientAudioComponents[LayerIndex];
        if (AudioComp && AudioComp->Sound)
        {
            AudioComp->FadeIn(FadeTime, 1.0f);
        }
    }
}

void AAudio_DynamicSoundscape::FadeOutLayer(int32 LayerIndex, float FadeTime)
{
    if (AmbientAudioComponents.IsValidIndex(LayerIndex))
    {
        UAudioComponent* AudioComp = AmbientAudioComponents[LayerIndex];
        if (AudioComp)
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
        }
    }
}

void AAudio_DynamicSoundscape::CrossfadeToZone(EAudio_SoundscapeZone TargetZone, float TransitionTime)
{
    if (TargetZone == CurrentZone)
    {
        return;
    }

    StartZoneTransition(CurrentZone, TargetZone, TransitionTime);
}

void AAudio_DynamicSoundscape::PlayNarrationClip(const FString& CharacterName, const FString& AudioFilePath)
{
    if (NarrationAudioComponent)
    {
        // Stop current narration
        StopNarration(false);

        UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Playing narration for %s"), *CharacterName);

        // In a full implementation, this would load and play the audio file
        // For now, we'll simulate the narration playback
        NarrationAudioComponent->SetVolumeMultiplier(1.0f);
        
        // Check if we have a path for this character
        if (NarrationAudioPaths.Contains(CharacterName))
        {
            FString* AudioPath = NarrationAudioPaths.Find(CharacterName);
            UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Narration audio path - %s"), **AudioPath);
        }
    }
}

void AAudio_DynamicSoundscape::StopNarration(bool bFadeOut)
{
    if (NarrationAudioComponent)
    {
        if (bFadeOut)
        {
            NarrationAudioComponent->FadeOut(1.0f, 0.0f);
        }
        else
        {
            NarrationAudioComponent->Stop();
        }
    }
}

void AAudio_DynamicSoundscape::UpdateZoneTransition(float DeltaTime)
{
    if (!bInTransition)
    {
        return;
    }

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        CompleteZoneTransition();
    }
}

void AAudio_DynamicSoundscape::ApplyDangerLevelModulation()
{
    // Modulate audio based on danger level
    // Higher danger = more tension, lower frequencies, increased reverb
    float DangerMod = 1.0f + (CurrentDangerLevel * 0.5f);
    
    // Apply to all active audio components
    for (UAudioComponent* AudioComp : AmbientAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            float ModulatedPitch = FMath::Lerp(1.0f, 0.8f, CurrentDangerLevel);
            AudioComp->SetPitchMultiplier(ModulatedPitch);
        }
    }
}

void AAudio_DynamicSoundscape::ApplyTimeOfDayModulation()
{
    // Modulate audio based on time of day
    // Dawn/Dusk = more atmospheric, Night = quieter ambient, Day = fuller soundscape
    
    float TimeBasedVolume = 1.0f;
    
    if (TimeOfDayRatio < 0.25f || TimeOfDayRatio > 0.75f) // Night time
    {
        TimeBasedVolume = 0.6f;
    }
    else if (TimeOfDayRatio < 0.35f || TimeOfDayRatio > 0.65f) // Dawn/Dusk
    {
        TimeBasedVolume = 0.8f;
    }
    
    // Apply time-based modulation to music layers
    for (UAudioComponent* MusicComp : MusicAudioComponents)
    {
        if (MusicComp)
        {
            float CurrentVolume = MusicComp->GetVolumeMultiplier();
            MusicComp->SetVolumeMultiplier(CurrentVolume * TimeBasedVolume);
        }
    }
}

void AAudio_DynamicSoundscape::UpdateAudioComponentVolumes()
{
    // Update volumes based on current zone configuration and modulations
    if (!ZoneConfigurations.Contains(CurrentZone))
    {
        return;
    }

    const FAudio_ZoneConfiguration& CurrentConfig = ZoneConfigurations[CurrentZone];
    
    // Apply zone-specific volume settings
    for (int32 i = 0; i < AmbientAudioComponents.Num() && i < CurrentConfig.AmbientLayers.Num(); i++)
    {
        UAudioComponent* AudioComp = AmbientAudioComponents[i];
        const FAudio_SoundscapeLayer& Layer = CurrentConfig.AmbientLayers[i];
        
        if (AudioComp)
        {
            float FinalVolume = CalculateLayerVolume(Layer, 1.0f, CurrentDangerLevel, TimeOfDayRatio);
            AudioComp->SetVolumeMultiplier(FinalVolume);
        }
    }
}

float AAudio_DynamicSoundscape::CalculateLayerVolume(const FAudio_SoundscapeLayer& Layer, float ZoneBlend, float DangerMod, float TimeMod)
{
    float BaseVolume = Layer.BaseVolume;
    float DangerModulation = 1.0f + (DangerMod * 0.3f);
    float TimeModulation = FMath::Lerp(0.7f, 1.0f, FMath::Abs(TimeMod - 0.5f) * 2.0f);
    
    return BaseVolume * ZoneBlend * DangerModulation * TimeModulation;
}

void AAudio_DynamicSoundscape::StartZoneTransition(EAudio_SoundscapeZone FromZone, EAudio_SoundscapeZone ToZone, float Duration)
{
    TargetZone = ToZone;
    bInTransition = true;
    TransitionProgress = 0.0f;
    TransitionDuration = Duration;
    
    UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Starting transition from %d to %d"), (int32)FromZone, (int32)ToZone);
}

void AAudio_DynamicSoundscape::CompleteZoneTransition()
{
    CurrentZone = TargetZone;
    bInTransition = false;
    TransitionProgress = 1.0f;
    
    // Update danger level to match new zone
    if (ZoneConfigurations.Contains(CurrentZone))
    {
        CurrentDangerLevel = ZoneConfigurations[CurrentZone].DangerLevel;
    }
    
    UE_LOG(LogTemp, Log, TEXT("DynamicSoundscape: Transition completed to zone %d"), (int32)CurrentZone);
}