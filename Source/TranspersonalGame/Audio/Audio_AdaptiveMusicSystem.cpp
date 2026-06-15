#include "Audio_AdaptiveMusicSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

UAudio_AdaptiveMusicSystem::UAudio_AdaptiveMusicSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds

    // Initialize state
    CurrentMusicState = EAudio_MusicState::Calm;
    CurrentBiomeType = EAudio_BiomeType::Forest;
    TensionLevel = 0.0f;
    bIsStorytellingActive = false;
    bIsNightTime = false;
    bIsFading = false;
    CurrentFadeTime = 0.0f;

    // Audio settings
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    StorytellingVolume = 0.9f;
    TensionThreshold = 0.5f;
    DinosaurProximityThreshold = 1000.0f;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    StorytellingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StorytellingAudioComponent"));
}

void UAudio_AdaptiveMusicSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    LoadDefaultMusicTracks();
    
    // Start with calm forest music
    SetMusicState(EAudio_MusicState::Calm);
    SetBiomeType(EAudio_BiomeType::Forest);
    
    // Setup tension decay timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TensionDecayTimerHandle,
            this,
            &UAudio_AdaptiveMusicSystem::DecayTensionOverTime,
            5.0f, // Every 5 seconds
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Initialized with %d music tracks"), MusicTracks.Num());
}

void UAudio_AdaptiveMusicSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle fade transitions
    if (bIsFading && MusicAudioComponent)
    {
        CurrentFadeTime += DeltaTime;
        // Fade logic would be implemented here with more complex audio mixing
    }
}

void UAudio_AdaptiveMusicSystem::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState != NewState)
    {
        EAudio_MusicState PreviousState = CurrentMusicState;
        CurrentMusicState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Music state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
        
        UpdateMusicBasedOnState();
    }
}

void UAudio_AdaptiveMusicSystem::SetBiomeType(EAudio_BiomeType NewBiome)
{
    if (CurrentBiomeType != NewBiome)
    {
        EAudio_BiomeType PreviousBiome = CurrentBiomeType;
        CurrentBiomeType = NewBiome;
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Biome changed from %d to %d"), 
               (int32)PreviousBiome, (int32)NewBiome);
        
        UpdateMusicBasedOnState();
    }
}

void UAudio_AdaptiveMusicSystem::PlayStorytellingAudio(const FAudio_StorytellingAudioData& AudioData)
{
    if (!StorytellingAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicSystem: StorytellingAudioComponent is null"));
        return;
    }
    
    // Stop any current storytelling
    StopStorytellingAudio();
    
    bIsStorytellingActive = true;
    
    // Lower music volume during storytelling
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * 0.3f);
    }
    
    // Note: In a full implementation, we would load the audio from the URL
    // For now, we simulate the storytelling duration
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Playing storytelling audio for %s (Duration: %.1fs)"), 
           *AudioData.CharacterName, AudioData.Duration);
    
    // Set timer to finish storytelling
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            StorytellingTimerHandle,
            this,
            &UAudio_AdaptiveMusicSystem::OnStorytellingFinished,
            AudioData.Duration,
            false
        );
    }
}

void UAudio_AdaptiveMusicSystem::StopStorytellingAudio()
{
    if (bIsStorytellingActive)
    {
        bIsStorytellingActive = false;
        
        if (StorytellingAudioComponent && StorytellingAudioComponent->IsPlaying())
        {
            StorytellingAudioComponent->Stop();
        }
        
        // Restore music volume
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
        }
        
        // Clear timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(StorytellingTimerHandle);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Storytelling audio stopped"));
    }
}

void UAudio_AdaptiveMusicSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (StorytellingAudioComponent)
    {
        StorytellingAudioComponent->SetVolumeMultiplier(StorytellingVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Master volume set to %.2f"), MasterVolume);
}

void UAudio_AdaptiveMusicSystem::FadeToTrack(const FAudio_MusicTrack& NewTrack, float FadeTime)
{
    if (!MusicAudioComponent)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Fading to track '%s' over %.1f seconds"), 
           *NewTrack.TrackName, FadeTime);
    
    StartFadeTransition(NewTrack, FadeTime);
}

void UAudio_AdaptiveMusicSystem::IncreaseTension(float Amount)
{
    TensionLevel = FMath::Clamp(TensionLevel + Amount, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Tension increased to %.2f"), TensionLevel);
    
    // Check if we need to transition to tension/combat music
    if (TensionLevel >= TensionThreshold)
    {
        if (CurrentMusicState == EAudio_MusicState::Calm || CurrentMusicState == EAudio_MusicState::Exploration)
        {
            SetMusicState(EAudio_MusicState::Tension);
        }
    }
}

void UAudio_AdaptiveMusicSystem::DecreaseTension(float Amount)
{
    TensionLevel = FMath::Clamp(TensionLevel - Amount, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Tension decreased to %.2f"), TensionLevel);
    
    // Check if we can return to calm music
    if (TensionLevel < TensionThreshold * 0.5f)
    {
        if (CurrentMusicState == EAudio_MusicState::Tension)
        {
            SetMusicState(EAudio_MusicState::Calm);
        }
    }
}

void UAudio_AdaptiveMusicSystem::ResetTension()
{
    TensionLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Tension reset to 0"));
    
    if (CurrentMusicState == EAudio_MusicState::Tension || CurrentMusicState == EAudio_MusicState::Combat)
    {
        SetMusicState(EAudio_MusicState::Calm);
    }
}

void UAudio_AdaptiveMusicSystem::OnDayNightTransition(bool bIsNight)
{
    bIsNightTime = bIsNight;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Day/Night transition - IsNight: %s"), 
           bIsNight ? TEXT("true") : TEXT("false"));
    
    // Transition to appropriate music
    if (bIsNight)
    {
        SetMusicState(EAudio_MusicState::Night);
    }
    else
    {
        SetMusicState(EAudio_MusicState::Day);
    }
}

void UAudio_AdaptiveMusicSystem::OnDinosaurProximity(float Distance, bool bIsPredator)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Dinosaur proximity - Distance: %.1f, IsPredator: %s"), 
           Distance, bIsPredator ? TEXT("true") : TEXT("false"));
    
    if (Distance < DinosaurProximityThreshold)
    {
        if (bIsPredator)
        {
            // Immediate tension increase for predators
            IncreaseTension(0.3f);
            
            if (Distance < DinosaurProximityThreshold * 0.5f)
            {
                SetMusicState(EAudio_MusicState::Combat);
            }
            else
            {
                SetMusicState(EAudio_MusicState::Tension);
            }
        }
        else
        {
            // Gentle tension increase for herbivores
            IncreaseTension(0.1f);
        }
    }
}

