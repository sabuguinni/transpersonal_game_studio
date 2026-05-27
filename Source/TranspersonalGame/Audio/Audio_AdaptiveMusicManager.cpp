#include "Audio_AdaptiveMusicManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_AdaptiveMusicManager::UAudio_AdaptiveMusicManager()
{
    DefaultFadeTime = 2.0f;
    IntensityUpdateRate = 0.1f;
}

void UAudio_AdaptiveMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Initializing adaptive music system"));
    
    // Initialize default music state
    CurrentMusicState = FAudio_MusicState();
    
    // Initialize audio components
    InitializeAudioComponents();
    
    // Start intensity update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            IntensityUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UAudio_AdaptiveMusicManager::UpdateIntensity, IntensityUpdateRate),
            IntensityUpdateRate,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Initialization complete"));
}

void UAudio_AdaptiveMusicManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Deinitializing"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(IntensityUpdateTimer);
        World->GetTimerManager().ClearTimer(CrossfadeTimer);
    }
    
    // Cleanup audio components
    CleanupAudioComponents();
    
    Super::Deinitialize();
}

void UAudio_AdaptiveMusicManager::UpdateMusicState(const FAudio_MusicState& NewState)
{
    FAudio_MusicState OldState = CurrentMusicState;
    CurrentMusicState = NewState;
    
    // Handle story phase transitions
    if (OldState.CurrentStoryPhase != NewState.CurrentStoryPhase)
    {
        HandleStoryPhaseTransition(OldState.CurrentStoryPhase, NewState.CurrentStoryPhase);
    }
    
    // Handle biome transitions
    if (OldState.CurrentBiome != NewState.CurrentBiome)
    {
        HandleBiomeTransition(OldState.CurrentBiome, NewState.CurrentBiome);
    }
    
    // Update active layer based on new state
    UpdateLayerBasedOnState();
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_AdaptiveMusicManager: Music state updated - Phase: %d, Biome: %d, Fear: %f"), 
           (int32)NewState.CurrentStoryPhase, (int32)NewState.CurrentBiome, NewState.FearLevel);
}

void UAudio_AdaptiveMusicManager::SetStoryPhase(EStoryPhase NewPhase)
{
    if (CurrentMusicState.CurrentStoryPhase != NewPhase)
    {
        EStoryPhase OldPhase = CurrentMusicState.CurrentStoryPhase;
        CurrentMusicState.CurrentStoryPhase = NewPhase;
        HandleStoryPhaseTransition(OldPhase, NewPhase);
        UpdateLayerBasedOnState();
    }
}

void UAudio_AdaptiveMusicManager::SetBiome(EAudio_BiomeTheme NewBiome)
{
    if (CurrentMusicState.CurrentBiome != NewBiome)
    {
        EAudio_BiomeTheme OldBiome = CurrentMusicState.CurrentBiome;
        CurrentMusicState.CurrentBiome = NewBiome;
        HandleBiomeTransition(OldBiome, NewBiome);
        UpdateLayerBasedOnState();
    }
}

void UAudio_AdaptiveMusicManager::SetFearLevel(float NewFearLevel)
{
    CurrentMusicState.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    // Update intensity based on fear level
    CurrentMusicState.IntensityLevel = CurrentMusicState.FearLevel * 0.7f + 
                                      (CurrentMusicState.bInCombat ? 0.3f : 0.0f);
    
    UpdateVolumeBasedOnIntensity();
    
    // Trigger tension layer if fear is high
    if (CurrentMusicState.FearLevel > 0.6f && !CurrentMusicState.bInCombat)
    {
        if (CurrentMusicState.ActiveLayer != EAudio_MusicLayer::Tension)
        {
            CrossfadeToLayer(EAudio_MusicLayer::Tension);
        }
    }
    else if (CurrentMusicState.FearLevel < 0.3f && CurrentMusicState.ActiveLayer == EAudio_MusicLayer::Tension)
    {
        CrossfadeToLayer(EAudio_MusicLayer::Ambient);
    }
}

