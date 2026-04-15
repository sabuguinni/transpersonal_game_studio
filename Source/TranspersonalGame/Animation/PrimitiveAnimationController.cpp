#include "PrimitiveAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    MovementData = FAnim_MovementData();
    CombatData = FAnim_CombatData();
    
    // Set default thresholds for primitive human movement
    WalkThreshold = 50.0f;      // Slow cautious walk
    RunThreshold = 200.0f;      // Normal running speed
    SprintThreshold = 400.0f;   // Emergency sprint (limited by stamina)
    
    AnimationBlendSpeed = 5.0f;
    TerrainAdaptationSpeed = 3.0f;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Set initial state
    MovementData.MovementState = EAnim_MovementState::Idle;
    CombatData.CombatState = EAnim_CombatState::Neutral;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent.IsValid() || !AnimInstance.IsValid())
    {
        return;
    }
    
    UpdateMovementBlending(DeltaTime);
    UpdateCombatBlending(DeltaTime);
    HandleStateTransitions();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UPrimitiveAnimationController::InitializeComponents()
{
    if (AActor* Owner = GetOwner())
    {
        // Try to find skeletal mesh component
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (SkeletalMeshComponent.IsValid())
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            UE_LOG(LogTemp, Log, TEXT("Animation components initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UPrimitiveAnimationController::UpdateMovementState(float Speed, FVector Velocity, bool bIsInAir, bool bIsCrouching)
{
    // Store previous state for transition detection
    EAnim_MovementState PreviousState = MovementData.MovementState;
    
    // Update movement data
    MovementData.Speed = Speed;
    MovementData.Velocity = Velocity;
    MovementData.bIsMoving = Speed > 1.0f;
    MovementData.bIsInAir = bIsInAir;
    MovementData.bIsCrouching = bIsCrouching;
    
    // Calculate direction (0-360 degrees relative to forward)
    if (MovementData.bIsMoving && GetOwner())
    {
        FVector ForwardVector = GetOwner()->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        MovementData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        if (MovementData.Direction < 0.0f)
        {
            MovementData.Direction += 360.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Determine new movement state
    MovementData.MovementState = CalculateMovementState(Speed, bIsInAir, bIsCrouching);
    
    // Handle landing detection
    if (bWasInAir && !bIsInAir && PreviousState != EAnim_MovementState::Landing)
    {
        float FallHeight = FMath::Abs(LastVelocity.Z);
        PlayLandingAnimation(FallHeight);
    }
    
    bWasInAir = bIsInAir;
    LastVelocity = Velocity;
}

EAnim_MovementState UPrimitiveAnimationController::CalculateMovementState(float Speed, bool bIsInAir, bool bIsCrouching)
{
    // Air states take priority
    if (bIsInAir)
    {
        return MovementData.Velocity.Z > 0.0f ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    
    // Crouching states
    if (bIsCrouching)
    {
        return Speed > 1.0f ? EAnim_MovementState::Crawling : EAnim_MovementState::Crouching;
    }
    
    // Ground movement states based on speed
    if (Speed < WalkThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (Speed < SprintThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UPrimitiveAnimationController::SetCombatState(EAnim_CombatState NewState)
{
    if (CombatData.CombatState != NewState)
    {
        EAnim_CombatState PreviousState = CombatData.CombatState;
        CombatData.CombatState = NewState;
        
        // Handle state-specific logic
        switch (NewState)
        {
            case EAnim_CombatState::Attacking:
                CombatData.bIsAttacking = true;
                break;
                
            case EAnim_CombatState::Blocking:
                CombatData.bIsBlocking = true;
                CombatData.bIsAttacking = false;
                break;
                
            case EAnim_CombatState::Neutral:
                CombatData.bIsAttacking = false;
                CombatData.bIsBlocking = false;
                CombatData.ComboIndex = 0;
                break;
                
            default:
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UPrimitiveAnimationController::PlayAttackAnimation(int32 AttackIndex)
{
    if (AttackMontage && AnimInstance.IsValid())
    {
        CombatData.ComboIndex = AttackIndex;
        SetCombatState(EAnim_CombatState::Attacking);
        
        // Calculate attack speed based on fatigue and weapon weight
        float PlayRate = CombatData.AttackSpeed * (1.0f - FatigueLevel * 0.3f);
        PlayRate = FMath::Clamp(PlayRate, 0.5f, 2.0f);
        
        PlayMontageIfValid(AttackMontage, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing attack animation %d at rate %.2f"), AttackIndex, PlayRate);
    }
}

void UPrimitiveAnimationController::PlayDodgeAnimation(FVector DodgeDirection)
{
    if (DodgeMontage && AnimInstance.IsValid())
    {
        SetCombatState(EAnim_CombatState::Dodging);
        
        // Adjust dodge speed based on fatigue
        float PlayRate = 1.0f - (FatigueLevel * 0.2f);
        PlayRate = FMath::Clamp(PlayRate, 0.6f, 1.2f);
        
        PlayMontageIfValid(DodgeMontage, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing dodge animation in direction %s"), *DodgeDirection.ToString());
    }
}

void UPrimitiveAnimationController::PlayJumpAnimation()
{
    if (JumpMontage && AnimInstance.IsValid())
    {
        MovementData.MovementState = EAnim_MovementState::Jumping;
        PlayMontageIfValid(JumpMontage);
        
        UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
    }
}

void UPrimitiveAnimationController::PlayLandingAnimation(float FallHeight)
{
    // Different landing animations based on fall height
    if (FallHeight > 800.0f) // High fall - potential damage
    {
        MovementData.MovementState = EAnim_MovementState::Landing;
        // Play heavy landing animation
        UE_LOG(LogTemp, Log, TEXT("Playing heavy landing animation (fall height: %.1f)"), FallHeight);
    }
    else if (FallHeight > 300.0f) // Medium fall
    {
        MovementData.MovementState = EAnim_MovementState::Landing;
        // Play normal landing animation
        UE_LOG(LogTemp, Log, TEXT("Playing normal landing animation (fall height: %.1f)"), FallHeight);
    }
    // Light falls don't need special landing animation
}

void UPrimitiveAnimationController::AdaptToTerrain(float SlopeAngle, ESurfaceType SurfaceType)
{
    CurrentSlopeAngle = SlopeAngle;
    CurrentSurfaceType = SurfaceType;
    
    // Adjust movement parameters based on terrain
    float TerrainSpeedMultiplier = 1.0f;
    
    // Slope adjustments
    if (FMath::Abs(SlopeAngle) > 15.0f)
    {
        TerrainSpeedMultiplier *= 0.8f; // Slower on steep terrain
    }
    
    // Surface type adjustments
    switch (SurfaceType)
    {
        case ESurfaceType::Mud:
            TerrainSpeedMultiplier *= 0.6f;
            break;
        case ESurfaceType::Sand:
            TerrainSpeedMultiplier *= 0.7f;
            break;
        case ESurfaceType::Snow:
            TerrainSpeedMultiplier *= 0.5f;
            break;
        case ESurfaceType::Rock:
            TerrainSpeedMultiplier *= 0.9f;
            break;
        default:
            break;
    }
    
    // Apply terrain adaptation to movement thresholds
    // This affects how quickly we transition between movement states
    
    UE_LOG(LogTemp, Log, TEXT("Adapted to terrain: Slope %.1f°, Surface %d, Speed multiplier %.2f"), 
           SlopeAngle, (int32)SurfaceType, TerrainSpeedMultiplier);
}

void UPrimitiveAnimationController::SetCarryingState(bool bIsCarryingNew, float CarryWeightNew)
{
    bIsCarrying = bIsCarryingNew;
    CarryWeight = CarryWeightNew;
    
    if (bIsCarrying)
    {
        // Adjust movement speeds when carrying heavy objects
        float WeightPenalty = FMath::Clamp(CarryWeight / 100.0f, 0.0f, 0.5f);
        // Movement will be slower and more labored
        
        UE_LOG(LogTemp, Log, TEXT("Now carrying %.1f units of weight (penalty: %.2f)"), 
               CarryWeight, WeightPenalty);
    }
}

void UPrimitiveAnimationController::SetFatigueLevel(float FatiguePercent)
{
    FatigueLevel = FMath::Clamp(FatiguePercent, 0.0f, 1.0f);
    
    // High fatigue affects all animations
    if (FatigueLevel > 0.8f)
    {
        // Very tired - slower, heavier movements
        AnimationBlendSpeed = 3.0f;
    }
    else if (FatigueLevel > 0.5f)
    {
        // Moderately tired
        AnimationBlendSpeed = 4.0f;
    }
    else
    {
        // Fresh and energetic
        AnimationBlendSpeed = 5.0f;
    }
}

void UPrimitiveAnimationController::PlayCraftingAnimation(ECraftingType CraftingType)
{
    if (CraftingMontage && AnimInstance.IsValid())
    {
        // Different crafting animations based on type
        float PlayRate = 1.0f - (FatigueLevel * 0.2f); // Slower when tired
        PlayMontageIfValid(CraftingMontage, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing crafting animation for type %d"), (int32)CraftingType);
    }
}

void UPrimitiveAnimationController::PlayGatheringAnimation(EResourceType ResourceType)
{
    if (GatheringMontage && AnimInstance.IsValid())
    {
        // Different gathering animations for different resources
        float PlayRate = 1.0f - (FatigueLevel * 0.15f);
        PlayMontageIfValid(GatheringMontage, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing gathering animation for resource %d"), (int32)ResourceType);
    }
}

void UPrimitiveAnimationController::PlayRestAnimation(bool bIsSleeping)
{
    if (bIsSleeping)
    {
        // Transition to sleep state
        MovementData.MovementState = EAnim_MovementState::Idle;
        UE_LOG(LogTemp, Log, TEXT("Transitioning to sleep animation"));
    }
    else
    {
        // Simple rest/idle animation
        UE_LOG(LogTemp, Log, TEXT("Playing rest animation"));
    }
}

bool UPrimitiveAnimationController::IsPlayingMontage() const
{
    if (AnimInstance.IsValid())
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    return false;
}

void UPrimitiveAnimationController::UpdateMovementBlending(float DeltaTime)
{
    // Smooth blending between movement states
    // This would typically update blend space parameters
    
    if (AnimInstance.IsValid())
    {
        // Update animation variables that would be used in Animation Blueprint
        // In a real implementation, these would be connected to the AnimBP
        
        float TargetSpeed = MovementData.Speed;
        float BlendedSpeed = FMath::FInterpTo(MovementData.Speed, TargetSpeed, DeltaTime, AnimationBlendSpeed);
        
        // Update movement data with blended values
        MovementData.Speed = BlendedSpeed;
    }
}

void UPrimitiveAnimationController::UpdateCombatBlending(float DeltaTime)
{
    // Handle combat animation blending and transitions
    
    if (CombatData.CombatState == EAnim_CombatState::Attacking && !IsPlayingMontage())
    {
        // Attack animation finished, return to neutral
        SetCombatState(EAnim_CombatState::Neutral);
    }
    
    if (CombatData.CombatState == EAnim_CombatState::Dodging && !IsPlayingMontage())
    {
        // Dodge animation finished, return to alert
        SetCombatState(EAnim_CombatState::Alert);
    }
}

void UPrimitiveAnimationController::HandleStateTransitions()
{
    // Handle complex state transitions that require special logic
    
    // Example: Auto-transition from landing back to appropriate movement state
    if (MovementData.MovementState == EAnim_MovementState::Landing)
    {
        // Check if landing animation should be finished
        float TimeSinceLanding = GetWorld()->GetTimeSeconds() - LastUpdateTime;
        if (TimeSinceLanding > 0.5f) // Landing animation duration
        {
            // Transition back to appropriate movement state
            MovementData.MovementState = CalculateMovementState(
                MovementData.Speed, 
                MovementData.bIsInAir, 
                MovementData.bIsCrouching
            );
        }
    }
}

void UPrimitiveAnimationController::PlayMontageIfValid(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && AnimInstance.IsValid())
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage - invalid montage or anim instance"));
    }
}