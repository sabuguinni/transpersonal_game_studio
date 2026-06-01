#include "NPC_DinosaurAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_DinosaurAnimationController::UNPC_DinosaurAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentAnimState = ENPC_DinosaurAnimState::Idle;
    PreviousAnimState = ENPC_DinosaurAnimState::Idle;
    StateTransitionTime = 0.0f;
    
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    AnimationSpeedMultiplier = 1.0f;
    StateTransitionDuration = 0.3f;
    
    CurrentSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsMoving = false;
    bIsInCombat = false;
    
    bIsPackLeader = false;
    PackSyncTimer = 0.0f;
    PackSyncUpdateInterval = 0.1f;
    
    LastAnimationUpdate = 0.0f;
}

void UNPC_DinosaurAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAnimationController();
    
    // Set initial animation state
    SetAnimationState(ENPC_DinosaurAnimState::Idle);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: Initialized for species %d"), (int32)DinosaurSpecies);
}

void UNPC_DinosaurAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update animation parameters
    UpdateAnimationParameters();
    
    // Update movement parameters
    UpdateMovementParameters();
    
    // Process pack synchronization if needed
    if (PackMembers.Num() > 0)
    {
        ProcessPackSynchronization(DeltaTime);
    }
    
    // Update state transition timer
    if (StateTransitionTime > 0.0f)
    {
        StateTransitionTime -= DeltaTime;
        if (StateTransitionTime <= 0.0f)
        {
            StateTransitionTime = 0.0f;
        }
    }
}

void UNPC_DinosaurAnimationController::InitializeAnimationController()
{
    // Get owner character and components
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter.IsValid())
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
    
    // Initialize species-specific settings
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            AnimationSpeedMultiplier = 0.8f; // Slower, more deliberate movements
            StateTransitionDuration = 0.5f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            AnimationSpeedMultiplier = 1.2f; // Faster, more agile
            StateTransitionDuration = 0.2f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            AnimationSpeedMultiplier = 0.7f; // Heavy, slow movements
            StateTransitionDuration = 0.6f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            AnimationSpeedMultiplier = 0.5f; // Very slow, massive movements
            StateTransitionDuration = 0.8f;
            break;
            
        default:
            AnimationSpeedMultiplier = 1.0f;
            StateTransitionDuration = 0.3f;
            break;
    }
}

void UNPC_DinosaurAnimationController::SetAnimationState(ENPC_DinosaurAnimState NewState)
{
    if (CurrentAnimState == NewState)
    {
        return;
    }
    
    ENPC_DinosaurAnimState OldState = CurrentAnimState;
    PreviousAnimState = CurrentAnimState;
    CurrentAnimState = NewState;
    
    HandleStateTransition(NewState);
    
    // Broadcast state change event
    OnAnimationStateChanged.Broadcast(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: State changed from %d to %d"), (int32)OldState, (int32)NewState);
}

void UNPC_DinosaurAnimationController::HandleStateTransition(ENPC_DinosaurAnimState NewState)
{
    StateTransitionTime = StateTransitionDuration;
    
    // Update animation instance parameters if available
    if (AnimInstance.IsValid())
    {
        // Set animation state enum parameter
        AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
    }
    
    // Handle specific state transitions
    switch (NewState)
    {
        case ENPC_DinosaurAnimState::Alert:
            // Increase animation speed for alert state
            SetAnimationSpeed(AnimationSpeedMultiplier * 1.2f);
            break;
            
        case ENPC_DinosaurAnimState::Hunting:
            // Set hunting-specific parameters
            SetAnimationSpeed(AnimationSpeedMultiplier * 1.1f);
            bIsInCombat = true;
            break;
            
        case ENPC_DinosaurAnimState::Feeding:
            // Slow down for feeding
            SetAnimationSpeed(AnimationSpeedMultiplier * 0.7f);
            bIsInCombat = false;
            break;
            
        case ENPC_DinosaurAnimState::Sleeping:
            // Very slow for sleeping
            SetAnimationSpeed(AnimationSpeedMultiplier * 0.3f);
            bIsInCombat = false;
            break;
            
        case ENPC_DinosaurAnimState::Territorial:
            // Aggressive display animations
            SetAnimationSpeed(AnimationSpeedMultiplier * 0.9f);
            break;
            
        default:
            SetAnimationSpeed(AnimationSpeedMultiplier);
            bIsInCombat = false;
            break;
    }
}

void UNPC_DinosaurAnimationController::PlaySpeciesAnimation(ENPC_DinosaurSpecies Species, ENPC_DinosaurAnimState AnimState)
{
    if (Species != DinosaurSpecies)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimationController: Trying to play animation for wrong species"));
        return;
    }
    
    // Find appropriate animation montage
    if (UAnimMontage** FoundMontage = SpeciesAnimations.Find(AnimState))
    {
        if (*FoundMontage)
        {
            PlayAnimationMontage(*FoundMontage);
        }
    }
}

