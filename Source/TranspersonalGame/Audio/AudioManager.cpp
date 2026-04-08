#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudioManager::AAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    AmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceComponent"));
    TensionComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TensionComponent"));

    // Set default values
    CurrentAudioState = EAudioState::Exploration;
    CurrentEnvironment = EEnvironmentType::Forest;
    CurrentTensionLevel = 0.0f;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
}

void AAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio components
    if (MusicComponent)
    {
        MusicComponent->bAutoActivate = false;
        MusicComponent->VolumeMultiplier = 0.7f;
    }
    
    if (AmbienceComponent)
    {
        AmbienceComponent->bAutoActivate = false;
        AmbienceComponent->VolumeMultiplier = 0.5f;
    }
    
    if (TensionComponent)
    {
        TensionComponent->bAutoActivate = false;
        TensionComponent->VolumeMultiplier = 0.0f;
    }

    // Start with exploration state
    SetAudioState(EAudioState::Exploration, 0.0f);
}

void AAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTimer -= DeltaTime;
        if (StateTransitionTimer <= 0.0f)
        {
            bIsTransitioning = false;
            CurrentAudioState = TargetAudioState;
        }
    }
    
    // Update music layers based on current state
    UpdateMusicLayers();
    
    // Process ambient environmental sounds
    ProcessAmbientSounds();
}

void AAudioManager::SetAudioState(EAudioState NewState, float TransitionTime)
{
    if (NewState == CurrentAudioState && !bIsTransitioning)
        return;
        
    TransitionToState(NewState, TransitionTime);
}

void AAudioManager::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (NewEnvironment != CurrentEnvironment)
    {
        CurrentEnvironment = NewEnvironment;
        // Trigger ambient sound change
        ProcessAmbientSounds();
    }
}

void AAudioManager::UpdateTensionLevel(float NewTensionLevel)
{
    CurrentTensionLevel = FMath::Clamp(NewTensionLevel, 0.0f, 1.0f);
    
    // Auto-transition based on tension thresholds
    if (CurrentTensionLevel >= 0.8f && CurrentAudioState != EAudioState::Danger)
    {
        SetAudioState(EAudioState::Danger, 1.5f);
    }
    else if (CurrentTensionLevel >= 0.5f && CurrentAudioState == EAudioState::Exploration)
    {
        SetAudioState(EAudioState::Tension, 2.0f);
    }
    else if (CurrentTensionLevel < 0.3f && CurrentAudioState != EAudioState::Exploration)
    {
        SetAudioState(EAudioState::Exploration, 3.0f);
    }
}

void AAudioManager::PlayDinosaurSound(ADinosaur* Dinosaur, const FString& SoundType)
{
    if (!Dinosaur)
        return;
        
    // This will be implemented when Dinosaur class is available
    // For now, create placeholder for dynamic dinosaur audio
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    // Calculate distance-based volume
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Player)
    {
        float Distance = FVector::Dist(DinosaurLocation, Player->GetActorLocation());
        float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / 3000.0f), 0.1f, 1.0f);
        
        // Increase tension based on proximity and dinosaur type
        if (Distance < 1500.0f)
        {
            float TensionIncrease = (1500.0f - Distance) / 1500.0f * 0.3f;
            UpdateTensionLevel(CurrentTensionLevel + TensionIncrease);
        }
    }
}

void AAudioManager::PlayEnvironmentalSound(FVector Location, USoundCue* SoundCue, float VolumeMultiplier)
{
    if (!SoundCue)
        return;
        
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
}

void AAudioManager::TransitionToState(EAudioState NewState, float TransitionTime)
{
    TargetAudioState = NewState;
    StateTransitionTimer = TransitionTime;
    bIsTransitioning = true;
    
    // Immediate changes for critical states
    if (NewState == EAudioState::Danger || NewState == EAudioState::Combat)
    {
        if (TensionComponent)
        {
            TensionComponent->SetVolumeMultiplier(0.8f);
            TensionComponent->FadeIn(0.5f);
        }
    }
}

void AAudioManager::UpdateMusicLayers()
{
    if (!MusicComponent || !AudioStates.Contains(CurrentAudioState))
        return;
        
    const FAudioStateConfig& StateConfig = AudioStates[CurrentAudioState];
    
    // Update music volume based on tension level
    float MusicVolume = 0.7f;
    
    switch (CurrentAudioState)
    {
        case EAudioState::Exploration:
            MusicVolume = 0.6f - (CurrentTensionLevel * 0.3f);
            break;
        case EAudioState::Tension:
            MusicVolume = 0.4f + (CurrentTensionLevel * 0.2f);
            break;
        case EAudioState::Danger:
            MusicVolume = 0.8f;
            break;
        case EAudioState::Combat:
            MusicVolume = 0.9f;
            break;
        case EAudioState::Safe:
            MusicVolume = 0.8f;
            break;
    }
    
    MusicComponent->SetVolumeMultiplier(MusicVolume);
    
    // Update tension layer
    if (TensionComponent)
    {
        float TensionVolume = CurrentTensionLevel * 0.6f;
        TensionComponent->SetVolumeMultiplier(TensionVolume);
    }
}

void AAudioManager::ProcessAmbientSounds()
{
    if (!AmbienceComponent)
        return;
        
    // Environment-specific ambient processing
    float AmbienceVolume = 0.5f;
    
    switch (CurrentEnvironment)
    {
        case EEnvironmentType::Forest:
            AmbienceVolume = 0.6f - (CurrentTensionLevel * 0.2f);
            break;
        case EEnvironmentType::Cave:
            AmbienceVolume = 0.3f + (CurrentTensionLevel * 0.3f);
            break;
        case EEnvironmentType::River:
            AmbienceVolume = 0.7f;
            break;
        default:
            AmbienceVolume = 0.5f;
            break;
    }
    
    AmbienceComponent->SetVolumeMultiplier(AmbienceVolume);
}