#include "Audio_AdaptiveMusicSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAudio_AdaptiveMusicSystem::AAudio_AdaptiveMusicSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    RootComponent = MusicAudioComponent;
    MusicAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    TribalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TribalAudioComponent"));
    TribalAudioComponent->SetupAttachment(RootComponent);
    TribalAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentMusicState = EAudio_MusicState::Silence;
    CurrentIntensity = EAudio_IntensityLevel::Calm;
    MasterMusicVolume = 0.7f;
    MasterAmbientVolume = 0.5f;
    CurrentTribalTerritory = TEXT("Neutral");
    CurrentTribalRelationship = 0.0f;

    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    TargetMusicState = EAudio_MusicState::Silence;
    TargetIntensity = EAudio_IntensityLevel::Calm;

    // Setup default tribal zones
    FAudio_TribalZoneSettings StormwatcherZone;
    StormwatcherZone.TribeName = TEXT("Stormwatcher Clan");
    StormwatcherZone.ZoneCenter = FVector(-2000, -2000, 100);
    StormwatcherZone.ZoneRadius = 1500.0f;
    StormwatcherZone.RelationshipModifier = 25.0f;

    FAudio_TribalZoneSettings BonecrusherZone;
    BonecrusherZone.TribeName = TEXT("Bonecrusher Pack");
    BonecrusherZone.ZoneCenter = FVector(2000, 2000, 300);
    BonecrusherZone.ZoneRadius = 1200.0f;
    BonecrusherZone.RelationshipModifier = -15.0f;

    FAudio_TribalZoneSettings GathererZone;
    GathererZone.TribeName = TEXT("Gatherer Circle");
    GathererZone.ZoneCenter = FVector(0, -3000, 50);
    GathererZone.ZoneRadius = 1800.0f;
    GathererZone.RelationshipModifier = 10.0f;

    TribalZones.Add(StormwatcherZone);
    TribalZones.Add(BonecrusherZone);
    TribalZones.Add(GathererZone);
}

void AAudio_AdaptiveMusicSystem::BeginPlay()
{
    Super::BeginPlay();

    // Set initial volumes
    MusicAudioComponent->SetVolumeMultiplier(MasterMusicVolume);
    AmbientAudioComponent->SetVolumeMultiplier(MasterAmbientVolume);
    TribalAudioComponent->SetVolumeMultiplier(MasterAmbientVolume * 0.8f);

    // Start with exploration music
    TransitionToMusicState(EAudio_MusicState::Exploration, EAudio_IntensityLevel::Calm);

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicSystem initialized with %d tribal zones"), TribalZones.Num());
}

void AAudio_AdaptiveMusicSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle music transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        
        // Check if we should complete the transition
        if (TransitionTimer >= 3.0f) // 3 second transition time
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            CurrentMusicState = TargetMusicState;
            CurrentIntensity = TargetIntensity;
        }
    }

    // Check player location for tribal territory updates
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (const FAudio_TribalZoneSettings& Zone : TribalZones)
        {
            float DistanceToZone = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
            
            if (DistanceToZone <= Zone.ZoneRadius)
            {
                if (CurrentTribalTerritory != Zone.TribeName)
                {
                    UpdateTribalTerritory(Zone.TribeName, Zone.RelationshipModifier);
                }
                break;
            }
        }
    }
}

void AAudio_AdaptiveMusicSystem::TransitionToMusicState(EAudio_MusicState NewState, EAudio_IntensityLevel NewIntensity)
{
    if (NewState == CurrentMusicState && NewIntensity == CurrentIntensity && !bIsTransitioning)
    {
        return; // Already in this state
    }

    TargetMusicState = NewState;
    TargetIntensity = NewIntensity;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;

    // Find and play the appropriate track
    FAudio_MusicTrack* NewTrack = FindMusicTrack(NewState, NewIntensity);
    if (NewTrack)
    {
        CrossfadeToTrack(*NewTrack);
    }

    UE_LOG(LogTemp, Log, TEXT("Transitioning to music state: %d, intensity: %d"), 
           (int32)NewState, (int32)NewIntensity);
}

