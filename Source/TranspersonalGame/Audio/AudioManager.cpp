#include "AudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioMixerBlueprintLibrary.h"
#include "MetasoundParameterPack.h"

UAudioManager::UAudioManager()
{
    // Initialize default audio state
    CurrentAudioState.EmotionalState = EEmotionalState::Calm;
    CurrentAudioState.Zone = EEnvironmentalZone::Forest;
    CurrentAudioState.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioState.TensionLevel = 0.0f;
    CurrentAudioState.DinosaurProximity = 0.0f;
    CurrentAudioState.bInCombat = false;
    CurrentAudioState.bNearWater = false;
    CurrentAudioState.WeatherIntensity = 0.0f;

    TargetAudioState = CurrentAudioState;
}

void UAudioManager::InitializeAudioManager(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioManager: Cannot initialize without valid World context"));
        return;
    }

    WorldContext = World;

    // Create audio components
    if (AActor* DummyActor = World->SpawnActor<AActor>())
    {
        MusicComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
        AmbientComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
        StingerComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("StingerComponent"));
        VoiceComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceComponent"));

        // Configure audio components
        if (MusicComponent)
        {
            MusicComponent->SetVolumeMultiplier(MusicVolume);
            MusicComponent->bAutoActivate = false;
        }

        if (AmbientComponent)
        {
            AmbientComponent->SetVolumeMultiplier(EnvironmentalVolume);
            AmbientComponent->bAutoActivate = false;
        }

        if (VoiceComponent)
        {
            VoiceComponent->SetVolumeMultiplier(VoiceVolume);
            VoiceComponent->bAutoActivate = false;
        }
    }

    // Start audio update timer
    World->GetTimerManager().SetTimer(AudioUpdateTimer, [this]()
    {
        ProcessAudioTransitions(0.1f); // 10Hz update rate
    }, 0.1f, true);

    // Initialize with default ambient sounds
    UpdateAmbientSounds(CurrentAudioState.Zone, CurrentAudioState.TimeOfDay);

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Initialized successfully"));
}

void UAudioManager::UpdateAudioState(const FAudioState& NewState)
{
    TargetAudioState = NewState;
    
    // Check if we need immediate transitions for critical states
    if (NewState.bInCombat != CurrentAudioState.bInCombat)
    {
        if (NewState.bInCombat)
        {
            SetEmotionalState(EEmotionalState::Danger, 0.5f);
        }
        else
        {
            SetEmotionalState(EEmotionalState::Calm, 2.0f);
        }
    }

    // Update dinosaur proximity audio
    if (FMath::Abs(NewState.DinosaurProximity - CurrentAudioState.DinosaurProximity) > 0.1f)
    {
        UpdateDinosaurProximityAudio(NewState.DinosaurProximity);
    }

    // Start transition if not already transitioning
    if (!bIsTransitioning)
    {
        bIsTransitioning = true;
        TransitionProgress = 0.0f;
    }
}

void UAudioManager::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState == CurrentAudioState.EmotionalState)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Transitioning emotional state to %d over %.2f seconds"), 
           (int32)NewState, TransitionTime);

    TargetAudioState.EmotionalState = NewState;
    PlayAdaptiveMusic(NewState, CurrentAudioState.Zone);
}

void UAudioManager::SetEnvironmentalZone(EEnvironmentalZone NewZone, float TransitionTime)
{
    if (NewZone == CurrentAudioState.Zone)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Transitioning to environmental zone %d"), (int32)NewZone);

    TargetAudioState.Zone = NewZone;
    UpdateAmbientSounds(NewZone, CurrentAudioState.TimeOfDay);
}

void UAudioManager::PlayAdaptiveMusic(EEmotionalState State, EEnvironmentalZone Zone)
{
    if (!MusicComponent || !WorldContext)
    {
        return;
    }

    // Find appropriate music track for the emotional state
    UMetaSoundSource** FoundTrack = EmotionalMusicTracks.Find(State);
    if (FoundTrack && *FoundTrack)
    {
        // Stop current music with fade
        if (MusicComponent->IsPlaying())
        {
            MusicComponent->FadeOut(2.0f, 0.0f);
        }

        // Start new track with fade in
        MusicComponent->SetSound(*FoundTrack);
        MusicComponent->FadeIn(2.0f, MusicVolume);
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing adaptive music for emotional state %d"), (int32)State);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No music track found for emotional state %d"), (int32)State);
    }
}

