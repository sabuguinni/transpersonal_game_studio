#include "Audio_AdaptiveMusicManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAudio_AdaptiveMusicManager::AAudio_AdaptiveMusicManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio components for each music state
    CalmMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CalmMusicComponent"));
    CalmMusicComponent->SetupAttachment(RootComponent);
    CalmMusicComponent->bAutoActivate = false;

    TensionMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TensionMusicComponent"));
    TensionMusicComponent->SetupAttachment(RootComponent);
    TensionMusicComponent->bAutoActivate = false;

    DangerMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerMusicComponent"));
    DangerMusicComponent->SetupAttachment(RootComponent);
    DangerMusicComponent->bAutoActivate = false;

    CombatMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CombatMusicComponent"));
    CombatMusicComponent->SetupAttachment(RootComponent);
    CombatMusicComponent->bAutoActivate = false;

    // Initialize default values
    CurrentMusicState = EAudio_MusicState::Calm;
    TargetMusicState = EAudio_MusicState::Calm;
    TransitionDuration = 2.0f;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;

    // Volume settings
    MasterVolume = 0.7f;
    CalmVolume = 0.6f;
    TensionVolume = 0.7f;
    DangerVolume = 0.8f;
    CombatVolume = 1.0f;
}

void AAudio_AdaptiveMusicManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with calm music
    SetMusicState(EAudio_MusicState::Calm);
    
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: System initialized with Calm state"));
}

void AAudio_AdaptiveMusicManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTransitioning)
    {
        UpdateMusicTransition(DeltaTime);
    }
}

void AAudio_AdaptiveMusicManager::SetMusicState(EAudio_MusicState NewState)
{
    if (NewState == CurrentMusicState)
    {
        return;
    }

    // Stop current music
    UAudioComponent* CurrentComponent = GetCurrentMusicComponent();
    if (CurrentComponent && CurrentComponent->IsPlaying())
    {
        StopMusicComponent(CurrentComponent);
    }

    // Update state
    CurrentMusicState = NewState;
    TargetMusicState = NewState;
    bIsTransitioning = false;

    // Start new music
    UAudioComponent* NewComponent = GetCurrentMusicComponent();
    if (NewComponent)
    {
        StartMusicComponent(NewComponent);
        UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Switched to %s music"), 
               *UEnum::GetValueAsString(NewState));
    }
}

void AAudio_AdaptiveMusicManager::TransitionToMusicState(EAudio_MusicState NewState, float Duration)
{
    if (NewState == CurrentMusicState)
    {
        return;
    }

    TargetMusicState = NewState;
    TransitionDuration = FMath::Max(Duration, 0.1f);
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    // Start target music component if not already playing
    UAudioComponent* TargetComponent = GetMusicComponentForState(TargetMusicState);
    if (TargetComponent && !TargetComponent->IsPlaying())
    {
        TargetComponent->SetVolumeMultiplier(0.0f);
        StartMusicComponent(TargetComponent);
    }

    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Starting transition from %s to %s over %.1f seconds"),
           *UEnum::GetValueAsString(CurrentMusicState),
           *UEnum::GetValueAsString(TargetMusicState),
           Duration);
}

void AAudio_AdaptiveMusicManager::StopAllMusic()
{
    StopMusicComponent(CalmMusicComponent);
    StopMusicComponent(TensionMusicComponent);
    StopMusicComponent(DangerMusicComponent);
    StopMusicComponent(CombatMusicComponent);

    bIsTransitioning = false;
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: All music stopped"));
}

void AAudio_AdaptiveMusicManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    // Update all active components
    if (CalmMusicComponent->IsPlaying())
        SetComponentVolume(CalmMusicComponent, CalmVolume);
    if (TensionMusicComponent->IsPlaying())
        SetComponentVolume(TensionMusicComponent, TensionVolume);
    if (DangerMusicComponent->IsPlaying())
        SetComponentVolume(DangerMusicComponent, DangerVolume);
    if (CombatMusicComponent->IsPlaying())
        SetComponentVolume(CombatMusicComponent, CombatVolume);
}

UAudioComponent* AAudio_AdaptiveMusicManager::GetCurrentMusicComponent() const
{
    return GetMusicComponentForState(CurrentMusicState);
}

UAudioComponent* AAudio_AdaptiveMusicManager::GetMusicComponentForState(EAudio_MusicState State) const
{
    switch (State)
    {
        case EAudio_MusicState::Calm:
            return CalmMusicComponent;
        case EAudio_MusicState::Tension:
            return TensionMusicComponent;
        case EAudio_MusicState::Danger:
            return DangerMusicComponent;
        case EAudio_MusicState::Combat:
            return CombatMusicComponent;
        default:
            return CalmMusicComponent;
    }
}

void AAudio_AdaptiveMusicManager::UpdateMusicTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    UAudioComponent* CurrentComponent = GetCurrentMusicComponent();
    UAudioComponent* TargetComponent = GetMusicComponentForState(TargetMusicState);

    if (CurrentComponent && TargetComponent)
    {
        // Fade out current, fade in target
        float CurrentVolume = FMath::Lerp(1.0f, 0.0f, TransitionProgress);
        float TargetVolume = FMath::Lerp(0.0f, 1.0f, TransitionProgress);

        CurrentComponent->SetVolumeMultiplier(CurrentVolume * MasterVolume);
        TargetComponent->SetVolumeMultiplier(TargetVolume * MasterVolume);
    }

    // Check if transition is complete
    if (TransitionProgress >= 1.0f)
    {
        // Stop old music
        if (CurrentComponent)
        {
            StopMusicComponent(CurrentComponent);
        }

        // Update state
        CurrentMusicState = TargetMusicState;
        bIsTransitioning = false;

        UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Transition complete to %s"),
               *UEnum::GetValueAsString(CurrentMusicState));
    }
}

void AAudio_AdaptiveMusicManager::StartMusicComponent(UAudioComponent* Component)
{
    if (Component)
    {
        Component->Play();
        
        // Set appropriate volume based on state
        float StateVolume = CalmVolume;
        if (Component == TensionMusicComponent) StateVolume = TensionVolume;
        else if (Component == DangerMusicComponent) StateVolume = DangerVolume;
        else if (Component == CombatMusicComponent) StateVolume = CombatVolume;
        
        SetComponentVolume(Component, StateVolume);
    }
}

void AAudio_AdaptiveMusicManager::StopMusicComponent(UAudioComponent* Component)
{
    if (Component && Component->IsPlaying())
    {
        Component->Stop();
    }
}

void AAudio_AdaptiveMusicManager::SetComponentVolume(UAudioComponent* Component, float Volume)
{
    if (Component)
    {
        Component->SetVolumeMultiplier(Volume * MasterVolume);
    }
}