void UAudio_AdaptiveMusicSystem::OnDinosaurLeft()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Dinosaur left area"));
    
    // Gradual tension decrease
    DecreaseTension(0.2f);
}

void UAudio_AdaptiveMusicSystem::UpdateMusicBasedOnState()
{
    FAudio_MusicTrack* BestTrack = FindBestTrackForCurrentState();
    
    if (BestTrack)
    {
        FadeToTrack(*BestTrack, BestTrack->FadeInTime);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicSystem: No suitable track found for current state"));
    }
}

FAudio_MusicTrack* UAudio_AdaptiveMusicSystem::FindBestTrackForCurrentState()
{
    // Find tracks that match current state and biome
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == CurrentMusicState && Track.BiomeType == CurrentBiomeType)
        {
            return &Track;
        }
    }
    
    // Fallback: find tracks that match just the state
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == CurrentMusicState)
        {
            return &Track;
        }
    }
    
    // Last resort: return first track if any exist
    if (MusicTracks.Num() > 0)
    {
        return &MusicTracks[0];
    }
    
    return nullptr;
}

void UAudio_AdaptiveMusicSystem::StartFadeTransition(const FAudio_MusicTrack& NewTrack, float FadeTime)
{
    bIsFading = true;
    CurrentFadeTime = 0.0f;
    
    // In a full implementation, this would handle complex audio crossfading
    // For now, we simulate the transition
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Starting fade transition to '%s'"), *NewTrack.TrackName);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(NewTrack.Volume * MasterVolume);
    }
    
    // Reset fade state after transition
    if (UWorld* World = GetWorld())
    {
        FTimerHandle FadeTimerHandle;
        World->GetTimerManager().SetTimer(
            FadeTimerHandle,
            [this]() { bIsFading = false; },
            FadeTime,
            false
        );
    }
}

void UAudio_AdaptiveMusicSystem::OnStorytellingFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Storytelling finished"));
    StopStorytellingAudio();
}

void UAudio_AdaptiveMusicSystem::DecayTensionOverTime()
{
    if (TensionLevel > 0.0f)
    {
        DecreaseTension(0.05f); // Slowly decay tension over time
    }
}

void UAudio_AdaptiveMusicSystem::InitializeAudioComponents()
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = true;
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (StorytellingAudioComponent)
    {
        StorytellingAudioComponent->bAutoActivate = false;
        StorytellingAudioComponent->SetVolumeMultiplier(StorytellingVolume * MasterVolume);
    }
}

void UAudio_AdaptiveMusicSystem::LoadDefaultMusicTracks()
{
    // Create default music tracks for different states and biomes
    FAudio_MusicTrack CalmForestTrack;
    CalmForestTrack.TrackName = TEXT("Calm Forest Ambience");
    CalmForestTrack.MusicState = EAudio_MusicState::Calm;
    CalmForestTrack.BiomeType = EAudio_BiomeType::Forest;
    CalmForestTrack.Volume = 0.6f;
    MusicTracks.Add(CalmForestTrack);
    
    FAudio_MusicTrack TensionTrack;
    TensionTrack.TrackName = TEXT("Prehistoric Tension");
    TensionTrack.MusicState = EAudio_MusicState::Tension;
    TensionTrack.BiomeType = EAudio_BiomeType::Forest;
    TensionTrack.Volume = 0.8f;
    MusicTracks.Add(TensionTrack);
    
    FAudio_MusicTrack CombatTrack;
    CombatTrack.TrackName = TEXT("Dinosaur Combat");
    CombatTrack.MusicState = EAudio_MusicState::Combat;
    CombatTrack.BiomeType = EAudio_BiomeType::Forest;
    CombatTrack.Volume = 0.9f;
    MusicTracks.Add(CombatTrack);
    
    FAudio_MusicTrack NightTrack;
    NightTrack.TrackName = TEXT("Prehistoric Night");
    NightTrack.MusicState = EAudio_MusicState::Night;
    NightTrack.BiomeType = EAudio_BiomeType::Forest;
    NightTrack.Volume = 0.5f;
    MusicTracks.Add(NightTrack);
    
    FAudio_MusicTrack StorytellingTrack;
    StorytellingTrack.TrackName = TEXT("Tribal Storytelling");
    StorytellingTrack.MusicState = EAudio_MusicState::Storytelling;
    StorytellingTrack.BiomeType = EAudio_BiomeType::Forest;
    StorytellingTrack.Volume = 0.4f;
    MusicTracks.Add(StorytellingTrack);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AdaptiveMusicSystem: Loaded %d default music tracks"), MusicTracks.Num());
}