void UAudioManager::TransitionMusicLayers(float IntensityLevel, float TransitionTime)
{
    if (!MusicComponent)
    {
        return;
    }

    // Clamp intensity level
    IntensityLevel = FMath::Clamp(IntensityLevel, 0.0f, 1.0f);

    // Adjust music volume based on intensity
    float TargetVolume = MusicVolume * (0.3f + 0.7f * IntensityLevel);
    
    // Use MetaSound parameters if available
    if (UMetaSoundSource* MetaSound = Cast<UMetaSoundSource>(MusicComponent->GetSound()))
    {
        // Create parameter pack for MetaSound
        UMetasoundParameterPack* ParamPack = NewObject<UMetasoundParameterPack>();
        if (ParamPack)
        {
            ParamPack->SetFloat(TEXT("Intensity"), IntensityLevel);
            ParamPack->SetFloat(TEXT("Volume"), TargetVolume);
            MusicComponent->SetParameterPack(ParamPack);
        }
    }
    else
    {
        // Fallback to volume adjustment
        MusicComponent->SetVolumeMultiplier(TargetVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Music intensity set to %.2f"), IntensityLevel);
}

void UAudioManager::StopAllMusic(float FadeTime)
{
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->FadeOut(FadeTime, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Stopping all music with %.2f second fade"), FadeTime);
    }
}

void UAudioManager::UpdateAmbientSounds(EEnvironmentalZone Zone, ETimeOfDay TimeOfDay)
{
    if (!AmbientComponent || !WorldContext)
    {
        return;
    }

    // Find appropriate ambient sound for the zone
    USoundCue** FoundAmbient = EnvironmentalAmbients.Find(Zone);
    if (FoundAmbient && *FoundAmbient)
    {
        // Stop current ambient with fade
        if (AmbientComponent->IsPlaying())
        {
            AmbientComponent->FadeOut(3.0f, 0.0f);
        }

        // Start new ambient with fade in
        AmbientComponent->SetSound(*FoundAmbient);
        
        // Adjust volume based on time of day
        float TimeMultiplier = 1.0f;
        switch (TimeOfDay)
        {
            case ETimeOfDay::Night:
            case ETimeOfDay::DeepNight:
                TimeMultiplier = 0.6f; // Quieter at night
                break;
            case ETimeOfDay::Dawn:
            case ETimeOfDay::Dusk:
                TimeMultiplier = 0.8f; // Moderate during transitions
                break;
            default:
                TimeMultiplier = 1.0f; // Full volume during day
                break;
        }

        float TargetVolume = EnvironmentalVolume * TimeMultiplier;
        AmbientComponent->FadeIn(3.0f, TargetVolume);
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing ambient for zone %d at volume %.2f"), 
               (int32)Zone, TargetVolume);
    }
}

void UAudioManager::PlayEnvironmentalStinger(const FString& EventName, FVector Location)
{
    if (!StingerComponent || !WorldContext)
    {
        return;
    }

    // This would typically load a sound based on the event name
    // For now, we'll log the event
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing environmental stinger '%s' at location %s"), 
           *EventName, *Location.ToString());

    // Set 3D location for the stinger
    if (AActor* StingerActor = StingerComponent->GetOwner())
    {
        StingerActor->SetActorLocation(Location);
    }
}

void UAudioManager::PlayDinosaurCall(const FString& DinosaurType, FVector Location, float Intensity)
{
    if (!WorldContext)
    {
        return;
    }

    USoundCue** FoundSound = DinosaurSounds.Find(DinosaurType);
    if (FoundSound && *FoundSound)
    {
        // Play dinosaur sound at specific location with intensity-based volume
        float VolumeMultiplier = FMath::Clamp(Intensity, 0.1f, 2.0f);
        
        UGameplayStatics::PlaySoundAtLocation(
            WorldContext,
            *FoundSound,
            Location,
            VolumeMultiplier,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation override
            nullptr, // Concurrency override
            nullptr  // Owner
        );

        UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing dinosaur call '%s' at intensity %.2f"), 
               *DinosaurType, Intensity);
    }
}

void UAudioManager::UpdateDinosaurProximityAudio(float ProximityLevel)
{
    ProximityLevel = FMath::Clamp(ProximityLevel, 0.0f, 1.0f);

    // Increase tension level based on dinosaur proximity
    float TensionIncrease = ProximityLevel * 0.5f;
    TargetAudioState.TensionLevel = FMath::Clamp(
        CurrentAudioState.TensionLevel + TensionIncrease, 
        0.0f, 
        1.0f
    );

    // Adjust music intensity
    TransitionMusicLayers(TargetAudioState.TensionLevel, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Dinosaur proximity %.2f, tension level %.2f"), 
           ProximityLevel, TargetAudioState.TensionLevel);
}

