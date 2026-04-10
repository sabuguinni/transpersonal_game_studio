#include "TranspersonalAudioManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

ATranspersonalAudioManager::ATranspersonalAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    RootComponent = MusicAudioComponent;
    MusicAudioComponent->bAutoActivate = false;
    MusicAudioComponent->SetVolumeMultiplier(0.8f);

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.6f);

    TransitionAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TransitionAudioComponent"));
    TransitionAudioComponent->SetupAttachment(RootComponent);
    TransitionAudioComponent->bAutoActivate = false;
    TransitionAudioComponent->SetVolumeMultiplier(1.0f);

    SpatialAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SpatialAudioComponent"));
    SpatialAudioComponent->SetupAttachment(RootComponent);
    SpatialAudioComponent->bAutoActivate = false;
    SpatialAudioComponent->SetVolumeMultiplier(0.7f);

    // Initialize default values
    CurrentConsciousnessState = EConsciousnessAudioState::Dormant;
    CurrentEnvironmentType = EEnvironmentAudioType::Forest;
    CurrentMusicIntensity = 0.5f;
    TargetMusicIntensity = 0.5f;
    MusicIntensityChangeRate = 0.5f;
    
    MasterVolume = 1.0f;
    MusicVolume = 0.8f;
    AmbientVolume = 0.6f;
    SpatialAudioRange = 2000.0f;
    
    bEnableAdaptiveMusic = true;
    bEnableSpatialAudio = true;
    
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 0.0f;
    
    PendingConsciousnessState = EConsciousnessAudioState::Dormant;
    PendingEnvironmentType = EEnvironmentAudioType::Forest;
}

void ATranspersonalAudioManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Initializing audio system"));

    // Apply initial audio state
    ApplyConsciousnessAudioState(CurrentConsciousnessState);
    ApplyEnvironmentAudioLayer(CurrentEnvironmentType);
    
    // Update initial audio parameters
    UpdateAudioParameters();

    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Audio system initialized"));
}

void ATranspersonalAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableAdaptiveMusic)
    {
        UpdateAdaptiveMusic(DeltaTime);
    }

    if (bIsTransitioning)
    {
        ProcessConsciousnessTransition(DeltaTime);
        ProcessEnvironmentTransition(DeltaTime);
    }
}

void ATranspersonalAudioManager::SetConsciousnessState(EConsciousnessAudioState NewState, bool bForceImmediate)
{
    if (NewState == CurrentConsciousnessState)
    {
        return;
    }

    EConsciousnessAudioState OldState = CurrentConsciousnessState;

    if (bForceImmediate)
    {
        CurrentConsciousnessState = NewState;
        ApplyConsciousnessAudioState(NewState);
        OnConsciousnessStateChanged.Broadcast(OldState, NewState);
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Consciousness state changed immediately from %d to %d"), (int32)OldState, (int32)NewState);
    }
    else
    {
        // Find transition
        FAudioStateTransition* Transition = FindStateTransition(CurrentConsciousnessState, NewState);
        if (Transition)
        {
            PendingConsciousnessState = NewState;
            bIsTransitioning = true;
            TransitionTimer = 0.0f;
            TransitionDuration = Transition->TransitionDuration;

            // Play transition sound if available
            if (Transition->TransitionSound && TransitionAudioComponent)
            {
                TransitionAudioComponent->SetSound(Transition->TransitionSound);
                TransitionAudioComponent->Play();
            }

            PlayConsciousnessTransition(CurrentConsciousnessState, NewState);
            UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Starting consciousness transition from %d to %d (Duration: %.2f)"), (int32)CurrentConsciousnessState, (int32)NewState, TransitionDuration);
        }
        else
        {
            // No transition found, apply immediately
            CurrentConsciousnessState = NewState;
            ApplyConsciousnessAudioState(NewState);
            OnConsciousnessStateChanged.Broadcast(OldState, NewState);
            UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: No transition found, consciousness state changed immediately from %d to %d"), (int32)OldState, (int32)NewState);
        }
    }
}

