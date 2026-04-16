#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize state
    CurrentState = EAnim_PrimitiveState::Idle;
    PreviousState = EAnim_PrimitiveState::Idle;

    // Initialize movement data
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;

    // Initialize timers
    StateTimer = 0.0f;
    ActionTimer = 0.0f;

    // Set speed thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 200.0f;

    // Initialize blend data
    BlendData.BlendTime = 0.3f;
    BlendData.BlendWeight = 0.0f;
    BlendData.bIsBlending = false;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();

    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Initialized for character %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PrimitiveAnimationController: Owner is not a Character!"));
    }

    // Set initial state
    SetAnimationState(EAnim_PrimitiveState::Idle);
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update timers
    UpdateStateTimer(DeltaTime);
    UpdateBlending(DeltaTime);

    // Analyze current movement
    AnalyzeMovement();

    // Check for automatic state transitions
    CheckStateTransitions();
}

void UPrimitiveAnimationController::SetAnimationState(EAnim_PrimitiveState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    EAnim_PrimitiveState OldState = CurrentState;
    
    // Handle state exit
    HandleStateExit(OldState);

    // Update states
    PreviousState = CurrentState;
    CurrentState = NewState;

    // Handle state entry
    HandleStateEntry(NewState);

    // Reset state timer
    StateTimer = 0.0f;

    // Log state change
    LogStateChange(OldState, NewState);
}

void UPrimitiveAnimationController::UpdateMovementAnimation(float Speed, bool bInAir, bool bCrouching)
{
    MovementSpeed = Speed;
    bIsInAir = bInAir;
    bIsCrouching = bCrouching;

    // Update movement direction
    MovementDirection = CalculateMovementDirection();

    // Auto-transition based on movement
    if (ShouldTransitionToMovementState())
    {
        if (bIsInAir)
        {
            if (MovementComponent->IsFalling())
            {
                SetAnimationState(EAnim_PrimitiveState::Falling);
            }
            else
            {
                SetAnimationState(EAnim_PrimitiveState::Jumping);
            }
        }
        else if (bIsCrouching)
        {
            SetAnimationState(EAnim_PrimitiveState::Crouching);
        }
        else if (Speed > RunSpeedThreshold)
        {
            SetAnimationState(EAnim_PrimitiveState::Running);
        }
        else if (Speed > WalkSpeedThreshold)
        {
            SetAnimationState(EAnim_PrimitiveState::Walking);
        }
        else
        {
            SetAnimationState(EAnim_PrimitiveState::Idle);
        }
    }
}

void UPrimitiveAnimationController::PlayGatheringAnimation(float Duration)
{
    SetAnimationState(EAnim_PrimitiveState::Gathering);
    ActionTimer = Duration;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing gathering animation for %.2f seconds"), Duration);
}

void UPrimitiveAnimationController::PlayCraftingAnimation(float Duration)
{
    SetAnimationState(EAnim_PrimitiveState::Crafting);
    ActionTimer = Duration;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing crafting animation for %.2f seconds"), Duration);
}

void UPrimitiveAnimationController::PlayAttackAnimation(bool bIsHeavyAttack)
{
    SetAnimationState(EAnim_PrimitiveState::Attacking);
    ActionTimer = bIsHeavyAttack ? 1.5f : 0.8f;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing %s attack animation"), 
           bIsHeavyAttack ? TEXT("heavy") : TEXT("light"));
}

void UPrimitiveAnimationController::PlayInjuredAnimation(float Severity)
{
    SetAnimationState(EAnim_PrimitiveState::Injured);
    ActionTimer = FMath::Lerp(1.0f, 3.0f, Severity);
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing injured animation (severity: %.2f)"), Severity);
}

void UPrimitiveAnimationController::BlendToState(EAnim_PrimitiveState TargetState, float BlendDuration)
{
    BlendData.bIsBlending = true;
    BlendData.BlendTime = BlendDuration;
    BlendData.BlendWeight = 0.0f;

    // Set target state after blend completes
    SetAnimationState(TargetState);
}

void UPrimitiveAnimationController::UpdateStateTimer(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    if (ActionTimer > 0.0f)
    {
        ActionTimer -= DeltaTime;
        if (ActionTimer <= 0.0f)
        {
            ActionTimer = 0.0f;
            // Action completed, return to appropriate state
            if (IsMoving())
            {
                UpdateMovementAnimation(MovementSpeed, bIsInAir, bIsCrouching);
            }
            else
            {
                SetAnimationState(EAnim_PrimitiveState::Idle);
            }
        }
    }
}

