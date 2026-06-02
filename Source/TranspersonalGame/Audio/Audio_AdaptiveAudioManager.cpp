#include "Audio_AdaptiveAudioManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AAudio_AdaptiveAudioManager::AAudio_AdaptiveAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = true;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = true;

    EffectsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsAudio"));
    EffectsAudioComponent->SetupAttachment(RootComponent);
    EffectsAudioComponent->bAutoActivate = false;

    // Initialize default values
    TransitionSpeed = 1.0f;
    MaxAudioDistance = 5000.0f;
    CurrentVolumeMultiplier = 1.0f;
    TargetMusicVolume = 0.7f;
    CurrentMusicVolume = 0.7f;
    bIsTransitioning = false;
    TargetMusicLayer = EAudio_MusicLayer::Ambient;

    // Initialize music state
    CurrentMusicState.CurrentLayer = EAudio_MusicLayer::Ambient;
    CurrentMusicState.IntensityLevel = 0.5f;
    CurrentMusicState.EnvironmentType = EAudio_EnvironmentType::Forest;
    CurrentMusicState.bInDanger = false;
    CurrentMusicState.EmotionalIntensity = 0.0f;
}

void AAudio_AdaptiveAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Set initial ambient audio
    UpdateAmbientAudio();
    
    // Start with ambient music layer
    SetMusicLayer(EAudio_MusicLayer::Ambient, 0.0f);
}

void AAudio_AdaptiveAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update audio based on current game state
    UpdateAudioBasedOnGameState();
    
    // Handle music transitions
    HandleMusicTransition(DeltaTime);
}

void AAudio_AdaptiveAudioManager::SetMusicLayer(EAudio_MusicLayer NewLayer, float FadeTime)
{
    if (NewLayer != CurrentMusicState.CurrentLayer)
    {
        TargetMusicLayer = NewLayer;
        bIsTransitioning = true;
        
        // Find the sound cue for this layer
        if (MusicLayers.Contains(NewLayer))
        {
            USoundCue* NewMusic = MusicLayers[NewLayer];
            if (NewMusic && MusicAudioComponent)
            {
                if (FadeTime > 0.0f)
                {
                    // Implement crossfade logic here
                    MusicAudioComponent->FadeOut(FadeTime, 0.0f);
                    
                    // Use a timer to start new music after fade out
                    FTimerHandle FadeTimer;
                    GetWorld()->GetTimerManager().SetTimer(FadeTimer, [this, NewMusic, FadeTime]()
                    {
                        MusicAudioComponent->SetSound(NewMusic);
                        MusicAudioComponent->Play();
                        MusicAudioComponent->FadeIn(FadeTime, CurrentMusicVolume);
                        bIsTransitioning = false;
                    }, FadeTime, false);
                }
                else
                {
                    MusicAudioComponent->SetSound(NewMusic);
                    MusicAudioComponent->Play();
                    bIsTransitioning = false;
                }
            }
        }
        
        CurrentMusicState.CurrentLayer = NewLayer;
    }
}

void AAudio_AdaptiveAudioManager::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment)
{
    if (NewEnvironment != CurrentMusicState.EnvironmentType)
    {
        CurrentMusicState.EnvironmentType = NewEnvironment;
        UpdateAmbientAudio();
    }
}

void AAudio_AdaptiveAudioManager::SetDangerState(bool bDangerous, float IntensityLevel)
{
    CurrentMusicState.bInDanger = bDangerous;
    CurrentMusicState.IntensityLevel = FMath::Clamp(IntensityLevel, 0.0f, 1.0f);
    
    if (bDangerous)
    {
        if (IntensityLevel > 0.8f)
        {
            SetMusicLayer(EAudio_MusicLayer::Combat, 1.5f);
        }
        else
        {
            SetMusicLayer(EAudio_MusicLayer::Tension, 2.0f);
        }
        
        // Play danger stinger
        PlayDangerStinger();
    }
    else
    {
        // Return to exploration or calm music
        if (CurrentMusicState.EmotionalIntensity > 0.3f)
        {
            SetMusicLayer(EAudio_MusicLayer::Exploration, 3.0f);
        }
        else
        {
            SetMusicLayer(EAudio_MusicLayer::Calm, 4.0f);
        }
    }
}

void AAudio_AdaptiveAudioManager::UpdateEmotionalState(ENarr_EmotionalState EmotionalState, float Intensity)
{
    CurrentMusicState.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Adjust music based on emotional state
    switch (EmotionalState)
    {
        case ENarr_EmotionalState::Fear:
            if (!CurrentMusicState.bInDanger)
            {
                SetMusicLayer(EAudio_MusicLayer::Tension, 2.0f);
            }
            break;
            
        case ENarr_EmotionalState::Calm:
        case ENarr_EmotionalState::Relief:
            if (!CurrentMusicState.bInDanger)
            {
                SetMusicLayer(EAudio_MusicLayer::Calm, 3.0f);
            }
            break;
            
        case ENarr_EmotionalState::Excitement:
        case ENarr_EmotionalState::Curiosity:
            if (!CurrentMusicState.bInDanger)
            {
                SetMusicLayer(EAudio_MusicLayer::Exploration, 2.5f);
            }
            break;
            
        case ENarr_EmotionalState::Anger:
            SetMusicLayer(EAudio_MusicLayer::Combat, 1.0f);
            break;
            
        default:
            // Default to ambient if no specific emotional response
            if (!CurrentMusicState.bInDanger)
            {
                SetMusicLayer(EAudio_MusicLayer::Ambient, 2.0f);
            }
            break;
    }
}

