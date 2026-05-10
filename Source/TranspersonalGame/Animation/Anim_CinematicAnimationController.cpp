#include "Anim_CinematicAnimationController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Curves/CurveFloat.h"

UAnim_CinematicAnimationController::UAnim_CinematicAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize state
    CurrentState = EAnim_CinematicState::Idle;
    CurrentSequenceName = NAME_None;
    CurrentPlayTime = 0.0f;
    CurrentMontage = nullptr;
    bUseCinematicCamera = true;

    // Initialize component references
    SkeletalMeshComponent = nullptr;
    PlayerController = nullptr;
    CinematicCamera = nullptr;

    // Create timeline component for camera transitions
    CameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTimeline"));

    // Initialize default camera settings
    DefaultCameraSettings.CameraLocation = FVector(0, -400, 200);
    DefaultCameraSettings.CameraRotation = FRotator(-10, 0, 0);
    DefaultCameraSettings.FieldOfView = 90.0f;
    DefaultCameraSettings.TransitionTime = 2.0f;
    DefaultCameraSettings.bSmoothTransition = true;

    // Initialize default cinematic sequences
    FAnim_CinematicSequence DiscoverySequence;
    DiscoverySequence.SequenceName = FName("Discovery");
    DiscoverySequence.CinematicType = EAnim_CinematicType::Discovery;
    DiscoverySequence.Duration = 3.0f;
    DiscoverySequence.BlendInTime = 0.5f;
    DiscoverySequence.BlendOutTime = 0.5f;
    DiscoverySequence.bLooping = false;
    DiscoverySequence.bInterruptible = true;
    DiscoverySequence.Priority = 1;
    CinematicSequences.Add(DiscoverySequence.SequenceName, DiscoverySequence);

    FAnim_CinematicSequence DinosaurEncounterSequence;
    DinosaurEncounterSequence.SequenceName = FName("DinosaurEncounter");
    DinosaurEncounterSequence.CinematicType = EAnim_CinematicType::DinosaurEncounter;
    DinosaurEncounterSequence.Duration = 5.0f;
    DinosaurEncounterSequence.BlendInTime = 0.3f;
    DinosaurEncounterSequence.BlendOutTime = 0.3f;
    DinosaurEncounterSequence.bLooping = false;
    DinosaurEncounterSequence.bInterruptible = false;
    DinosaurEncounterSequence.Priority = 3;
    CinematicSequences.Add(DinosaurEncounterSequence.SequenceName, DinosaurEncounterSequence);

    FAnim_CinematicSequence SurvivalActionSequence;
    SurvivalActionSequence.SequenceName = FName("SurvivalAction");
    SurvivalActionSequence.CinematicType = EAnim_CinematicType::SurvivalAction;
    SurvivalActionSequence.Duration = 2.5f;
    SurvivalActionSequence.BlendInTime = 0.2f;
    SurvivalActionSequence.BlendOutTime = 0.2f;
    SurvivalActionSequence.bLooping = false;
    SurvivalActionSequence.bInterruptible = true;
    SurvivalActionSequence.Priority = 2;
    CinematicSequences.Add(SurvivalActionSequence.SequenceName, SurvivalActionSequence);
}

void UAnim_CinematicAnimationController::BeginPlay()
{
    Super::BeginPlay();

    // Get component references
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: No SkeletalMeshComponent found on owner"));
        }
    }

    // Get player controller
    UWorld* World = GetWorld();
    if (World)
    {
        PlayerController = World->GetFirstPlayerController();
        if (!PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: No PlayerController found"));
        }
    }

    // Validate sequence data
    ValidateSequenceData();

    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController initialized with %d sequences"), CinematicSequences.Num());
}

void UAnim_CinematicAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == EAnim_CinematicState::Playing)
    {
        UpdateCinematicPlayback(DeltaTime);
    }
}

