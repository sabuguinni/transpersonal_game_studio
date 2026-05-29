#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS

    // Initialize default states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousCombatState = EAnim_CombatState::Unarmed;
    
    // Initialize timing
    LastStateChangeTime = 0.0f;
    
    // Initialize animation data with defaults
    MovementData = FAnim_MovementData();
    CombatData = FAnim_CombatData();
    
    // Set default thresholds for realistic movement
    WalkSpeedThreshold = 100.0f;   // Walking speed threshold
    RunSpeedThreshold = 300.0f;    // Running speed threshold
    StateTransitionTime = 0.2f;    // Smooth transition time
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to the character's animation instance
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
            if (AnimInstance)
            {
                UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Animation instance found and cached"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No animation instance found on character mesh"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PrimitiveAnimationController: Owner is not a Character class"));
    }
    
    // Initialize with idle state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update movement data from character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // Update movement data
            FVector Velocity = MovementComp->Velocity;
            MovementData.Speed = Velocity.Size();
            MovementData.Direction = FMath::Atan2(Velocity.Y, Velocity.X);
            MovementData.bIsInAir = MovementComp->IsFalling();
            MovementData.bIsCrouching = MovementComp->IsCrouching();
            
            // Calculate ground distance for IK
            FVector StartLocation = Character->GetActorLocation();
            FVector EndLocation = StartLocation - FVector(0, 0, 200.0f); // Trace down 2 meters
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(Character);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
            {
                MovementData.GroundDistance = HitResult.Distance;
            }
            else
            {
                MovementData.GroundDistance = 200.0f; // Max distance if no ground found
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
    
    // Stop any currently playing montages in the same slot
    AnimInstance->StopAllMontagesByGroupName(Montage->GetGroupName());
    
    // Play the new montage
    float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
    
    if (Duration > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing montage %s with duration %.2f"), 
               *Montage->GetName(), Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Failed to play montage %s"), 
               *Montage->GetName());
    }
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
    
    // Determine new state based on movement data
    if (MovementData.bIsInAir)
    {
        NewState = EAnim_MovementState::Falling;
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.Speed < 10.0f) // Very low speed threshold for idle
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < WalkSpeedThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else
    {
        NewState = EAnim_MovementState::Running; // Max speed is still running
    }
    
    // Update state if changed
    if (NewState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Movement state changed from %d to %d"), 
               (int32)PreviousMovementState, (int32)CurrentMovementState);
    }
}

void UPrimitiveAnimationController::DetermineCombatState()
{
    EAnim_CombatState NewState = CurrentCombatState;
    
    // Determine new state based on combat data
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
        NewState = EAnim_CombatState::MeleeReady;
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
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Combat state changed from %d to %d"), 
               (int32)PreviousCombatState, (int32)CurrentCombatState);
    }
}

void UPrimitiveAnimationController::HandleStateTransitions()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastChange = CurrentTime - LastStateChangeTime;
    
    // Only handle transitions if enough time has passed for smooth blending
    if (TimeSinceLastChange >= StateTransitionTime)
    {
        // Trigger appropriate montages based on current states
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
            case EAnim_MovementState::Falling:
                if (JumpMontage && (PreviousMovementState != EAnim_MovementState::Jumping && 
                                   PreviousMovementState != EAnim_MovementState::Falling))
                {
                    TriggerMontage(JumpMontage, 1.0f);
                }
                break;
                
            default:
                break;
        }
        
        // Handle combat state transitions
        if (CurrentCombatState == EAnim_CombatState::Attacking && AttackMontage)
        {
            if (PreviousCombatState != EAnim_CombatState::Attacking)
            {
                float AttackPlayRate = FMath::Clamp(CombatData.AttackSpeed, 0.5f, 2.0f);
                TriggerMontage(AttackMontage, AttackPlayRate);
            }
        }
    }
}