void AAudio_AdaptiveAudioManager::PlayDangerStinger()
{
    if (DangerStingers.Num() > 0 && EffectsAudioComponent)
    {
        int32 RandomIndex = FMath::RandRange(0, DangerStingers.Num() - 1);
        USoundCue* StingerSound = DangerStingers[RandomIndex];
        
        if (StingerSound)
        {
            EffectsAudioComponent->SetSound(StingerSound);
            EffectsAudioComponent->Play();
        }
    }
}

void AAudio_AdaptiveAudioManager::UpdateMusicIntensity(float NewIntensity)
{
    CurrentMusicState.IntensityLevel = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    // Adjust volume based on intensity
    TargetMusicVolume = 0.5f + (CurrentMusicState.IntensityLevel * 0.3f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(TargetMusicVolume * CurrentVolumeMultiplier);
    }
}

void AAudio_AdaptiveAudioManager::TriggerEnvironmentalAudio(FVector Location, float Radius, USoundCue* SoundToPlay)
{
    if (SoundToPlay)
    {
        float DistanceToPlayer = CalculateDistanceToPlayer();
        
        if (DistanceToPlayer <= Radius)
        {
            // Calculate volume based on distance
            float VolumeMultiplier = 1.0f - (DistanceToPlayer / Radius);
            
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SoundToPlay,
                Location,
                VolumeMultiplier,
                1.0f,
                0.0f,
                nullptr,
                nullptr,
                true
            );
        }
    }
}

void AAudio_AdaptiveAudioManager::SetGlobalAudioVolume(float VolumeMultiplier)
{
    CurrentVolumeMultiplier = FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolumeMultiplier);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(TargetMusicVolume * CurrentVolumeMultiplier);
    }
}

void AAudio_AdaptiveAudioManager::UpdateAudioBasedOnGameState()
{
    // Check if player is in danger
    bool bPlayerInDanger = IsPlayerInDanger();
    
    if (bPlayerInDanger != CurrentMusicState.bInDanger)
    {
        SetDangerState(bPlayerInDanger, CurrentMusicState.IntensityLevel);
    }
    
    // Update intensity based on distance to threats
    float DistanceToPlayer = CalculateDistanceToPlayer();
    float NewIntensity = FMath::Clamp(1.0f - (DistanceToPlayer / MaxAudioDistance), 0.0f, 1.0f);
    
    if (FMath::Abs(NewIntensity - CurrentMusicState.IntensityLevel) > 0.1f)
    {
        UpdateMusicIntensity(NewIntensity);
    }
}

void AAudio_AdaptiveAudioManager::HandleMusicTransition(float DeltaTime)
{
    if (bIsTransitioning)
    {
        // Smooth volume transitions during music layer changes
        if (MusicAudioComponent)
        {
            float CurrentVolume = MusicAudioComponent->GetVolumeMultiplier();
            float NewVolume = FMath::FInterpTo(CurrentVolume, TargetMusicVolume * CurrentVolumeMultiplier, DeltaTime, TransitionSpeed);
            MusicAudioComponent->SetVolumeMultiplier(NewVolume);
            
            if (FMath::Abs(NewVolume - TargetMusicVolume * CurrentVolumeMultiplier) < 0.01f)
            {
                bIsTransitioning = false;
            }
        }
    }
}

void AAudio_AdaptiveAudioManager::UpdateAmbientAudio()
{
    if (AmbientSounds.Contains(CurrentMusicState.EnvironmentType) && AmbientAudioComponent)
    {
        USoundCue* AmbientSound = AmbientSounds[CurrentMusicState.EnvironmentType];
        if (AmbientSound)
        {
            AmbientAudioComponent->SetSound(AmbientSound);
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Play();
            }
        }
    }
}

float AAudio_AdaptiveAudioManager::CalculateDistanceToPlayer()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        return FVector::Dist(GetActorLocation(), PlayerLocation);
    }
    
    return MaxAudioDistance; // Return max distance if no player found
}

bool AAudio_AdaptiveAudioManager::IsPlayerInDanger()
{
    // Simple danger detection - check for nearby dinosaurs or threats
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        
        // Check for actors with "TRex", "Raptor" etc in their name within danger radius
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("TRex")) || 
                Actor->GetName().Contains(TEXT("Raptor")) ||
                Actor->GetName().Contains(TEXT("Danger")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                if (Distance < 1500.0f) // Danger radius
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}