void AAudio_AdaptiveMusicSystem::SetMusicIntensity(EAudio_IntensityLevel NewIntensity)
{
    TransitionToMusicState(CurrentMusicState, NewIntensity);
}

void AAudio_AdaptiveMusicSystem::PlayTribalMusic(const FString& TribeName, float RelationshipLevel)
{
    // Find the tribal zone
    for (const FAudio_TribalZoneSettings& Zone : TribalZones)
    {
        if (Zone.TribeName == TribeName)
        {
            if (Zone.TribalMusic.IsValid())
            {
                USoundCue* TribalSoundCue = Zone.TribalMusic.LoadSynchronous();
                if (TribalSoundCue)
                {
                    TribalAudioComponent->SetSound(TribalSoundCue);
                    
                    // Adjust volume based on relationship
                    float VolumeMultiplier = FMath::Clamp((RelationshipLevel + 100.0f) / 200.0f, 0.1f, 1.0f);
                    TribalAudioComponent->SetVolumeMultiplier(MasterAmbientVolume * VolumeMultiplier);
                    
                    TribalAudioComponent->Play();
                }
            }
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Playing tribal music for %s with relationship %f"), 
           *TribeName, RelationshipLevel);
}

void AAudio_AdaptiveMusicSystem::StopAllMusic(float FadeOutTime)
{
    MusicAudioComponent->FadeOut(FadeOutTime, 0.0f);
    TribalAudioComponent->FadeOut(FadeOutTime, 0.0f);
    
    CurrentMusicState = EAudio_MusicState::Silence;
    bIsTransitioning = false;

    UE_LOG(LogTemp, Log, TEXT("Stopping all music with fade out time: %f"), FadeOutTime);
}

void AAudio_AdaptiveMusicSystem::UpdateTribalTerritory(const FString& TribeName, float RelationshipLevel)
{
    CurrentTribalTerritory = TribeName;
    CurrentTribalRelationship = RelationshipLevel;

    // Update ambient audio based on territory
    for (const FAudio_TribalZoneSettings& Zone : TribalZones)
    {
        if (Zone.TribeName == TribeName)
        {
            if (Zone.AmbientSound.IsValid())
            {
                USoundCue* AmbientSoundCue = Zone.AmbientSound.LoadSynchronous();
                if (AmbientSoundCue)
                {
                    AmbientAudioComponent->SetSound(AmbientSoundCue);
                    AmbientAudioComponent->Play();
                }
            }
            break;
        }
    }

    // Transition to tribal music state
    TransitionToMusicState(EAudio_MusicState::Tribal, EAudio_IntensityLevel::Calm);
    
    // Update tribal audio based on relationship
    UpdateTribalAudioBasedOnRelationship(RelationshipLevel);

    UE_LOG(LogTemp, Warning, TEXT("Entered %s territory with relationship level %f"), 
           *TribeName, RelationshipLevel);
}

void AAudio_AdaptiveMusicSystem::PlayEnvironmentalStory(const FString& LocationName)
{
    // This would trigger location-specific audio cues
    // For now, log the event
    UE_LOG(LogTemp, Log, TEXT("Playing environmental story audio for location: %s"), *LocationName);
    
    // Could trigger specific sound effects or ambient changes based on location
    if (LocationName.Contains(TEXT("Cave")))
    {
        // Play cave reverb and dripping sounds
        SetMusicIntensity(EAudio_IntensityLevel::Tense);
    }
    else if (LocationName.Contains(TEXT("BoneYard")))
    {
        // Play ominous wind and bone rattling
        SetMusicIntensity(EAudio_IntensityLevel::Intense);
    }
    else if (LocationName.Contains(TEXT("SacredGrove")))
    {
        // Play mystical forest sounds
        TransitionToMusicState(EAudio_MusicState::Tribal, EAudio_IntensityLevel::Calm);
    }
}

void AAudio_AdaptiveMusicSystem::OnDinosaurEncounter(const FString& DinosaurType, float ThreatLevel)
{
    EAudio_IntensityLevel NewIntensity = EAudio_IntensityLevel::Calm;
    
    if (ThreatLevel >= 0.8f)
    {
        NewIntensity = EAudio_IntensityLevel::Extreme;
    }
    else if (ThreatLevel >= 0.6f)
    {
        NewIntensity = EAudio_IntensityLevel::Intense;
    }
    else if (ThreatLevel >= 0.3f)
    {
        NewIntensity = EAudio_IntensityLevel::Tense;
    }

    TransitionToMusicState(EAudio_MusicState::Danger, NewIntensity);

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur encounter: %s with threat level %f"), 
           *DinosaurType, ThreatLevel);
}

