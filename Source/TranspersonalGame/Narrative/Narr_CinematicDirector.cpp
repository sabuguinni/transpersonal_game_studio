#include "Narr_CinematicDirector.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANarr_CinematicDirector::ANarr_CinematicDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create cinematic camera
    CinematicCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CinematicCamera"));
    CinematicCamera->SetupAttachment(RootComponent);
    CinematicCamera->SetFieldOfView(75.0f);

    // Create audio component for voicelines
    VoicelineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VoicelineAudio"));
    VoicelineAudio->SetupAttachment(RootComponent);
    VoicelineAudio->bAutoActivate = false;

    // Initialize default values
    CurrentCinematicType = ENarr_CinematicType::GameIntro;
    bAutoPlayOnBeginPlay = false;
    FadeInDuration = 2.0f;
    FadeOutDuration = 1.5f;
    bIsPlayingCinematic = false;
    CurrentSequenceIndex = 0;
    CurrentSequenceTime = 0.0f;

    // Setup default cinematic sequences
    SetupGameIntroSequence();
}

void ANarr_CinematicDirector::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoPlayOnBeginPlay)
    {
        PlayCinematic(CurrentCinematicType);
    }
}

void ANarr_CinematicDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPlayingCinematic)
    {
        UpdateCinematicSequence(DeltaTime);
    }
}

void ANarr_CinematicDirector::PlayCinematic(ENarr_CinematicType CinematicType)
{
    if (bIsPlayingCinematic)
    {
        StopCinematic();
    }

    CurrentCinematicType = CinematicType;
    
    // Setup sequences based on type
    switch (CinematicType)
    {
        case ENarr_CinematicType::GameIntro:
            SetupGameIntroSequence();
            break;
        case ENarr_CinematicType::ThreatWarning:
            SetupThreatWarningSequence();
            break;
        case ENarr_CinematicType::Discovery:
            SetupDiscoverySequence();
            break;
        default:
            SetupGameIntroSequence();
            break;
    }

    if (CinematicSequences.Num() > 0)
    {
        bIsPlayingCinematic = true;
        CurrentSequenceIndex = 0;
        CurrentSequenceTime = 0.0f;

        // Take control of player camera
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            PC->SetViewTargetWithBlend(this, FadeInDuration);
        }

        UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Started %s cinematic with %d sequences"), 
               *UEnum::GetValueAsString(CinematicType), CinematicSequences.Num());
    }
}

void ANarr_CinematicDirector::StopCinematic()
{
    if (!bIsPlayingCinematic)
        return;

    bIsPlayingCinematic = false;
    CurrentSequenceIndex = 0;
    CurrentSequenceTime = 0.0f;

    // Stop any playing voiceline
    if (VoicelineAudio && VoicelineAudio->IsPlaying())
    {
        VoicelineAudio->Stop();
    }

    // Return camera control to player
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        PC->SetViewTargetWithBlend(PC->GetPawn(), FadeOutDuration);
    }

    UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Stopped cinematic"));
}

void ANarr_CinematicDirector::SkipToNextSequence()
{
    if (!bIsPlayingCinematic || CurrentSequenceIndex >= CinematicSequences.Num() - 1)
    {
        EndCinematic();
        return;
    }

    StartNextSequence();
}

void ANarr_CinematicDirector::SetupGameIntroSequence()
{
    CinematicSequences.Empty();

    // Sequence 1: Wide establishing shot
    FNarr_CinematicSequence Seq1;
    Seq1.SequenceName = TEXT("Establishing Shot");
    Seq1.Duration = 6.0f;
    Seq1.CameraStartLocation = FVector(2000, 0, 800);
    Seq1.CameraEndLocation = FVector(1500, -500, 600);
    Seq1.CameraStartRotation = FRotator(-20, -45, 0);
    Seq1.CameraEndRotation = FRotator(-15, -60, 0);
    Seq1.VoicelineText = TEXT("The massive predator's eyes lock onto yours through the thick undergrowth...");
    Seq1.VoicelineAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777703415799_ThreatNarrator.mp3");
    CinematicSequences.Add(Seq1);

    // Sequence 2: Close-up on player area
    FNarr_CinematicSequence Seq2;
    Seq2.SequenceName = TEXT("Player Focus");
    Seq2.Duration = 4.0f;
    Seq2.CameraStartLocation = FVector(500, 200, 300);
    Seq2.CameraEndLocation = FVector(200, 100, 200);
    Seq2.CameraStartRotation = FRotator(-10, 180, 0);
    Seq2.CameraEndRotation = FRotator(-5, 160, 0);
    Seq2.VoicelineText = TEXT("I've found fresh water upstream, but the tracks around it... they're enormous.");
    Seq2.VoicelineAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777703421494_ScoutCharacter.mp3");
    CinematicSequences.Add(Seq2);

    UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Setup Game Intro sequence with %d shots"), CinematicSequences.Num());
}