void UNPC_DinosaurAnimationController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !AnimInstance.IsValid())
    {
        return;
    }
    
    // Play the montage
    AnimInstance->Montage_Play(Montage, PlayRate * AnimationSpeedMultiplier);
    
    // Bind to montage end event
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UNPC_DinosaurAnimationController::OnMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: Playing montage %s"), *Montage->GetName());
}

void UNPC_DinosaurAnimationController::StopAnimationMontage(UAnimMontage* Montage)
{
    if (!AnimInstance.IsValid())
    {
        return;
    }
    
    if (Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
    else
    {
        AnimInstance->StopAllMontages(0.2f);
    }
}

void UNPC_DinosaurAnimationController::TriggerTerritorialDisplay()
{
    SetAnimationState(ENPC_DinosaurAnimState::Territorial);
    
    // Play species-specific territorial animation
    PlaySpeciesAnimation(DinosaurSpecies, ENPC_DinosaurAnimState::Territorial);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: Triggered territorial display"));
}

void UNPC_DinosaurAnimationController::TriggerHuntingAnimation()
{
    SetAnimationState(ENPC_DinosaurAnimState::Hunting);
    PlaySpeciesAnimation(DinosaurSpecies, ENPC_DinosaurAnimState::Hunting);
}

void UNPC_DinosaurAnimationController::TriggerFeedingAnimation()
{
    SetAnimationState(ENPC_DinosaurAnimState::Feeding);
    PlaySpeciesAnimation(DinosaurSpecies, ENPC_DinosaurAnimState::Feeding);
}

void UNPC_DinosaurAnimationController::TriggerSleepingAnimation()
{
    SetAnimationState(ENPC_DinosaurAnimState::Sleeping);
    PlaySpeciesAnimation(DinosaurSpecies, ENPC_DinosaurAnimState::Sleeping);
}

void UNPC_DinosaurAnimationController::TriggerAlertAnimation()
{
    SetAnimationState(ENPC_DinosaurAnimState::Alert);
    PlaySpeciesAnimation(DinosaurSpecies, ENPC_DinosaurAnimState::Alert);
}

void UNPC_DinosaurAnimationController::SynchronizePackAnimation(const TArray<UNPC_DinosaurAnimationController*>& PackMembers)
{
    this->PackMembers.Empty();
    
    for (UNPC_DinosaurAnimationController* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            this->PackMembers.Add(Member);
        }
    }
    
    bIsPackLeader = true;
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: Pack synchronized with %d members"), this->PackMembers.Num());
}

void UNPC_DinosaurAnimationController::PlayPackCoordinatedAnimation(ENPC_DinosaurAnimState AnimState, float SyncDelay)
{
    // Play animation immediately for pack leader
    SetAnimationState(AnimState);
    
    // Schedule delayed animation for pack members
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDelegate;
        
        TimerDelegate.BindLambda([this, AnimState]()
        {
            for (auto& MemberPtr : PackMembers)
            {
                if (MemberPtr.IsValid())
                {
                    MemberPtr->SetAnimationState(AnimState);
                }
            }
        });
        
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, SyncDelay, false);
    }
}

void UNPC_DinosaurAnimationController::ReactToPlayerPresence(float PlayerDistance, bool bPlayerVisible)
{
    if (!bPlayerVisible)
    {
        return;
    }
    
    // React based on species and distance
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            if (PlayerDistance < 3000.0f)
            {
                TriggerAlertAnimation();
            }
            if (PlayerDistance < 1500.0f)
            {
                TriggerTerritorialDisplay();
            }
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            if (PlayerDistance < 2000.0f)
            {
                TriggerAlertAnimation();
                // Coordinate with pack if available
                if (bIsPackLeader)
                {
                    PlayPackCoordinatedAnimation(ENPC_DinosaurAnimState::Alert, 0.2f);
                }
            }
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            if (PlayerDistance < 1000.0f)
            {
                TriggerTerritorialDisplay();
            }
            break;
            
        default:
            if (PlayerDistance < 1500.0f)
            {
                TriggerAlertAnimation();
            }
            break;
    }
}

void UNPC_DinosaurAnimationController::ReactToWeatherChange(ENPC_WeatherType NewWeather)
{
    switch (NewWeather)
    {
        case ENPC_WeatherType::Storm:
            // Seek shelter behavior
            SetAnimationState(ENPC_DinosaurAnimState::Alert);
            break;
            
        case ENPC_WeatherType::Rain:
            // Slower movement in rain
            SetAnimationSpeed(AnimationSpeedMultiplier * 0.8f);
            break;
            
        case ENPC_WeatherType::Clear:
            // Normal behavior
            SetAnimationSpeed(AnimationSpeedMultiplier);
            break;
    }
}