void AAudio_AdaptiveMusicSystem::OnCombatStart()
{
    TransitionToMusicState(EAudio_MusicState::Combat, EAudio_IntensityLevel::Intense);
    UE_LOG(LogTemp, Warning, TEXT("Combat started - switching to combat music"));
}

void AAudio_AdaptiveMusicSystem::OnCombatEnd(bool bPlayerVictory)
{
    if (bPlayerVictory)
    {
        TransitionToMusicState(EAudio_MusicState::Victory, EAudio_IntensityLevel::Calm);
    }
    else
    {
        TransitionToMusicState(EAudio_MusicState::Exploration, EAudio_IntensityLevel::Tense);
    }

    UE_LOG(LogTemp, Log, TEXT("Combat ended - player victory: %s"), 
           bPlayerVictory ? TEXT("true") : TEXT("false"));
}

FAudio_MusicTrack* AAudio_AdaptiveMusicSystem::FindMusicTrack(EAudio_MusicState State, EAudio_IntensityLevel Intensity)
{
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == State && Track.IntensityLevel == Intensity)
        {
            return &Track;
        }
    }
    
    // Fallback to any track with the same state
    for (FAudio_MusicTrack& Track : MusicTracks)
    {
        if (Track.MusicState == State)
        {
            return &Track;
        }
    }
    
    return nullptr;
}

void AAudio_AdaptiveMusicSystem::CrossfadeToTrack(const FAudio_MusicTrack& NewTrack)
{
    if (NewTrack.SoundCue.IsValid())
    {
        USoundCue* SoundCue = NewTrack.SoundCue.LoadSynchronous();
        if (SoundCue)
        {
            // Fade out current track
            if (MusicAudioComponent->IsPlaying())
            {
                MusicAudioComponent->FadeOut(NewTrack.FadeOutTime, 0.0f);
            }

            // Set new track and fade in
            MusicAudioComponent->SetSound(SoundCue);
            MusicAudioComponent->FadeIn(NewTrack.FadeInTime, MasterMusicVolume);
        }
    }
}

void AAudio_AdaptiveMusicSystem::UpdateTribalAudioBasedOnRelationship(float RelationshipLevel)
{
    // Adjust tribal audio volume and pitch based on relationship
    float VolumeMultiplier = FMath::Clamp((RelationshipLevel + 100.0f) / 200.0f, 0.2f, 1.0f);
    float PitchMultiplier = FMath::Clamp(1.0f + (RelationshipLevel / 200.0f), 0.8f, 1.2f);

    TribalAudioComponent->SetVolumeMultiplier(MasterAmbientVolume * VolumeMultiplier);
    TribalAudioComponent->SetPitchMultiplier(PitchMultiplier);

    UE_LOG(LogTemp, Log, TEXT("Updated tribal audio - Volume: %f, Pitch: %f"), 
           VolumeMultiplier, PitchMultiplier);
}