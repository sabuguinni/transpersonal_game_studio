#include "Audio_CampfireSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UAudio_CampfireSystem::UAudio_CampfireSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms

    // Initialize state
    CurrentState = EAudio_CampfireState::Unlit;
    bIsStoryPlaying = false;
    bPlayerInRange = false;
    CurrentStoryDuration = 0.0f;
    StoryPlaybackTime = 0.0f;
    
    // Initialize fade variables
    FadeTimer = 0.0f;
    FadeTargetVolume = 0.0f;
    FadeStartVolume = 0.0f;
    FadeDuration = 0.0f;
    bIsFading = false;

    // Initialize audio components as null - will be created in BeginPlay
    CrackleAudioComponent = nullptr;
    StoryAudioComponent = nullptr;
    AmbientAudioComponent = nullptr;
    
    // Initialize audio assets as null
    CampfireCrackleCue = nullptr;
    CampfireAmbientCue = nullptr;
}

void UAudio_CampfireSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeAudioComponents();
}

void UAudio_CampfireSystem::InitializeAudioComponents()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_CampfireSystem: No owner found"));
        return;
    }

    // Create Crackle Audio Component
    CrackleAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (CrackleAudioComponent)
    {
        CrackleAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        CrackleAudioComponent->SetVolumeMultiplier(AudioSettings.CrackleVolume);
        CrackleAudioComponent->bAutoActivate = false;
        CrackleAudioComponent->RegisterComponent();
    }

    // Create Story Audio Component
    StoryAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (StoryAudioComponent)
    {
        StoryAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        StoryAudioComponent->SetVolumeMultiplier(AudioSettings.StorytellingVolume);
        StoryAudioComponent->bAutoActivate = false;
        StoryAudioComponent->RegisterComponent();
    }

    // Create Ambient Audio Component
    AmbientAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        AmbientAudioComponent->SetVolumeMultiplier(0.3f);
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->RegisterComponent();
    }

    UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: Audio components initialized"));
}

void UAudio_CampfireSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Check player proximity
    CheckPlayerProximity();

    // Update story playback
    if (bIsStoryPlaying)
    {
        UpdateStoryPlayback(DeltaTime);
    }

    // Handle volume fading
    if (bIsFading && CrackleAudioComponent)
    {
        FadeTimer += DeltaTime;
        float FadeProgress = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);
        float CurrentVolume = FMath::Lerp(FadeStartVolume, FadeTargetVolume, FadeProgress);
        
        CrackleAudioComponent->SetVolumeMultiplier(CurrentVolume);
        
        if (FadeProgress >= 1.0f)
        {
            bIsFading = false;
        }
    }

    // Update audio based on current state
    UpdateAudioBasedOnState();
}

void UAudio_CampfireSystem::SetCampfireState(EAudio_CampfireState NewState)
{
    if (CurrentState != NewState)
    {
        EAudio_CampfireState OldState = CurrentState;
        CurrentState = NewState;
        HandleStateTransition(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: State changed from %d to %d"), 
            (int32)OldState, (int32)NewState);
    }
}

void UAudio_CampfireSystem::HandleStateTransition(EAudio_CampfireState OldState, EAudio_CampfireState NewState)
{
    switch (NewState)
    {
        case EAudio_CampfireState::Unlit:
            if (CrackleAudioComponent && CrackleAudioComponent->IsPlaying())
            {
                FadeCrackleAudio(0.0f, AudioSettings.FadeOutDuration);
            }
            break;

        case EAudio_CampfireState::Crackling:
            if (CrackleAudioComponent && CampfireCrackleCue)
            {
                if (!CrackleAudioComponent->IsPlaying())
                {
                    CrackleAudioComponent->SetSound(CampfireCrackleCue);
                    CrackleAudioComponent->Play();
                }
                FadeCrackleAudio(AudioSettings.CrackleVolume, AudioSettings.FadeInDuration);
            }
            break;

        case EAudio_CampfireState::Storytelling:
            if (CrackleAudioComponent)
            {
                FadeCrackleAudio(AudioSettings.CrackleVolume * 0.3f, 1.0f); // Reduce crackle during stories
            }
            break;

        case EAudio_CampfireState::Dying:
            if (CrackleAudioComponent)
            {
                FadeCrackleAudio(AudioSettings.CrackleVolume * 0.1f, AudioSettings.FadeOutDuration);
            }
            break;
    }
}

