#include "PrimitiveAnimationController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousCombatState = EAnim_CombatState::Unarmed;
    
    // Initialize timing
    LastStateChangeTime = 0.0f;
    
    // Initialize thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    StateTransitionTime = 0.2f;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get animation instance from owner character
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
            if (AnimInstance)
            {
                UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Animation instance found"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No animation instance found"));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: BeginPlay completed"));
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!AnimInstance)
    {
        return;
    }
    
    // Update movement data from character
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
        {
            // Update movement data
            FVector Velocity = MovementComp->Velocity;
            MovementData.Speed = Velocity.Size2D();
            MovementData.Direction = FMath::Atan2(Velocity.Y, Velocity.X);
            MovementData.bIsInAir = MovementComp->IsFalling();
            MovementData.bIsCrouching = MovementComp->IsCrouching();
            
            // Calculate ground distance for foot IK
            FHitResult HitResult;
            FVector StartLocation = OwnerCharacter->GetActorLocation();
            FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
            {
                MovementData.GroundDistance = HitResult.Distance;
            }
            else
            {
                MovementData.GroundDistance = 200.0f; // Max distance
            }
        }
    }
    
    // Determine new states based on current data
    DetermineMovementState();
    DetermineCombatState();
    
    // Handle smooth transitions
    HandleStateTransitions();
}

void UPrimitiveAnimationController::UpdateMovementState(const FAnim_MovementData& NewMovementData)
{
    MovementData = NewMovementData;
    DetermineMovementState();
}

void UPrimitiveAnimationController::UpdateCombatState(const FAnim_CombatData& NewCombatData)
{
    CombatData = NewCombatData;
    DetermineCombatState();
}

void UPrimitiveAnimationController::TriggerMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Cannot play montage - missing AnimInstance or Montage"));
        return;
    }
    
    // Stop any currently playing montage
    AnimInstance->StopAllMontages(0.2f);
    
    // Play the new montage
    AnimInstance->Montage_Play(Montage, PlayRate);
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing montage %s at rate %f"), 
           *Montage->GetName(), PlayRate);
}

void UPrimitiveAnimationController::StopAllMontages()
{
    if (AnimInstance)
    {
        AnimInstance->StopAllMontages(StateTransitionTime);
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Stopped all montages"));
    }
}

void UPrimitiveAnimationController::DetermineMovementState()
{
    EAnim_MovementState NewState = CurrentMovementState;
    
    // Priority order: Air states -> Ground states
    if (MovementData.bIsInAir)
    {
        // Check if we're jumping up or falling down
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            float VerticalVelocity = OwnerCharacter->GetVelocity().Z;
            if (VerticalVelocity > 0.0f)
            {
                NewState = EAnim_MovementState::Jumping;
            }
            else
            {
                NewState = EAnim_MovementState::Falling;
            }
        }
    }
    else
    {
        // Ground-based movement
        if (MovementData.bIsCrouching)
        {
            NewState = EAnim_MovementState::Crouching;
        }
        else if (MovementData.Speed < 10.0f)
        {
            NewState = EAnim_MovementState::Idle;
        }
        else if (MovementData.Speed < RunSpeedThreshold)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else
        {
            NewState = EAnim_MovementState::Running;
        }
    }
    
    // Update state if changed
    if (NewState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Movement state changed to %d"), 
               (int32)CurrentMovementState);
    }
}

void UPrimitiveAnimationController::DetermineCombatState()
{
    EAnim_CombatState NewState = CurrentCombatState;
    
    // Combat state logic based on combat data
    if (CombatData.bIsAttacking)
    {
        NewState = EAnim_CombatState::Attacking;
    }
    else if (CombatData.bIsBlocking)
    {
        NewState = EAnim_CombatState::Blocking;
    }
    else if (CombatData.bHasWeapon)
    {
        // Determine weapon type and set appropriate ready state
        NewState = EAnim_CombatState::MeleeReady; // Default to melee for now
    }
    else
    {
        NewState = EAnim_CombatState::Unarmed;
    }
    
    // Update state if changed
    if (NewState != CurrentCombatState)
    {
        PreviousCombatState = CurrentCombatState;
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Combat state changed to %d"), 
               (int32)CurrentCombatState);
    }
}

void UPrimitiveAnimationController::HandleStateTransitions()
{
    if (!AnimInstance)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastChange = CurrentTime - LastStateChangeTime;
    
    // Only handle transitions after minimum transition time
    if (TimeSinceLastChange < StateTransitionTime)
    {
        return;
    }
    
    // Play appropriate montages based on current state
    switch (CurrentMovementState)
    {
        case EAnim_MovementState::Idle:
            if (IdleMontage && PreviousMovementState != EAnim_MovementState::Idle)
            {
                TriggerMontage(IdleMontage, 1.0f);
            }
            break;
            
        case EAnim_MovementState::Walking:
            if (WalkMontage && PreviousMovementState != EAnim_MovementState::Walking)
            {
                TriggerMontage(WalkMontage, 1.0f);
            }
            break;
            
        case EAnim_MovementState::Running:
            if (RunMontage && PreviousMovementState != EAnim_MovementState::Running)
            {
                TriggerMontage(RunMontage, 1.0f);
            }
            break;
            
        case EAnim_MovementState::Jumping:
            if (JumpMontage && PreviousMovementState != EAnim_MovementState::Jumping)
            {
                TriggerMontage(JumpMontage, 1.0f);
            }
            break;
            
        default:
            break;
    }
    
    // Handle combat state transitions
    switch (CurrentCombatState)
    {
        case EAnim_CombatState::Attacking:
            if (AttackMontage && PreviousCombatState != EAnim_CombatState::Attacking)
            {
                TriggerMontage(AttackMontage, CombatData.AttackSpeed);
            }
            break;
            
        default:
            break;
    }
}