void UAudio_AdaptiveMusicManager::SetCombatState(bool bInCombat)
{
    CurrentMusicState.bInCombat = bInCombat;
    
    if (bInCombat)
    {
        CrossfadeToLayer(EAudio_MusicLayer::Combat, 1.0f);
        PlayStinger("CombatStart");
    }
    else
    {
        // Return to appropriate layer based on fear level
        if (CurrentMusicState.FearLevel > 0.5f)
        {
            CrossfadeToLayer(EAudio_MusicLayer::Tension, 3.0f);
        }
        else
        {
            CrossfadeToLayer(EAudio_MusicLayer::Ambient, 4.0f);
        }
        PlayStinger("CombatEnd");
    }
    
    UpdateLayerBasedOnState();
}

void UAudio_AdaptiveMusicManager::SetPredatorProximity(bool bNearPredator)
{
    CurrentMusicState.bNearPredator = bNearPredator;
    
    if (bNearPredator && !CurrentMusicState.bInCombat)
    {
        SetFearLevel(FMath::Max(CurrentMusicState.FearLevel, 0.7f));
        PlayStinger("PredatorNear");
    }
}

void UAudio_AdaptiveMusicManager::FadeInLayer(EAudio_MusicLayer Layer, float FadeTime)
{
    if (UAudioComponent** AudioCompPtr = MusicLayers.Find(Layer))
    {
        if (UAudioComponent* AudioComp = *AudioCompPtr)
        {
            AudioComp->FadeIn(FadeTime, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("UAudio_AdaptiveMusicManager: Fading in layer %d"), (int32)Layer);
        }
    }
}

void UAudio_AdaptiveMusicManager::FadeOutLayer(EAudio_MusicLayer Layer, float FadeTime)
{
    if (UAudioComponent** AudioCompPtr = MusicLayers.Find(Layer))
    {
        if (UAudioComponent* AudioComp = *AudioCompPtr)
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
            UE_LOG(LogTemp, Log, TEXT("UAudio_AdaptiveMusicManager: Fading out layer %d"), (int32)Layer);
        }
    }
}

void UAudio_AdaptiveMusicManager::CrossfadeToLayer(EAudio_MusicLayer NewLayer, float FadeTime)
{
    if (CurrentMusicState.ActiveLayer == NewLayer)
    {
        return;
    }
    
    // Fade out current layer
    FadeOutLayer(CurrentMusicState.ActiveLayer, FadeTime);
    
    // Fade in new layer
    FadeInLayer(NewLayer, FadeTime);
    
    CurrentMusicState.ActiveLayer = NewLayer;
    
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Crossfading to layer %d"), (int32)NewLayer);
}

void UAudio_AdaptiveMusicManager::UpdateIntensity(float DeltaTime)
{
    // Calculate target intensity based on game state
    float TargetIntensity = 0.0f;
    
    if (CurrentMusicState.bInCombat)
    {
        TargetIntensity = 1.0f;
    }
    else if (CurrentMusicState.bNearPredator)
    {
        TargetIntensity = 0.8f;
    }
    else
    {
        TargetIntensity = CurrentMusicState.FearLevel * 0.6f;
    }
    
    // Smooth interpolation to target intensity
    CurrentMusicState.IntensityLevel = FMath::FInterpTo(
        CurrentMusicState.IntensityLevel, 
        TargetIntensity, 
        DeltaTime, 
        2.0f
    );
    
    UpdateVolumeBasedOnIntensity();
}

void UAudio_AdaptiveMusicManager::PlayStinger(const FString& StingerName)
{
    if (USoundCue** StingerPtr = StingerSounds.Find(StingerName))
    {
        if (USoundCue* StingerSound = *StingerPtr)
        {
            if (UWorld* World = GetWorld())
            {
                UGameplayStatics::PlaySound2D(World, StingerSound);
                UE_LOG(LogTemp, Log, TEXT("UAudio_AdaptiveMusicManager: Playing stinger %s"), *StingerName);
            }
        }
    }
}