void UAudioManager::PlayUISound(const FString& SoundName)
{
    if (!WorldContext)
    {
        return;
    }

    USoundWave** FoundSound = UISounds.Find(SoundName);
    if (FoundSound && *FoundSound)
    {
        UGameplayStatics::PlaySound2D(WorldContext, *FoundSound, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing UI sound '%s'"), *SoundName);
    }
}

void UAudioManager::PlayInteractionSound(const FString& InteractionType, FVector Location)
{
    if (!WorldContext)
    {
        return;
    }

    // This would typically map interaction types to specific sounds
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing interaction sound '%s' at %s"), 
           *InteractionType, *Location.ToString());
}

void UAudioManager::PlayVoiceLine(const FString& CharacterName, const FString& LineID, FVector Location)
{
    if (!VoiceComponent || !WorldContext)
    {
        return;
    }

    // This would typically load the appropriate voice line based on character and line ID
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing voice line '%s' for character '%s'"), 
           *LineID, *CharacterName);

    // Set 3D location for voice
    if (AActor* VoiceActor = VoiceComponent->GetOwner())
    {
        VoiceActor->SetActorLocation(Location);
    }
}

void UAudioManager::PlayNarration(const FString& NarrationID)
{
    if (!VoiceComponent)
    {
        return;
    }

    // Narration is typically played as 2D audio
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing narration '%s'"), *NarrationID);
}

float UAudioManager::GetCurrentMusicIntensity() const
{
    return CurrentAudioState.TensionLevel;
}

TArray<float> UAudioManager::GetSpectrumData() const
{
    TArray<float> SpectrumData;
    
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        // This would typically use UAudioMixerBlueprintLibrary::GetMagnitudeForFrequencies
        // For now, return dummy data
        SpectrumData.Init(0.0f, 64); // 64 frequency bands
    }
    
    return SpectrumData;
}

void UAudioManager::ProcessAudioTransitions(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    TransitionProgress += DeltaTime;
    float Alpha = FMath::Clamp(TransitionProgress / 2.0f, 0.0f, 1.0f); // 2 second transition

    // Interpolate audio state values
    CurrentAudioState.TensionLevel = FMath::Lerp(
        CurrentAudioState.TensionLevel, 
        TargetAudioState.TensionLevel, 
        Alpha
    );

    CurrentAudioState.DinosaurProximity = FMath::Lerp(
        CurrentAudioState.DinosaurProximity,
        TargetAudioState.DinosaurProximity,
        Alpha
    );

    CurrentAudioState.WeatherIntensity = FMath::Lerp(
        CurrentAudioState.WeatherIntensity,
        TargetAudioState.WeatherIntensity,
        Alpha
    );

    // Update discrete states when transition is complete
    if (Alpha >= 1.0f)
    {
        CurrentAudioState = TargetAudioState;
        bIsTransitioning = false;
        TransitionProgress = 0.0f;
    }

    // Update audio layers based on current state
    UpdateMusicLayers();
    UpdateAmbientLayers();
}

void UAudioManager::UpdateMusicLayers()
{
    // Adjust music parameters based on current audio state
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        float IntensityLevel = CurrentAudioState.TensionLevel;
        
        // Add dinosaur proximity influence
        IntensityLevel += CurrentAudioState.DinosaurProximity * 0.3f;
        
        // Add combat influence
        if (CurrentAudioState.bInCombat)
        {
            IntensityLevel += 0.5f;
        }
        
        IntensityLevel = FMath::Clamp(IntensityLevel, 0.0f, 1.0f);
        
        // Apply intensity to music
        TransitionMusicLayers(IntensityLevel, 0.1f);
    }
}

void UAudioManager::UpdateAmbientLayers()
{
    // Adjust ambient parameters based on current audio state
    if (AmbientComponent && AmbientComponent->IsPlaying())
    {
        float AmbientVolume = EnvironmentalVolume;
        
        // Reduce ambient volume during high tension
        AmbientVolume *= (1.0f - CurrentAudioState.TensionLevel * 0.4f);
        
        // Weather influence
        AmbientVolume *= (1.0f + CurrentAudioState.WeatherIntensity * 0.3f);
        
        AmbientComponent->SetVolumeMultiplier(AmbientVolume);
    }
}