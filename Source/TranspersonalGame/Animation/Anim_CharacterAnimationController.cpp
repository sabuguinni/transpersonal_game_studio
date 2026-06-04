#include "Anim_CharacterAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    AnimationSmoothingSpeed = 10.0f;
    LastUpdateTime = 0.0f;
    LastVelocity = FVector::ZeroVector;
    bWasMovingLastFrame = false;
    
    // Initialize component pointers
    MeshComponent = nullptr;
    MovementComponent = nullptr;
    AnimationBlueprint = nullptr;
    MovementBlendSpace = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Set initial animation state
    MovementData.MovementState = EAnim_MovementState::Idle;
    ActionData.CurrentAction = EAnim_ActionState::None;
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (MeshComponent && MovementComponent)
    {
        UpdateMovementAnimation(DeltaTime);
    }
}

void UAnim_CharacterAnimationController::InitializeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: No owner found"));
        return;
    }
    
    // Get mesh component
    MeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: No SkeletalMeshComponent found on owner"));
    }
    
    // Get movement component
    MovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: No CharacterMovementComponent found on owner"));
    }
    
    // Apply animation blueprint if set
    if (MeshComponent && AnimationBlueprint)
    {
        MeshComponent->SetAnimInstanceClass(AnimationBlueprint);
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Applied animation blueprint"));
    }
}

void UAnim_CharacterAnimationController::UpdateMovementAnimation(float DeltaTime)
{
    if (!MovementComponent || !MeshComponent)
    {
        return;
    }
    
    UpdateMovementData(DeltaTime);
    UpdateActionData(DeltaTime);
    
    // Update animation instance variables if available
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        // Set animation variables (these would be read by the Animation Blueprint)
        // Note: In a real implementation, you'd create custom AnimInstance class
        // with these variables exposed for the Animation Blueprint to read
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_CharacterAnimationController::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector CurrentVelocity = MovementComponent->Velocity;
    float CurrentSpeed = CurrentVelocity.Size();
    
    // Smooth speed changes
    float TargetSpeed = CurrentSpeed;
    MovementData.Speed = FMath::FInterpTo(MovementData.Speed, TargetSpeed, DeltaTime, AnimationSmoothingSpeed);
    
    // Calculate movement direction
    MovementData.Direction = CalculateMovementDirection();
    
    // Update movement state
    EAnim_MovementState NewState = CalculateMovementState();
    if (NewState != MovementData.MovementState)
    {
        SetMovementState(NewState);
    }
    
    // Update movement flags
    MovementData.bIsMoving = CurrentSpeed > 10.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Store for next frame
    LastVelocity = CurrentVelocity;
    bWasMovingLastFrame = MovementData.bIsMoving;
}

void UAnim_CharacterAnimationController::UpdateActionData(float DeltaTime)
{
    // Update action progress if performing an action
    if (ActionData.bIsPerformingAction)
    {
        // Check if montage is still playing
        if (!IsPlayingActionMontage())
        {
            // Action completed
            ActionData.bIsPerformingAction = false;
            ActionData.CurrentAction = EAnim_ActionState::None;
            ActionData.ActionProgress = 0.0f;
            ActionData.ActionTarget = TEXT("");
        }
        else
        {
            // Update progress based on montage position
            if (MeshComponent && MeshComponent->GetAnimInstance())
            {
                UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
                if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
                {
                    float MontagePosition = AnimInstance->Montage_GetPosition(CurrentMontage);
                    float MontageLength = CurrentMontage->GetPlayLength();
                    ActionData.ActionProgress = (MontageLength > 0.0f) ? (MontagePosition / MontageLength) : 0.0f;
                }
            }
        }
    }
}

EAnim_MovementState UAnim_CharacterAnimationController::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for special movement states first
    if (MovementComponent->IsFalling())
    {
        return EAnim_MovementState::Falling;
    }
    
    if (MovementComponent->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check speed-based states
    float CurrentSpeed = MovementComponent->Velocity.Size();
    
    if (CurrentSpeed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentSpeed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

float UAnim_CharacterAnimationController::CalculateMovementDirection() const
{
    if (!MovementComponent || !GetOwner())
    {
        return 0.0f;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    FVector Forward = GetOwner()->GetActorForwardVector();
    FVector Right = GetOwner()->GetActorRightVector();
    
    // Normalize velocity to get direction
    Velocity.Normalize();
    
    // Calculate angle relative to forward direction
    float ForwardDot = FVector::DotProduct(Forward, Velocity);
    float RightDot = FVector::DotProduct(Right, Velocity);
    
    return FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
}

void UAnim_CharacterAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_MovementState PreviousState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_CharacterAnimationController::PlayActionMontage(EAnim_ActionState ActionType, float PlayRate)
{
    if (!MeshComponent || !MeshComponent->GetAnimInstance())
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: Cannot play montage - no mesh or anim instance"));
        return;
    }
    
    UAnimMontage** FoundMontage = ActionMontages.Find(ActionType);
    if (!FoundMontage || !*FoundMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: No montage registered for action type %d"), (int32)ActionType);
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    float MontageLength = AnimInstance->Montage_Play(*FoundMontage, PlayRate);
    
    if (MontageLength > 0.0f)
    {
        ActionData.CurrentAction = ActionType;
        ActionData.bIsPerformingAction = true;
        ActionData.ActionProgress = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Started action montage for type %d"), (int32)ActionType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: Failed to play montage for action type %d"), (int32)ActionType);
    }
}

void UAnim_CharacterAnimationController::StopActionMontage()
{
    if (MeshComponent && MeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        AnimInstance->Montage_Stop(0.2f); // 0.2 second blend out
        
        ActionData.bIsPerformingAction = false;
        ActionData.CurrentAction = EAnim_ActionState::None;
        ActionData.ActionProgress = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Stopped action montage"));
    }
}

bool UAnim_CharacterAnimationController::IsPlayingActionMontage() const
{
    if (MeshComponent && MeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        return AnimInstance->IsAnyMontagePlaying();
    }
    
    return false;
}

void UAnim_CharacterAnimationController::SetAnimationBlueprint(TSubclassOf<UAnimInstance> NewAnimBP)
{
    AnimationBlueprint = NewAnimBP;
    
    if (MeshComponent && AnimationBlueprint)
    {
        MeshComponent->SetAnimInstanceClass(AnimationBlueprint);
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Set new animation blueprint"));
    }
}

void UAnim_CharacterAnimationController::RegisterAnimationMontage(EAnim_ActionState ActionType, UAnimMontage* Montage)
{
    if (Montage)
    {
        ActionMontages.Add(ActionType, Montage);
        UE_LOG(LogTemp, Log, TEXT("UAnim_CharacterAnimationController: Registered montage for action type %d"), (int32)ActionType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_CharacterAnimationController: Attempted to register null montage for action type %d"), (int32)ActionType);
    }
}