void UAudio_CampfireSystem::PlayStoryAudio(const FString& AudioURL, float Duration)
{
    if (!StoryAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_CampfireSystem: No story audio component"));
        return;
    }

    // Stop any currently playing story
    if (bIsStoryPlaying)
    {
        StopStoryAudio();
    }

    // For now, log the audio URL since we can't directly load from URL in runtime
    // In a full implementation, this would load the audio asset
    UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: Playing story audio URL: %s"), *AudioURL);

    CurrentStoryDuration = Duration;
    StoryPlaybackTime = 0.0f;
    bIsStoryPlaying = true;

    // Set campfire to storytelling state
    SetCampfireState(EAudio_CampfireState::Storytelling);
}

void UAudio_CampfireSystem::StopStoryAudio()
{
    if (StoryAudioComponent && StoryAudioComponent->IsPlaying())
    {
        StoryAudioComponent->Stop();
    }

    bIsStoryPlaying = false;
    StoryPlaybackTime = 0.0f;
    CurrentStoryDuration = 0.0f;

    // Return to crackling state
    SetCampfireState(EAudio_CampfireState::Crackling);

    UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: Story audio stopped"));
}

void UAudio_CampfireSystem::UpdateStoryPlayback(float DeltaTime)
{
    if (!bIsStoryPlaying)
    {
        return;
    }

    StoryPlaybackTime += DeltaTime;

    // Check if story has finished
    if (StoryPlaybackTime >= CurrentStoryDuration)
    {
        StopStoryAudio();
    }
}

void UAudio_CampfireSystem::SetCrackleVolume(float Volume)
{
    AudioSettings.CrackleVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (CrackleAudioComponent)
    {
        CrackleAudioComponent->SetVolumeMultiplier(AudioSettings.CrackleVolume);
    }
}

void UAudio_CampfireSystem::SetStoryVolume(float Volume)
{
    AudioSettings.StorytellingVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (StoryAudioComponent)
    {
        StoryAudioComponent->SetVolumeMultiplier(AudioSettings.StorytellingVolume);
    }
}

void UAudio_CampfireSystem::CheckPlayerProximity()
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        bPlayerInRange = false;
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector CampfireLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, CampfireLocation);

    bool WasInRange = bPlayerInRange;
    bPlayerInRange = Distance <= AudioSettings.StorytellingRange;

    // Log proximity changes
    if (WasInRange != bPlayerInRange)
    {
        UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: Player proximity changed - In range: %s"), 
            bPlayerInRange ? TEXT("true") : TEXT("false"));
    }
}

void UAudio_CampfireSystem::FadeCrackleAudio(float TargetVolume, float Duration)
{
    if (!CrackleAudioComponent)
    {
        return;
    }

    FadeStartVolume = CrackleAudioComponent->VolumeMultiplier;
    FadeTargetVolume = TargetVolume;
    FadeDuration = Duration;
    FadeTimer = 0.0f;
    bIsFading = true;

    UE_LOG(LogTemp, Log, TEXT("UAudio_CampfireSystem: Fading crackle from %f to %f over %f seconds"), 
        FadeStartVolume, FadeTargetVolume, FadeDuration);
}

void UAudio_CampfireSystem::UpdateAudioBasedOnState()
{
    // Additional state-based audio logic can be added here
    // For example, adjusting ambient sounds based on time of day or weather
}