void UAudio_AdaptiveMusicManager::InitializeAudioComponents()
{
    if (UWorld* World = GetWorld())
    {
        // Create audio components for each music layer
        for (int32 i = 0; i < (int32)EAudio_MusicLayer::Discovery + 1; ++i)
        {
            EAudio_MusicLayer Layer = (EAudio_MusicLayer)i;
            
            UAudioComponent* AudioComp = NewObject<UAudioComponent>(this);
            if (AudioComp)
            {
                AudioComp->bAutoActivate = false;
                AudioComp->SetVolumeMultiplier(0.0f);
                MusicLayers.Add(Layer, AudioComp);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Initialized %d audio components"), MusicLayers.Num());
    }
}

void UAudio_AdaptiveMusicManager::CleanupAudioComponents()
{
    for (auto& LayerPair : MusicLayers)
    {
        if (UAudioComponent* AudioComp = LayerPair.Value)
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    
    MusicLayers.Empty();
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Cleaned up audio components"));
}

void UAudio_AdaptiveMusicManager::UpdateLayerBasedOnState()
{
    EAudio_MusicLayer TargetLayer = EAudio_MusicLayer::Ambient;
    
    if (CurrentMusicState.bInCombat)
    {
        TargetLayer = EAudio_MusicLayer::Combat;
    }
    else if (CurrentMusicState.FearLevel > 0.6f || CurrentMusicState.bNearPredator)
    {
        TargetLayer = EAudio_MusicLayer::Tension;
    }
    else if (CurrentMusicState.CurrentStoryPhase == EStoryPhase::Discovery || 
             CurrentMusicState.CurrentStoryPhase == EStoryPhase::Mastery)
    {
        TargetLayer = EAudio_MusicLayer::Discovery;
    }
    else
    {
        TargetLayer = EAudio_MusicLayer::Ambient;
    }
    
    if (CurrentMusicState.ActiveLayer != TargetLayer)
    {
        CrossfadeToLayer(TargetLayer);
    }
}

void UAudio_AdaptiveMusicManager::UpdateVolumeBasedOnIntensity()
{
    for (auto& LayerPair : MusicLayers)
    {
        if (UAudioComponent* AudioComp = LayerPair.Value)
        {
            float LayerVolume = 0.0f;
            
            if (LayerPair.Key == CurrentMusicState.ActiveLayer)
            {
                LayerVolume = FMath::Lerp(0.3f, 1.0f, CurrentMusicState.IntensityLevel);
            }
            
            AudioComp->SetVolumeMultiplier(LayerVolume);
        }
    }
}

void UAudio_AdaptiveMusicManager::HandleStoryPhaseTransition(EStoryPhase OldPhase, EStoryPhase NewPhase)
{
    FString TransitionStinger = FString::Printf(TEXT("Phase_%d_To_%d"), (int32)OldPhase, (int32)NewPhase);
    PlayStinger(TransitionStinger);
    
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Story phase transition from %d to %d"), 
           (int32)OldPhase, (int32)NewPhase);
}

void UAudio_AdaptiveMusicManager::HandleBiomeTransition(EAudio_BiomeTheme OldBiome, EAudio_BiomeTheme NewBiome)
{
    // Play biome transition stinger
    FString BiomeStinger = FString::Printf(TEXT("Biome_%d_Enter"), (int32)NewBiome);
    PlayStinger(BiomeStinger);
    
    // Crossfade to new biome's ambient layer
    CrossfadeToLayer(EAudio_MusicLayer::Ambient, 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("UAudio_AdaptiveMusicManager: Biome transition from %d to %d"), 
           (int32)OldBiome, (int32)NewBiome);
}