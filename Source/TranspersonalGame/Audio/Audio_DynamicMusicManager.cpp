#include "Audio_DynamicMusicManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UAudio_DynamicMusicManager::UAudio_DynamicMusicManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Create audio components
    PrimaryMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryMusicComponent"));
    SecondaryMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryMusicComponent"));
    
    // Initialize default values
    CurrentMusicState = EAudio_MusicState::Calm;
    CurrentStoryPhase = EAudio_StoryPhase::Awakening;
    MasterMusicVolume = 0.7f;
    CrossfadeTime = 3.0f;
    bIsTransitioning = false;
    
    // Configure audio components
    if (PrimaryMusicComponent)
    {
        PrimaryMusicComponent->bAutoActivate = false;
        PrimaryMusicComponent->SetVolumeMultiplier(MasterMusicVolume);
    }
    
    if (SecondaryMusicComponent)
    {
        SecondaryMusicComponent->bAutoActivate = false;
        SecondaryMusicComponent->SetVolumeMultiplier(0.0f);
    }
}

void UAudio_DynamicMusicManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultTracks();
    
    // Start with awakening calm music
    PlayMusicForStateAndPhase(EAudio_MusicState::Calm, EAudio_StoryPhase::Awakening);
    
    UE_LOG(LogTemp, Log, TEXT("Dynamic Music Manager initialized"));
}

void UAudio_DynamicMusicManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle crossfade transitions
    if (bIsTransitioning && PrimaryMusicComponent && SecondaryMusicComponent)
    {
        // This would contain crossfade logic in a full implementation
        // For now, we'll keep it simple
    }
}

void UAudio_DynamicMusicManager::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState != NewState)
    {
        CurrentMusicState = NewState;
        PlayMusicForStateAndPhase(CurrentMusicState, CurrentStoryPhase);
        
        UE_LOG(LogTemp, Log, TEXT("Music state changed to: %d"), (int32)NewState);
    }
}

void UAudio_DynamicMusicManager::SetStoryPhase(EAudio_StoryPhase NewPhase)
{
    if (CurrentStoryPhase != NewPhase)
    {
        CurrentStoryPhase = NewPhase;
        PlayMusicForStateAndPhase(CurrentMusicState, CurrentStoryPhase);
        
        UE_LOG(LogTemp, Log, TEXT("Story phase changed to: %d"), (int32)NewPhase);
    }
}

void UAudio_DynamicMusicManager::PlayMusicForStateAndPhase(EAudio_MusicState State, EAudio_StoryPhase Phase)
{
    FAudio_MusicTrack* Track = FindTrackForStateAndPhase(State, Phase);
    
    if (Track && Track->SoundCue.LoadSynchronous())
    {
        CrossfadeToTrack(*Track);
        UE_LOG(LogTemp, Log, TEXT("Playing music for State: %d, Phase: %d"), (int32)State, (int32)Phase);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No music track found for State: %d, Phase: %d"), (int32)State, (int32)Phase);
    }
}

void UAudio_DynamicMusicManager::StopAllMusic()
{
    if (PrimaryMusicComponent && PrimaryMusicComponent->IsPlaying())
    {
        PrimaryMusicComponent->FadeOut(CrossfadeTime, 0.0f);
    }
    
    if (SecondaryMusicComponent && SecondaryMusicComponent->IsPlaying())
    {
        SecondaryMusicComponent->FadeOut(CrossfadeTime, 0.0f);
    }
    
    bIsTransitioning = false;
    UE_LOG(LogTemp, Log, TEXT("All music stopped"));
}