void UNPC_DinosaurAnimationController::ReactToTimeOfDay(float TimeOfDay)
{
    // Time of day reactions (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    if (TimeOfDay >= 0.9f || TimeOfDay <= 0.1f) // Night time
    {
        // Most dinosaurs sleep at night
        if (CurrentAnimState != ENPC_DinosaurAnimState::Hunting)
        {
            TriggerSleepingAnimation();
        }
    }
    else if (TimeOfDay >= 0.2f && TimeOfDay <= 0.3f) // Dawn
    {
        // Wake up and become active
        if (CurrentAnimState == ENPC_DinosaurAnimState::Sleeping)
        {
            SetAnimationState(ENPC_DinosaurAnimState::Idle);
        }
    }
}

void UNPC_DinosaurAnimationController::UpdateMovementParameters()
{
    if (!MovementComponent.IsValid())
    {
        return;
    }
    
    // Calculate movement parameters
    FVector Velocity = MovementComponent->Velocity;
    CurrentSpeed = Velocity.Size();
    bIsMoving = CurrentSpeed > 10.0f;
    
    if (bIsMoving)
    {
        // Calculate movement direction relative to forward vector
        if (OwnerCharacter.IsValid())
        {
            FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
            FVector NormalizedVelocity = Velocity.GetSafeNormal();
            MovementDirection = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        }
        
        // Update animation state based on movement
        if (CurrentAnimState == ENPC_DinosaurAnimState::Idle)
        {
            SetAnimationState(ENPC_DinosaurAnimState::Walking);
        }
    }
    else
    {
        // Return to idle if not moving
        if (CurrentAnimState == ENPC_DinosaurAnimState::Walking)
        {
            SetAnimationState(ENPC_DinosaurAnimState::Idle);
        }
    }
    
    UpdateBlendSpaceParameters();
}

void UNPC_DinosaurAnimationController::UpdateBlendSpaceParameters()
{
    if (!AnimInstance.IsValid())
    {
        return;
    }
    
    // Update blend space parameters for smooth locomotion
    if (MovementBlendSpace)
    {
        // Set speed and direction parameters
        // These would be used by the Animation Blueprint
    }
    
    if (CombatBlendSpace && bIsInCombat)
    {
        // Update combat blend space parameters
    }
}

void UNPC_DinosaurAnimationController::ProcessPackSynchronization(float DeltaTime)
{
    PackSyncTimer += DeltaTime;
    
    if (PackSyncTimer >= PackSyncUpdateInterval)
    {
        PackSyncTimer = 0.0f;
        
        // Clean up invalid pack members
        for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
        {
            if (!PackMembers[i].IsValid())
            {
                PackMembers.RemoveAt(i);
            }
        }
        
        // Update pack coordination if leader
        if (bIsPackLeader && PackMembers.Num() > 0)
        {
            // Synchronize basic states with pack members
            for (auto& MemberPtr : PackMembers)
            {
                if (MemberPtr.IsValid())
                {
                    // Simple state synchronization logic
                    if (CurrentAnimState == ENPC_DinosaurAnimState::Alert && 
                        MemberPtr->GetCurrentAnimationState() == ENPC_DinosaurAnimState::Idle)
                    {
                        MemberPtr->SetAnimationState(ENPC_DinosaurAnimState::Alert);
                    }
                }
            }
        }
    }
}

void UNPC_DinosaurAnimationController::UpdateAnimationParameters()
{
    LastAnimationUpdate = GetWorld()->GetTimeSeconds();
    
    // Update animation instance parameters
    if (AnimInstance.IsValid())
    {
        // Set common animation parameters that would be used by Animation Blueprints
        // These parameters would drive the animation state machine
    }
}

void UNPC_DinosaurAnimationController::SetAnimationSpeed(float Speed)
{
    if (AnimInstance.IsValid())
    {
        // Set global animation speed multiplier
        AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
    }
}

void UNPC_DinosaurAnimationController::SetAnimationIntensity(float Intensity)
{
    // Set animation intensity for emotional states
    if (AnimInstance.IsValid())
    {
        // This would control the intensity of animations (subtle vs dramatic)
    }
}

void UNPC_DinosaurAnimationController::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // Broadcast montage completion event
    OnMontageCompleted.Broadcast(Montage);
    
    // Return to appropriate state after montage completion
    if (!bInterrupted)
    {
        SetAnimationState(ENPC_DinosaurAnimState::Idle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimationController: Montage %s ended (interrupted: %s)"), 
           *Montage->GetName(), bInterrupted ? TEXT("true") : TEXT("false"));
}