void UPrimitiveAnimationController::UpdateBlending(float DeltaTime)
{
    if (BlendData.bIsBlending)
    {
        BlendData.BlendWeight += DeltaTime / BlendData.BlendTime;
        
        if (BlendData.BlendWeight >= 1.0f)
        {
            BlendData.BlendWeight = 1.0f;
            BlendData.bIsBlending = false;
        }
    }
}

void UPrimitiveAnimationController::CheckStateTransitions()
{
    // Only auto-transition if not in an action state
    if (ActionTimer > 0.0f)
    {
        return;
    }

    // Check for death state
    if (OwnerCharacter)
    {
        // This would typically check health from a health component
        // For now, we'll skip this check
    }

    // Auto-transition based on movement when not in action
    if (ShouldTransitionToMovementState())
    {
        UpdateMovementAnimation(MovementSpeed, bIsInAir, bIsCrouching);
    }
}

void UPrimitiveAnimationController::HandleStateEntry(EAnim_PrimitiveState NewState)
{
    switch (NewState)
    {
        case EAnim_PrimitiveState::Idle:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Idle state"));
            break;
            
        case EAnim_PrimitiveState::Walking:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Walking state"));
            break;
            
        case EAnim_PrimitiveState::Running:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Running state"));
            break;
            
        case EAnim_PrimitiveState::Jumping:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Jumping state"));
            break;
            
        case EAnim_PrimitiveState::Falling:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Falling state"));
            break;
            
        case EAnim_PrimitiveState::Crouching:
            UE_LOG(LogTemp, Verbose, TEXT("Entering Crouching state"));
            break;
            
        case EAnim_PrimitiveState::Attacking:
            UE_LOG(LogTemp, Log, TEXT("Entering Attacking state"));
            break;
            
        case EAnim_PrimitiveState::Gathering:
            UE_LOG(LogTemp, Log, TEXT("Entering Gathering state"));
            break;
            
        case EAnim_PrimitiveState::Crafting:
            UE_LOG(LogTemp, Log, TEXT("Entering Crafting state"));
            break;
            
        case EAnim_PrimitiveState::Injured:
            UE_LOG(LogTemp, Warning, TEXT("Entering Injured state"));
            break;
            
        case EAnim_PrimitiveState::Dead:
            UE_LOG(LogTemp, Warning, TEXT("Entering Dead state"));
            break;
    }
}

void UPrimitiveAnimationController::HandleStateExit(EAnim_PrimitiveState OldState)
{
    // Cleanup for specific states
    switch (OldState)
    {
        case EAnim_PrimitiveState::Attacking:
            // Reset attack flags, etc.
            break;
            
        case EAnim_PrimitiveState::Gathering:
            // Complete gathering action
            break;
            
        case EAnim_PrimitiveState::Crafting:
            // Complete crafting action
            break;
            
        default:
            break;
    }
}

void UPrimitiveAnimationController::AnalyzeMovement()
{
    if (!MovementComponent)
    {
        return;
    }

    // Get current velocity
    FVector Velocity = MovementComponent->Velocity;
    MovementSpeed = Velocity.Size2D();

    // Update air state
    bIsInAir = MovementComponent->IsFalling() || MovementComponent->IsFlying();

    // Update crouching state
    bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement direction relative to character facing
    MovementDirection = CalculateMovementDirection();
}

float UPrimitiveAnimationController::CalculateMovementDirection()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }

    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.Size2D() < 5.0f)
    {
        return 0.0f;
    }

    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = Velocity.GetSafeNormal2D();

    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;

    return FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
}

bool UPrimitiveAnimationController::ShouldTransitionToMovementState()
{
    // Don't auto-transition if in an action state with active timer
    if (ActionTimer > 0.0f)
    {
        return false;
    }

    // Don't auto-transition if in death state
    if (CurrentState == EAnim_PrimitiveState::Dead)
    {
        return false;
    }

    return true;
}

void UPrimitiveAnimationController::ResetActionTimer()
{
    ActionTimer = 0.0f;
}

bool UPrimitiveAnimationController::IsActionComplete() const
{
    return ActionTimer <= 0.0f;
}

void UPrimitiveAnimationController::LogStateChange(EAnim_PrimitiveState From, EAnim_PrimitiveState To)
{
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: State transition %d -> %d"), 
           static_cast<int32>(From), static_cast<int32>(To));
}