void ATranspersonalAudioManager::SetEnvironmentType(EEnvironmentAudioType NewEnvironment, bool bForceImmediate)
{
    if (NewEnvironment == CurrentEnvironmentType)
    {
        return;
    }

    EEnvironmentAudioType OldEnvironment = CurrentEnvironmentType;

    if (bForceImmediate)
    {
        CurrentEnvironmentType = NewEnvironment;
        ApplyEnvironmentAudioLayer(NewEnvironment);
        OnEnvironmentChanged.Broadcast(OldEnvironment, NewEnvironment);
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Environment changed immediately from %d to %d"), (int32)OldEnvironment, (int32)NewEnvironment);
    }
    else
    {
        PendingEnvironmentType = NewEnvironment;
        bIsTransitioning = true;
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Starting environment transition from %d to %d"), (int32)CurrentEnvironmentType, (int32)NewEnvironment);
    }
}

void ATranspersonalAudioManager::SetMusicIntensity(float NewIntensity, float TransitionTime)
{
    TargetMusicIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    if (TransitionTime > 0.0f)
    {
        MusicIntensityChangeRate = FMath::Abs(TargetMusicIntensity - CurrentMusicIntensity) / TransitionTime;
    }
    else
    {
        CurrentMusicIntensity = TargetMusicIntensity;
        UpdateAudioParameters();
    }

    UE_LOG(LogTemp, Log, TEXT("TranspersonalAudioManager: Music intensity target set to %.2f (Transition time: %.2f)"), TargetMusicIntensity, TransitionTime);
}

void ATranspersonalAudioManager::PlaySpatialSound(USoundCue* Sound, FVector Location, float Volume, float Pitch)
{
    if (!Sound || !bEnableSpatialAudio)
    {
        return;
    }

    // Calculate distance from player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        if (Distance <= SpatialAudioRange)
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Volume, Pitch);
            UE_LOG(LogTemp, Log, TEXT("TranspersonalAudioManager: Playing spatial sound at location (%.2f, %.2f, %.2f)"), Location.X, Location.Y, Location.Z);
        }
    }
}

void ATranspersonalAudioManager::PlayConsciousnessTransition(EConsciousnessAudioState FromState, EConsciousnessAudioState ToState)
{
    // This is where we would trigger specific transition effects
    // For now, we'll just log the transition
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Playing consciousness transition from %d to %d"), (int32)FromState, (int32)ToState);
    
    // In a full implementation, this would trigger:
    // - Specific transition sound effects
    // - MetaSound parameter changes
    // - Audio reverb zone transitions
    // - Binaural beat adjustments for consciousness states
}

void ATranspersonalAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioParameters();
}

void ATranspersonalAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioParameters();
}

void ATranspersonalAudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioParameters();
}

void ATranspersonalAudioManager::FadeOutAllAudio(float FadeTime)
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->FadeOut(FadeTime, 0.0f);
    }
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeOut(FadeTime, 0.0f);
    }
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Fading out all audio over %.2f seconds"), FadeTime);
}

void ATranspersonalAudioManager::FadeInAllAudio(float FadeTime)
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->FadeIn(FadeTime, MusicVolume * MasterVolume);
    }
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeIn(FadeTime, AmbientVolume * MasterVolume);
    }
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Fading in all audio over %.2f seconds"), FadeTime);
}

void ATranspersonalAudioManager::UpdateAdaptiveMusic(float DeltaTime)
{
    if (FMath::Abs(CurrentMusicIntensity - TargetMusicIntensity) > 0.01f)
    {
        float Direction = (TargetMusicIntensity > CurrentMusicIntensity) ? 1.0f : -1.0f;
        CurrentMusicIntensity += Direction * MusicIntensityChangeRate * DeltaTime;
        CurrentMusicIntensity = FMath::Clamp(CurrentMusicIntensity, 0.0f, 1.0f);

        // Clamp to target if we've reached it
        if ((Direction > 0.0f && CurrentMusicIntensity >= TargetMusicIntensity) ||
            (Direction < 0.0f && CurrentMusicIntensity <= TargetMusicIntensity))
        {
            CurrentMusicIntensity = TargetMusicIntensity;
        }

        UpdateAudioParameters();
    }
}

void ATranspersonalAudioManager::ProcessConsciousnessTransition(float DeltaTime)
{
    if (bIsTransitioning && TransitionDuration > 0.0f)
    {
        TransitionTimer += DeltaTime;
        float TransitionProgress = TransitionTimer / TransitionDuration;

        if (TransitionProgress >= 1.0f)
        {
            // Transition complete
            EConsciousnessAudioState OldState = CurrentConsciousnessState;
            CurrentConsciousnessState = PendingConsciousnessState;
            ApplyConsciousnessAudioState(CurrentConsciousnessState);
            OnConsciousnessStateChanged.Broadcast(OldState, CurrentConsciousnessState);
            
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Consciousness transition completed to state %d"), (int32)CurrentConsciousnessState);
        }
    }
}

