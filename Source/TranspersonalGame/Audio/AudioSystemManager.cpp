#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"

UAudioSystemManager::UAudioSystemManager()
{
    // Initialize default configuration
    MusicConfig.AmbientVolume = 0.7f;
    MusicConfig.MelodicVolume = 0.5f;
    MusicConfig.RhythmicVolume = 0.3f;
    MusicConfig.SpiritualVolume = 0.4f;
    MusicConfig.CurrentEmotionalState = EAudio_EmotionalState::Peaceful;
    MusicConfig.TransitionDuration = 3.0f;

    DefaultSpatialConfig.MaxAudibleDistance = 5000.0f;
    DefaultSpatialConfig.AttenuationRadius = 1000.0f;
    DefaultSpatialConfig.bUse3DAudio = true;
    DefaultSpatialConfig.DopplerFactor = 1.0f;

    PreviousEmotionalState = EAudio_EmotionalState::Peaceful;
    bIsTransitioning = false;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing..."));
    
    // Initialize audio components
    InitializeMusicLayers();
    InitializeAmbienceLayers();
    
    // Create narration component
    if (GetWorld())
    {
        AActor* DummyActor = GetWorld()->SpawnActor<AActor>();
        if (DummyActor)
        {
            NarrationComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationComponent"));
            SFXComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("SFXComponent"));
            
            if (NarrationComponent)
            {
                NarrationComponent->bAutoActivate = false;
                NarrationComponent->SetVolumeMultiplier(0.8f);
            }
            
            if (SFXComponent)
            {
                SFXComponent->bAutoActivate = false;
                SFXComponent->SetVolumeMultiplier(1.0f);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initialization complete"));
}

void UAudioSystemManager::Deinitialize()
{
    // Clean up audio components
    for (UAudioComponent* Component : MusicLayers)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    for (UAudioComponent* Component : AmbienceLayers)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    if (NarrationComponent && IsValid(NarrationComponent))
    {
        NarrationComponent->Stop();
    }
    
    if (SFXComponent && IsValid(SFXComponent))
    {
        SFXComponent->Stop();
    }
    
    MusicLayers.Empty();
    AmbienceLayers.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeMusicLayers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create 4 music layers for adaptive composition
    for (int32 i = 0; i < 4; i++)
    {
        AActor* DummyActor = GetWorld()->SpawnActor<AActor>();
        if (DummyActor)
        {
            UAudioComponent* LayerComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(
                *FString::Printf(TEXT("MusicLayer_%d"), i)
            );
            
            if (LayerComponent)
            {
                LayerComponent->bAutoActivate = false;
                LayerComponent->SetVolumeMultiplier(0.0f); // Start silent
                MusicLayers.Add(LayerComponent);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Created %d music layers"), MusicLayers.Num());
}

void UAudioSystemManager::InitializeAmbienceLayers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create ambience layers for environmental audio
    for (int32 i = 0; i < 3; i++)
    {
        AActor* DummyActor = GetWorld()->SpawnActor<AActor>();
        if (DummyActor)
        {
            UAudioComponent* AmbienceComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(
                *FString::Printf(TEXT("AmbienceLayer_%d"), i)
            );
            
            if (AmbienceComponent)
            {
                AmbienceComponent->bAutoActivate = false;
                AmbienceComponent->SetVolumeMultiplier(0.0f);
                AmbienceLayers.Add(AmbienceComponent);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Created %d ambience layers"), AmbienceLayers.Num());
}

void UAudioSystemManager::SetEmotionalState(EAudio_EmotionalState NewState, float TransitionTime)
{
    if (NewState == MusicConfig.CurrentEmotionalState)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning to emotional state: %d"), (int32)NewState);
    
    PreviousEmotionalState = MusicConfig.CurrentEmotionalState;
    MusicConfig.CurrentEmotionalState = NewState;
    MusicConfig.TransitionDuration = TransitionTime;
    
    CrossfadeToNewState(NewState, TransitionTime);
}

void UAudioSystemManager::CrossfadeToNewState(EAudio_EmotionalState NewState, float Duration)
{
    bIsTransitioning = true;
    
    // Start transition timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MusicTransitionTimer,
            [this]()
            {
                bIsTransitioning = false;
                UpdateMusicBasedOnEmotionalState();
            },
            Duration,
            false
        );
    }
    
    UpdateMusicBasedOnEmotionalState();
}

void UAudioSystemManager::UpdateMusicBasedOnEmotionalState()
{
    // Adjust layer volumes based on emotional state
    float AmbientTarget = 0.0f;
    float MelodicTarget = 0.0f;
    float RhythmicTarget = 0.0f;
    float SpiritualTarget = 0.0f;
    
    switch (MusicConfig.CurrentEmotionalState)
    {
        case EAudio_EmotionalState::Peaceful:
            AmbientTarget = 0.8f;
            MelodicTarget = 0.3f;
            RhythmicTarget = 0.1f;
            SpiritualTarget = 0.2f;
            break;
            
        case EAudio_EmotionalState::Contemplative:
            AmbientTarget = 0.6f;
            MelodicTarget = 0.7f;
            RhythmicTarget = 0.2f;
            SpiritualTarget = 0.5f;
            break;
            
        case EAudio_EmotionalState::Mystical:
            AmbientTarget = 0.4f;
            MelodicTarget = 0.5f;
            RhythmicTarget = 0.3f;
            SpiritualTarget = 0.9f;
            break;
            
        case EAudio_EmotionalState::Transformative:
            AmbientTarget = 0.3f;
            MelodicTarget = 0.8f;
            RhythmicTarget = 0.6f;
            SpiritualTarget = 0.7f;
            break;
            
        case EAudio_EmotionalState::Awakening:
            AmbientTarget = 0.5f;
            MelodicTarget = 0.9f;
            RhythmicTarget = 0.4f;
            SpiritualTarget = 1.0f;
            break;
    }
    
    // Apply volumes to layers
    SetMusicLayerVolume(EAudio_MusicLayer::Ambient, AmbientTarget, MusicConfig.TransitionDuration);
    SetMusicLayerVolume(EAudio_MusicLayer::Melodic, MelodicTarget, MusicConfig.TransitionDuration);
    SetMusicLayerVolume(EAudio_MusicLayer::Rhythmic, RhythmicTarget, MusicConfig.TransitionDuration);
    SetMusicLayerVolume(EAudio_MusicLayer::Spiritual, SpiritualTarget, MusicConfig.TransitionDuration);
}

void UAudioSystemManager::SetMusicLayerVolume(EAudio_MusicLayer Layer, float Volume, float FadeTime)
{
    int32 LayerIndex = (int32)Layer;
    
    if (MusicLayers.IsValidIndex(LayerIndex) && MusicLayers[LayerIndex])
    {
        MusicLayers[LayerIndex]->SetVolumeMultiplier(Volume);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set layer %d volume to %f"), LayerIndex, Volume);
    }
}

void UAudioSystemManager::PlayConsciousnessShiftSound(FVector Location)
{
    if (SFXComponent)
    {
        // Play consciousness transformation sound effect
        SFXComponent->SetWorldLocation(Location);
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing consciousness shift sound at location: %s"), 
               *Location.ToString());
    }
}

void UAudioSystemManager::RegisterSpatialAudioSource(AActor* SourceActor, USoundCue* SoundCue, const FAudio_SpatialConfiguration& Config)
{
    if (!SourceActor || !SoundCue)
    {
        return;
    }
    
    UAudioComponent* AudioComp = SourceActor->FindComponentByClass<UAudioComponent>();
    if (!AudioComp)
    {
        AudioComp = SourceActor->CreateDefaultSubobject<UAudioComponent>(TEXT("SpatialAudioComponent"));
    }
    
    if (AudioComp)
    {
        AudioComp->SetSound(SoundCue);
        AudioComp->bAutoActivate = true;
        
        // Configure spatial audio properties
        if (Config.bUse3DAudio)
        {
            AudioComp->bOverrideAttenuation = true;
            // Additional spatial audio configuration would go here
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Registered spatial audio source for %s"), 
               *SourceActor->GetName());
    }
}

void UAudioSystemManager::UpdateListenerPosition(FVector NewPosition, FRotator NewRotation)
{
    // Update audio listener position for spatial audio calculations
    if (GetWorld())
    {
        // This would typically be handled by the player's audio listener component
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Updated listener position to %s"), 
               *NewPosition.ToString());
    }
}

void UAudioSystemManager::SetBiomeAmbience(const FString& BiomeName, float FadeTime)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Setting biome ambience to %s"), *BiomeName);
    
    // Fade out current ambience layers
    for (UAudioComponent* AmbienceLayer : AmbienceLayers)
    {
        if (AmbienceLayer)
        {
            AmbienceLayer->SetVolumeMultiplier(0.0f);
        }
    }
    
    // Load and play new biome-specific ambience
    // This would load appropriate sound assets based on biome name
}

void UAudioSystemManager::AddAmbienceLayer(const FString& LayerName, USoundCue* SoundCue, float Volume)
{
    if (!SoundCue)
    {
        return;
    }
    
    // Find available ambience layer
    for (UAudioComponent* AmbienceLayer : AmbienceLayers)
    {
        if (AmbienceLayer && !AmbienceLayer->IsPlaying())
        {
            AmbienceLayer->SetSound(SoundCue);
            AmbienceLayer->SetVolumeMultiplier(Volume);
            AmbienceLayer->Play();
            
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Added ambience layer %s"), *LayerName);
            break;
        }
    }
}

void UAudioSystemManager::PlayNarration(USoundWave* NarrationSound, bool bInterruptCurrent)
{
    if (!NarrationComponent || !NarrationSound)
    {
        return;
    }
    
    if (bInterruptCurrent || !NarrationComponent->IsPlaying())
    {
        NarrationComponent->SetSound(NarrationSound);
        NarrationComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing narration"));
    }
}

void UAudioSystemManager::SetNarrationVolume(float Volume)
{
    if (NarrationComponent)
    {
        NarrationComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
    }
}

void UAudioSystemManager::TestAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Running audio system test"));
    
    // Test emotional state transitions
    SetEmotionalState(EAudio_EmotionalState::Mystical, 2.0f);
    
    // Test consciousness shift sound
    PlayConsciousnessShiftSound(FVector(0, 0, 100));
    
    LogCurrentAudioState();
}

void UAudioSystemManager::LogCurrentAudioState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AUDIO SYSTEM STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Emotional State: %d"), (int32)MusicConfig.CurrentEmotionalState);
    UE_LOG(LogTemp, Warning, TEXT("Music Layers: %d"), MusicLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Ambience Layers: %d"), AmbienceLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Is Transitioning: %s"), bIsTransitioning ? TEXT("Yes") : TEXT("No"));
    
    for (int32 i = 0; i < MusicLayers.Num(); i++)
    {
        if (MusicLayers[i])
        {
            float Volume = MusicLayers[i]->GetVolumeMultiplier();
            UE_LOG(LogTemp, Warning, TEXT("Music Layer %d Volume: %f"), i, Volume);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END AUDIO STATE ==="));
}