bool UAnim_CinematicAnimationController::PlayCinematicSequence(FName SequenceName, bool bForcePlay)
{
    if (!CinematicSequences.Contains(SequenceName))
    {
        UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: Sequence '%s' not found"), *SequenceName.ToString());
        return false;
    }

    const FAnim_CinematicSequence& Sequence = CinematicSequences[SequenceName];

    // Check if we can interrupt current sequence
    if (CurrentState == EAnim_CinematicState::Playing && !bForcePlay)
    {
        if (CinematicSequences.Contains(CurrentSequenceName))
        {
            const FAnim_CinematicSequence& CurrentSequence = CinematicSequences[CurrentSequenceName];
            if (!CurrentSequence.bInterruptible && Sequence.Priority <= CurrentSequence.Priority)
            {
                UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: Cannot interrupt current sequence '%s'"), *CurrentSequenceName.ToString());
                return false;
            }
        }
    }

    // Stop current sequence if playing
    if (CurrentState == EAnim_CinematicState::Playing)
    {
        StopCurrentCinematic(true);
    }

    // Set up new sequence
    CurrentSequenceName = SequenceName;
    CurrentPlayTime = 0.0f;
    SetCinematicState(EAnim_CinematicState::Preparing);

    // Load animation montage if available
    if (Sequence.AnimationMontage.IsValid())
    {
        CurrentMontage = Sequence.AnimationMontage.LoadSynchronous();
        if (CurrentMontage && SkeletalMeshComponent)
        {
            UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            if (AnimInstance)
            {
                // Play montage with blend settings
                float PlayRate = 1.0f;
                AnimInstance->Montage_Play(CurrentMontage, PlayRate, EMontagePlayReturnType::MontageLength, 0.0f);
                AnimInstance->Montage_SetBlendingOutDelegate(FOnMontageBlendingOutStarted::CreateUObject(this, &UAnim_CinematicAnimationController::OnCinematicMontageEnded));
            }
        }
    }

    // Set up cinematic camera if enabled
    if (bUseCinematicCamera)
    {
        SetCinematicCamera(DefaultCameraSettings);
    }

    // Start playback
    SetCinematicState(EAnim_CinematicState::Playing);

    // Set timer for sequence completion
    if (Sequence.Duration > 0.0f && !Sequence.bLooping)
    {
        GetWorld()->GetTimerManager().SetTimer(CinematicTimerHandle, [this]()
        {
            OnCinematicCompleted.Broadcast(CurrentSequenceName);
            StopCurrentCinematic(true);
        }, Sequence.Duration, false);
    }

    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Playing sequence '%s'"), *SequenceName.ToString());
    return true;
}

void UAnim_CinematicAnimationController::StopCurrentCinematic(bool bBlendOut)
{
    if (CurrentState == EAnim_CinematicState::Idle)
    {
        return;
    }

    // Clear timers
    if (CinematicTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(CinematicTimerHandle);
    }

    // Stop animation montage
    if (CurrentMontage && SkeletalMeshComponent)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            if (bBlendOut)
            {
                const FAnim_CinematicSequence* CurrentSequence = CinematicSequences.Find(CurrentSequenceName);
                float BlendOutTime = CurrentSequence ? CurrentSequence->BlendOutTime : 0.25f;
                AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
            }
            else
            {
                AnimInstance->Montage_Stop(0.0f, CurrentMontage);
            }
        }
    }

    // Restore player camera
    if (bUseCinematicCamera)
    {
        RestorePlayerCamera();
    }

    // Reset state
    FName CompletedSequence = CurrentSequenceName;
    CurrentSequenceName = NAME_None;
    CurrentMontage = nullptr;
    CurrentPlayTime = 0.0f;
    SetCinematicState(EAnim_CinematicState::Idle);

    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Stopped sequence '%s'"), *CompletedSequence.ToString());
}

void UAnim_CinematicAnimationController::PauseCinematic()
{
    if (CurrentState != EAnim_CinematicState::Playing)
    {
        return;
    }

    if (CurrentMontage && SkeletalMeshComponent)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Pause(CurrentMontage);
        }
    }

    SetCinematicState(EAnim_CinematicState::Blending);
    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Paused sequence '%s'"), *CurrentSequenceName.ToString());
}

void UAnim_CinematicAnimationController::ResumeCinematic()
{
    if (CurrentState != EAnim_CinematicState::Blending)
    {
        return;
    }

    if (CurrentMontage && SkeletalMeshComponent)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Resume(CurrentMontage);
        }
    }

    SetCinematicState(EAnim_CinematicState::Playing);
    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Resumed sequence '%s'"), *CurrentSequenceName.ToString());
}

bool UAnim_CinematicAnimationController::RegisterCinematicSequence(const FAnim_CinematicSequence& NewSequence)
{
    if (NewSequence.SequenceName == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: Cannot register sequence with empty name"));
        return false;
    }

    CinematicSequences.Add(NewSequence.SequenceName, NewSequence);
    UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Registered sequence '%s'"), *NewSequence.SequenceName.ToString());
    return true;
}