void ATranspersonalAudioManager::ProcessEnvironmentTransition(float DeltaTime)
{
    // Simple environment transition logic
    if (PendingEnvironmentType != CurrentEnvironmentType)
    {
        EEnvironmentAudioType OldEnvironment = CurrentEnvironmentType;
        CurrentEnvironmentType = PendingEnvironmentType;
        ApplyEnvironmentAudioLayer(CurrentEnvironmentType);
        OnEnvironmentChanged.Broadcast(OldEnvironment, CurrentEnvironmentType);
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Environment transition completed to %d"), (int32)CurrentEnvironmentType);
    }
}

void ATranspersonalAudioManager::UpdateAudioParameters()
{
    if (MusicAudioComponent)
    {
        float FinalMusicVolume = MusicVolume * MasterVolume * CurrentMusicIntensity;
        MusicAudioComponent->SetVolumeMultiplier(FinalMusicVolume);
    }

    if (AmbientAudioComponent)
    {
        float FinalAmbientVolume = AmbientVolume * MasterVolume;
        AmbientAudioComponent->SetVolumeMultiplier(FinalAmbientVolume);
    }
}

FAudioStateTransition* ATranspersonalAudioManager::FindStateTransition(EConsciousnessAudioState FromState, EConsciousnessAudioState ToState)
{
    for (FAudioStateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == FromState && Transition.ToState == ToState)
        {
            return &Transition;
        }
    }
    return nullptr;
}

FEnvironmentAudioLayer* ATranspersonalAudioManager::FindEnvironmentLayer(EEnvironmentAudioType EnvironmentType)
{
    for (FEnvironmentAudioLayer& Layer : EnvironmentLayers)
    {
        if (Layer.EnvironmentType == EnvironmentType)
        {
            return &Layer;
        }
    }
    return nullptr;
}

void ATranspersonalAudioManager::ApplyConsciousnessAudioState(EConsciousnessAudioState State)
{
    // Find and apply the music for this consciousness state
    if (UMetaSoundSource** FoundMusic = ConsciousnessMusic.Find(State))
    {
        if (*FoundMusic && MusicAudioComponent)
        {
            MusicAudioComponent->SetSound(*FoundMusic);
            if (!MusicAudioComponent->IsPlaying())
            {
                MusicAudioComponent->Play();
            }
            UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Applied consciousness music for state %d"), (int32)State);
        }
    }

    // Adjust music intensity based on consciousness state
    switch (State)
    {
        case EConsciousnessAudioState::Dormant:
            SetMusicIntensity(0.3f, 2.0f);
            break;
        case EConsciousnessAudioState::Awakening:
            SetMusicIntensity(0.5f, 3.0f);
            break;
        case EConsciousnessAudioState::Aware:
            SetMusicIntensity(0.7f, 2.0f);
            break;
        case EConsciousnessAudioState::Transcendent:
            SetMusicIntensity(0.9f, 4.0f);
            break;
        case EConsciousnessAudioState::Unity:
            SetMusicIntensity(1.0f, 5.0f);
            break;
    }
}

void ATranspersonalAudioManager::ApplyEnvironmentAudioLayer(EEnvironmentAudioType EnvironmentType)
{
    FEnvironmentAudioLayer* Layer = FindEnvironmentLayer(EnvironmentType);
    if (Layer)
    {
        if (Layer->AmbientSound && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(Layer->AmbientSound);
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->FadeIn(Layer->FadeInTime, Layer->BaseVolume * AmbientVolume * MasterVolume);
            }
            UE_LOG(LogTemp, Warning, TEXT("TranspersonalAudioManager: Applied environment ambient for type %d"), (int32)EnvironmentType);
        }

        if (Layer->AdaptiveMusic && MusicAudioComponent)
        {
            // If we have adaptive music for this environment, we can blend it with consciousness music
            UE_LOG(LogTemp, Log, TEXT("TranspersonalAudioManager: Environment has adaptive music for type %d"), (int32)EnvironmentType);
        }
    }
}