void UAudio_DynamicMusicManager::SetMasterVolume(float Volume)
{
    MasterMusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (PrimaryMusicComponent)
    {
        PrimaryMusicComponent->SetVolumeMultiplier(MasterMusicVolume);
    }
    
    if (SecondaryMusicComponent)
    {
        SecondaryMusicComponent->SetVolumeMultiplier(MasterMusicVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Master music volume set to: %f"), MasterMusicVolume);
}

void UAudio_DynamicMusicManager::OnMusicFinished()
{
    // Handle music track completion
    bIsTransitioning = false;
    UE_LOG(LogTemp, Log, TEXT("Music track finished"));
}

void UAudio_DynamicMusicManager::CrossfadeToTrack(const FAudio_MusicTrack& NewTrack)
{
    if (!PrimaryMusicComponent || !SecondaryMusicComponent)
    {
        return;
    }
    
    USoundCue* SoundCue = NewTrack.SoundCue.LoadSynchronous();
    if (!SoundCue)
    {
        return;
    }
    
    // Simple crossfade implementation
    if (PrimaryMusicComponent->IsPlaying())
    {
        // Fade out primary, start secondary
        PrimaryMusicComponent->FadeOut(NewTrack.FadeOutTime, 0.0f);
        SecondaryMusicComponent->SetSound(SoundCue);
        SecondaryMusicComponent->FadeIn(NewTrack.FadeInTime, MasterMusicVolume);
        
        // Swap components for next transition
        UAudioComponent* Temp = PrimaryMusicComponent;
        PrimaryMusicComponent = SecondaryMusicComponent;
        SecondaryMusicComponent = Temp;
    }
    else
    {
        // Start primary directly
        PrimaryMusicComponent->SetSound(SoundCue);
        PrimaryMusicComponent->FadeIn(NewTrack.FadeInTime, MasterMusicVolume);
    }
    
    bIsTransitioning = true;
}

FAudio_MusicTrack* UAudio_DynamicMusicManager::FindTrackForStateAndPhase(EAudio_MusicState State, EAudio_StoryPhase Phase)
{
    // First try to find exact match
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == State && Track.StoryPhase == Phase)
        {
            return &Track;
        }
    }
    
    // Fallback to any track with matching state
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == State)
        {
            return &Track;
        }
    }
    
    // Last resort - return first track if any exist
    if (MusicTracks.Num() > 0)
    {
        return &MusicTracks[0];
    }
    
    return nullptr;
}

void UAudio_DynamicMusicManager::InitializeDefaultTracks()
{
    // Initialize with placeholder tracks for each state/phase combination
    // In a full implementation, these would be loaded from assets
    
    MusicTracks.Empty();
    
    // Awakening phase tracks
    FAudio_MusicTrack AwakeningCalm;
    AwakeningCalm.MusicState = EAudio_MusicState::Calm;
    AwakeningCalm.StoryPhase = EAudio_StoryPhase::Awakening;
    MusicTracks.Add(AwakeningCalm);
    
    FAudio_MusicTrack AwakeningDanger;
    AwakeningDanger.MusicState = EAudio_MusicState::Danger;
    AwakeningDanger.StoryPhase = EAudio_StoryPhase::Awakening;
    MusicTracks.Add(AwakeningDanger);
    
    // Survival phase tracks
    FAudio_MusicTrack SurvivalExploration;
    SurvivalExploration.MusicState = EAudio_MusicState::Exploration;
    SurvivalExploration.StoryPhase = EAudio_StoryPhase::Survival;
    MusicTracks.Add(SurvivalExploration);
    
    FAudio_MusicTrack SurvivalCombat;
    SurvivalCombat.MusicState = EAudio_MusicState::Combat;
    SurvivalCombat.StoryPhase = EAudio_StoryPhase::Survival;
    MusicTracks.Add(SurvivalCombat);
    
    // Discovery phase tracks
    FAudio_MusicTrack DiscoveryCalm;
    DiscoveryCalm.MusicState = EAudio_MusicState::Calm;
    DiscoveryCalm.StoryPhase = EAudio_StoryPhase::Discovery;
    MusicTracks.Add(DiscoveryCalm);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default music tracks"), MusicTracks.Num());
}