void UAnim_CinematicAnimationController::UnregisterCinematicSequence(FName SequenceName)
{
    if (CinematicSequences.Contains(SequenceName))
    {
        // Stop sequence if currently playing
        if (CurrentSequenceName == SequenceName)
        {
            StopCurrentCinematic(true);
        }

        CinematicSequences.Remove(SequenceName);
        UE_LOG(LogTemp, Log, TEXT("CinematicAnimationController: Unregistered sequence '%s'"), *SequenceName.ToString());
    }
}

void UAnim_CinematicAnimationController::SetCinematicCamera(const FAnim_CinematicCamera& CameraSettings)
{
    if (!PlayerController)
    {
        return;
    }

    // Store original camera settings if this is the first time
    if (CurrentState == EAnim_CinematicState::Preparing)
    {
        APawn* PlayerPawn = PlayerController->GetPawn();
        if (PlayerPawn)
        {
            OriginalCameraSettings.CameraLocation = PlayerPawn->GetActorLocation();
            OriginalCameraSettings.CameraRotation = PlayerController->GetControlRotation();
            OriginalCameraSettings.FieldOfView = 90.0f; // Default FOV
        }
    }

    // Apply camera transition
    BlendToCinematicCamera(CameraSettings);
}

void UAnim_CinematicAnimationController::RestorePlayerCamera()
{
    if (!PlayerController)
    {
        return;
    }

    // Restore original camera settings
    BlendToCinematicCamera(OriginalCameraSettings);
}

float UAnim_CinematicAnimationController::GetCurrentSequenceDuration() const
{
    if (CinematicSequences.Contains(CurrentSequenceName))
    {
        return CinematicSequences[CurrentSequenceName].Duration;
    }
    return 0.0f;
}

TArray<FName> UAnim_CinematicAnimationController::GetAvailableSequences() const
{
    TArray<FName> SequenceNames;
    CinematicSequences.GetKeys(SequenceNames);
    return SequenceNames;
}

FAnim_CinematicSequence UAnim_CinematicAnimationController::GetSequenceData(FName SequenceName) const
{
    if (CinematicSequences.Contains(SequenceName))
    {
        return CinematicSequences[SequenceName];
    }
    return FAnim_CinematicSequence();
}

void UAnim_CinematicAnimationController::UpdateCinematicPlayback(float DeltaTime)
{
    CurrentPlayTime += DeltaTime;

    // Check if sequence should complete
    if (CinematicSequences.Contains(CurrentSequenceName))
    {
        const FAnim_CinematicSequence& CurrentSequence = CinematicSequences[CurrentSequenceName];
        if (!CurrentSequence.bLooping && CurrentPlayTime >= CurrentSequence.Duration)
        {
            OnCinematicCompleted.Broadcast(CurrentSequenceName);
            StopCurrentCinematic(true);
        }
    }
}

void UAnim_CinematicAnimationController::SetCinematicState(EAnim_CinematicState NewState)
{
    if (CurrentState != NewState)
    {
        EAnim_CinematicState OldState = CurrentState;
        CurrentState = NewState;
        OnCinematicStateChanged.Broadcast(OldState, NewState);
    }
}

void UAnim_CinematicAnimationController::OnCinematicMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        if (bInterrupted)
        {
            OnCinematicInterrupted.Broadcast(CurrentSequenceName);
        }
        else
        {
            OnCinematicCompleted.Broadcast(CurrentSequenceName);
        }
        
        StopCurrentCinematic(false);
    }
}

void UAnim_CinematicAnimationController::BlendToCinematicCamera(const FAnim_CinematicCamera& CameraSettings)
{
    if (!PlayerController)
    {
        return;
    }

    // Simple immediate camera transition for now
    // In a full implementation, this would use the timeline component for smooth transitions
    PlayerController->SetControlRotation(CameraSettings.CameraRotation);
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (PlayerPawn)
    {
        FVector TargetLocation = PlayerPawn->GetActorLocation() + CameraSettings.CameraLocation;
        PlayerPawn->SetActorLocation(TargetLocation);
    }
}

void UAnim_CinematicAnimationController::ValidateSequenceData()
{
    TArray<FName> InvalidSequences;
    
    for (const auto& SequencePair : CinematicSequences)
    {
        const FAnim_CinematicSequence& Sequence = SequencePair.Value;
        
        if (Sequence.Duration <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: Sequence '%s' has invalid duration"), *Sequence.SequenceName.ToString());
        }
        
        if (Sequence.BlendInTime < 0.0f || Sequence.BlendOutTime < 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("CinematicAnimationController: Sequence '%s' has invalid blend times"), *Sequence.SequenceName.ToString());
        }
    }
}