void ANarr_CinematicDirector::SetupThreatWarningSequence()
{
    CinematicSequences.Empty();

    FNarr_CinematicSequence ThreatSeq;
    ThreatSeq.SequenceName = TEXT("Threat Warning");
    ThreatSeq.Duration = 5.0f;
    ThreatSeq.CameraStartLocation = FVector(800, 0, 400);
    ThreatSeq.CameraEndLocation = FVector(600, -200, 300);
    ThreatSeq.CameraStartRotation = FRotator(-25, 0, 0);
    ThreatSeq.CameraEndRotation = FRotator(-20, -30, 0);
    ThreatSeq.VoicelineText = TEXT("The pack is circling us. I count at least four, maybe five raptors.");
    ThreatSeq.VoicelineAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777703426371_WarriorSurvivor.mp3");
    CinematicSequences.Add(ThreatSeq);
}

void ANarr_CinematicDirector::SetupDiscoverySequence()
{
    CinematicSequences.Empty();

    FNarr_CinematicSequence DiscoverySeq;
    DiscoverySeq.SequenceName = TEXT("Discovery");
    DiscoverySeq.Duration = 6.0f;
    DiscoverySeq.CameraStartLocation = FVector(1000, 500, 500);
    DiscoverySeq.CameraEndLocation = FVector(700, 300, 350);
    DiscoverySeq.CameraStartRotation = FRotator(-30, -45, 0);
    DiscoverySeq.CameraEndRotation = FRotator(-15, -30, 0);
    DiscoverySeq.VoicelineText = TEXT("Listen carefully. The herbivores have gone silent. When the grazers stop feeding...");
    DiscoverySeq.VoicelineAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777703433676_ExperiencedTracker.mp3");
    CinematicSequences.Add(DiscoverySeq);
}

float ANarr_CinematicDirector::GetCurrentSequenceProgress() const
{
    if (!bIsPlayingCinematic || CurrentSequenceIndex >= CinematicSequences.Num())
        return 0.0f;

    const FNarr_CinematicSequence& CurrentSeq = CinematicSequences[CurrentSequenceIndex];
    return FMath::Clamp(CurrentSequenceTime / CurrentSeq.Duration, 0.0f, 1.0f);
}

void ANarr_CinematicDirector::UpdateCinematicSequence(float DeltaTime)
{
    if (CurrentSequenceIndex >= CinematicSequences.Num())
    {
        EndCinematic();
        return;
    }

    const FNarr_CinematicSequence& CurrentSeq = CinematicSequences[CurrentSequenceIndex];
    CurrentSequenceTime += DeltaTime;

    // Update camera position and rotation
    float Alpha = GetCurrentSequenceProgress();
    FVector NewLocation = LerpCameraPosition(CurrentSeq, Alpha);
    FRotator NewRotation = LerpCameraRotation(CurrentSeq, Alpha);

    CinematicCamera->SetWorldLocation(NewLocation);
    CinematicCamera->SetWorldRotation(NewRotation);

    // Play voiceline at start of sequence
    if (CurrentSequenceTime <= DeltaTime && !CurrentSeq.VoicelineAudioURL.IsEmpty())
    {
        PlayVoiceline(CurrentSeq.VoicelineAudioURL);
    }

    // Check if sequence is complete
    if (CurrentSequenceTime >= CurrentSeq.Duration)
    {
        if (CurrentSequenceIndex < CinematicSequences.Num() - 1)
        {
            StartNextSequence();
        }
        else
        {
            EndCinematic();
        }
    }
}

void ANarr_CinematicDirector::StartNextSequence()
{
    CurrentSequenceIndex++;
    CurrentSequenceTime = 0.0f;

    if (CurrentSequenceIndex < CinematicSequences.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Starting sequence %d: %s"), 
               CurrentSequenceIndex, *CinematicSequences[CurrentSequenceIndex].SequenceName);
    }
}

void ANarr_CinematicDirector::EndCinematic()
{
    UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Cinematic completed"));
    StopCinematic();
}

FVector ANarr_CinematicDirector::LerpCameraPosition(const FNarr_CinematicSequence& Sequence, float Alpha) const
{
    return FMath::Lerp(Sequence.CameraStartLocation, Sequence.CameraEndLocation, Alpha);
}

FRotator ANarr_CinematicDirector::LerpCameraRotation(const FNarr_CinematicSequence& Sequence, float Alpha) const
{
    return FMath::Lerp(Sequence.CameraStartRotation, Sequence.CameraEndRotation, Alpha);
}

void ANarr_CinematicDirector::PlayVoiceline(const FString& VoicelineURL)
{
    if (VoicelineAudio && !VoicelineURL.IsEmpty())
    {
        // Note: In a real implementation, you would load the audio from URL
        // For now, we'll just log that the voiceline should play
        UE_LOG(LogTemp, Warning, TEXT("CinematicDirector: Playing voiceline from %s"), *VoicelineURL);
        
        // You would implement URL-based audio loading here
        // For demonstration, we'll just mark that audio should play
        VoicelineAudio->Play();
    }
}