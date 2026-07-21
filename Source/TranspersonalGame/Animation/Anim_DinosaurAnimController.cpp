#include "Anim_DinosaurAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_DinosaurAnimController::UAnim_DinosaurAnimController()
{
    // Initialize default values
    MovementState = EAnim_DinosaurMovementState::Idle;
    BehaviorState = EAnim_DinosaurBehaviorState::Passive;
    Species = EAnim_DinosaurSpecies::Generic;
    SizeCategory = EAnim_DinosaurSize::Medium;
    
    Speed = 0.0f;
    Direction = 0.0f;
    TurnRate = 0.0f;
    bIsMoving = false;
    bIsRunning = false;
    bIsCharging = false;
    bIsTurning = false;
    
    AlertnessLevel = 0.0f;
    AggressionLevel = 0.0f;
    HungerLevel = 0.5f;
    FearLevel = 0.0f;
    bIsAlert = false;
    bIsHunting = false;
    bIsFleeing = false;
    bIsFeeding = false;
    
    // Initialize blend data
    BlendData = FAnim_DinosaurBlendData();
    
    // Initialize foot IK
    FootIK = FAnim_DinosaurFootIK();
    
    // Species parameters
    BodyLength = 5.0f;
    BodyHeight = 2.0f;
    NeckLength = 1.0f;
    TailLength = 3.0f;
    bIsBiped = false;
    bIsHerbivore = true;
    bIsCarnivore = false;
    bIsPackHunter = false;
    
    // Internal state
    PreviousMovementState = EAnim_DinosaurMovementState::Idle;
    PreviousBehaviorState = EAnim_DinosaurBehaviorState::Passive;
    StateTransitionTimer = 0.0f;
    LastSpeedUpdate = 0.0f;
    LastDirectionUpdate = 0.0f;
    
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_DinosaurAnimController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Initialized for character %s"), *OwningCharacter->GetName());
        
        // Configure species-specific settings
        ConfigureForSpecies(Species);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimController: Failed to get owning character"));
    }
}

void UAnim_DinosaurAnimController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement parameters
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    Direction = FMath::Atan2(Velocity.Y, Velocity.X) * 180.0f / PI;
    
    // Update movement flags
    bIsMoving = Speed > 1.0f;
    bIsRunning = Speed > 300.0f;
    bIsCharging = Speed > 600.0f;
    
    // Calculate turn rate
    float CurrentYaw = OwningCharacter->GetActorRotation().Yaw;
    float DeltaYaw = FMath::Abs(CurrentYaw - LastDirectionUpdate);
    TurnRate = DeltaYaw / DeltaTimeX;
    bIsTurning = TurnRate > 45.0f;
    LastDirectionUpdate = CurrentYaw;
    
    // Update animation states
    UpdateMovementState();
    UpdateBehaviorState();
    UpdateAnimationBlending(DeltaTimeX);
    
    // Update foot IK if enabled
    if (FootIK.bEnableFootIK)
    {
        UpdateFootIK();
    }
    
    // Update species parameters
    UpdateSpeciesParameters();
    
    // Update state transition timer
    StateTransitionTimer += DeltaTimeX;
    
    // Log state changes for debugging
    if (MovementState != PreviousMovementState || BehaviorState != PreviousBehaviorState)
    {
        LogAnimationState();
        PreviousMovementState = MovementState;
        PreviousBehaviorState = BehaviorState;
        StateTransitionTimer = 0.0f;
    }
}

void UAnim_DinosaurAnimController::SetMovementState(EAnim_DinosaurMovementState NewState)
{
    if (ShouldTransitionToState(NewState))
    {
        MovementState = NewState;
        StateTransitionTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Movement state changed to %d"), (int32)NewState);
    }
}

void UAnim_DinosaurAnimController::SetBehaviorState(EAnim_DinosaurBehaviorState NewState)
{
    if (BehaviorState != NewState)
    {
        BehaviorState = NewState;
        StateTransitionTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Behavior state changed to %d"), (int32)NewState);
        
        // Update behavior flags
        bIsAlert = (NewState == EAnim_DinosaurBehaviorState::Territorial || 
                   NewState == EAnim_DinosaurBehaviorState::Hunting);
        bIsHunting = (NewState == EAnim_DinosaurBehaviorState::Hunting);
        bIsFleeing = (NewState == EAnim_DinosaurBehaviorState::Fleeing);
        bIsFeeding = (NewState == EAnim_DinosaurBehaviorState::Feeding);
    }
}

void UAnim_DinosaurAnimController::SetSpecies(EAnim_DinosaurSpecies NewSpecies)
{
    if (Species != NewSpecies)
    {
        Species = NewSpecies;
        ConfigureForSpecies(NewSpecies);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Species changed to %d"), (int32)NewSpecies);
    }
}

void UAnim_DinosaurAnimController::UpdateMovementParameters(float NewSpeed, float NewDirection)
{
    Speed = NewSpeed;
    Direction = NewDirection;
    
    // Update movement flags based on speed
    bIsMoving = Speed > 1.0f;
    bIsRunning = Speed > 300.0f;
    bIsCharging = Speed > 600.0f;
}

void UAnim_DinosaurAnimController::UpdateBehaviorParameters(float Alertness, float Aggression, float Hunger, float Fear)
{
    AlertnessLevel = FMath::Clamp(Alertness, 0.0f, 1.0f);
    AggressionLevel = FMath::Clamp(Aggression, 0.0f, 1.0f);
    HungerLevel = FMath::Clamp(Hunger, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    
    // Update behavior flags
    bIsAlert = AlertnessLevel > 0.5f;
    
    // Determine behavior state based on parameters
    if (FearLevel > 0.7f)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Fleeing);
    }
    else if (AggressionLevel > 0.6f && bIsCarnivore)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Hunting);
    }
    else if (HungerLevel > 0.8f)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Feeding);
    }
    else if (AlertnessLevel > 0.5f)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Territorial);
    }
    else
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Passive);
    }
}

void UAnim_DinosaurAnimController::TriggerRoar()
{
    SetMovementState(EAnim_DinosaurMovementState::Roaring);
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Roar triggered"));
}

void UAnim_DinosaurAnimController::TriggerAttack()
{
    SetMovementState(EAnim_DinosaurMovementState::Attacking);
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Attack triggered"));
}

void UAnim_DinosaurAnimController::TriggerDeath()
{
    SetMovementState(EAnim_DinosaurMovementState::Dying);
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Death triggered"));
}

void UAnim_DinosaurAnimController::UpdateFootIK()
{
    if (!OwningCharacter || !FootIK.bEnableFootIK)
    {
        return;
    }
    
    // Get foot bone locations (approximate)
    FVector ActorLocation = OwningCharacter->GetActorLocation();
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    FVector RightVector = OwningCharacter->GetActorRightVector();
    
    // Calculate foot positions based on species type
    float FootSpacing = BodyLength * 0.3f;
    float SideSpacing = BodyLength * 0.15f;
    
    FVector LeftFootPos = ActorLocation - (RightVector * SideSpacing);
    FVector RightFootPos = ActorLocation + (RightVector * SideSpacing);
    
    if (!bIsBiped)
    {
        // Quadruped - front and back feet
        FVector FrontLeftPos = ActorLocation + (ForwardVector * FootSpacing) - (RightVector * SideSpacing);
        FVector FrontRightPos = ActorLocation + (ForwardVector * FootSpacing) + (RightVector * SideSpacing);
        
        FootIK.FrontLeftFootOffset = TraceForGround(FrontLeftPos, FootIK.TraceDistance);
        FootIK.FrontRightFootOffset = TraceForGround(FrontRightPos, FootIK.TraceDistance);
    }
    
    // Trace for ground
    FootIK.LeftFootOffset = TraceForGround(LeftFootPos, FootIK.TraceDistance);
    FootIK.RightFootOffset = TraceForGround(RightFootPos, FootIK.TraceDistance);
    
    // Update IK alpha based on movement
    FootIK.IKAlpha = bIsMoving ? 0.7f : 1.0f;
}

FVector UAnim_DinosaurAnimController::TraceForGround(const FVector& FootLocation, float TraceDistance)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }
    
    UWorld* World = OwningCharacter->GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    FVector StartLocation = FootLocation + FVector(0, 0, 50.0f);
    FVector EndLocation = FootLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        float GroundOffset = HitResult.Location.Z - FootLocation.Z;
        return FVector(0, 0, GroundOffset);
    }
    
    return FVector::ZeroVector;
}

void UAnim_DinosaurAnimController::EnableFootIK(bool bEnable)
{
    FootIK.bEnableFootIK = bEnable;
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Foot IK %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnim_DinosaurAnimController::ConfigureForSpecies(EAnim_DinosaurSpecies TargetSpecies)
{
    Species = TargetSpecies;
    
    switch (TargetSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:
            SizeCategory = EAnim_DinosaurSize::Massive;
            BodyLength = 12.0f;
            BodyHeight = 4.0f;
            NeckLength = 1.5f;
            TailLength = 8.0f;
            bIsBiped = true;
            bIsHerbivore = false;
            bIsCarnivore = true;
            bIsPackHunter = false;
            break;
            
        case EAnim_DinosaurSpecies::Velociraptor:
            SizeCategory = EAnim_DinosaurSize::Small;
            BodyLength = 2.0f;
            BodyHeight = 1.0f;
            NeckLength = 0.5f;
            TailLength = 1.5f;
            bIsBiped = true;
            bIsHerbivore = false;
            bIsCarnivore = true;
            bIsPackHunter = true;
            break;
            
        case EAnim_DinosaurSpecies::Triceratops:
            SizeCategory = EAnim_DinosaurSize::Large;
            BodyLength = 9.0f;
            BodyHeight = 3.0f;
            NeckLength = 1.0f;
            TailLength = 4.0f;
            bIsBiped = false;
            bIsHerbivore = true;
            bIsCarnivore = false;
            bIsPackHunter = false;
            break;
            
        case EAnim_DinosaurSpecies::Brachiosaurus:
            SizeCategory = EAnim_DinosaurSize::Massive;
            BodyLength = 22.0f;
            BodyHeight = 12.0f;
            NeckLength = 8.0f;
            TailLength = 10.0f;
            bIsBiped = false;
            bIsHerbivore = true;
            bIsCarnivore = false;
            bIsPackHunter = false;
            break;
            
        case EAnim_DinosaurSpecies::Ankylosaurus:
            SizeCategory = EAnim_DinosaurSize::Large;
            BodyLength = 6.0f;
            BodyHeight = 1.5f;
            NeckLength = 0.8f;
            TailLength = 3.0f;
            bIsBiped = false;
            bIsHerbivore = true;
            bIsCarnivore = false;
            bIsPackHunter = false;
            break;
            
        default:
            // Generic dinosaur settings
            SizeCategory = EAnim_DinosaurSize::Medium;
            BodyLength = 5.0f;
            BodyHeight = 2.0f;
            NeckLength = 1.0f;
            TailLength = 3.0f;
            bIsBiped = false;
            bIsHerbivore = true;
            bIsCarnivore = false;
            bIsPackHunter = false;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController: Configured for species %d - Length: %.1f, Height: %.1f, Biped: %s"), 
           (int32)TargetSpecies, BodyLength, BodyHeight, bIsBiped ? TEXT("Yes") : TEXT("No"));
}

void UAnim_DinosaurAnimController::UpdateMovementState()
{
    EAnim_DinosaurMovementState NewState = MovementState;
    
    // Determine movement state based on speed and behavior
    if (Speed < 1.0f)
    {
        if (BehaviorState == EAnim_DinosaurBehaviorState::Feeding)
        {
            NewState = bIsHerbivore ? EAnim_DinosaurMovementState::Grazing : EAnim_DinosaurMovementState::Idle;
        }
        else if (BehaviorState == EAnim_DinosaurBehaviorState::Passive && HungerLevel < 0.3f)
        {
            NewState = EAnim_DinosaurMovementState::Sleeping;
        }
        else if (bIsAlert)
        {
            NewState = EAnim_DinosaurMovementState::Alert;
        }
        else
        {
            NewState = EAnim_DinosaurMovementState::Idle;
        }
    }
    else if (Speed < 200.0f)
    {
        NewState = EAnim_DinosaurMovementState::Walking;
    }
    else if (Speed < 500.0f)
    {
        NewState = EAnim_DinosaurMovementState::Running;
    }
    else
    {
        NewState = EAnim_DinosaurMovementState::Charging;
    }
    
    // Handle turning
    if (bIsTurning && Speed < 100.0f)
    {
        NewState = EAnim_DinosaurMovementState::Turning;
    }
    
    // Override with behavior-specific states
    if (BehaviorState == EAnim_DinosaurBehaviorState::Hunting && AggressionLevel > 0.8f)
    {
        NewState = EAnim_DinosaurMovementState::Attacking;
    }
    
    SetMovementState(NewState);
}

void UAnim_DinosaurAnimController::UpdateBehaviorState()
{
    // Behavior state is primarily updated through UpdateBehaviorParameters
    // This function handles automatic state transitions based on time and conditions
    
    if (StateTransitionTimer > 5.0f) // 5 second minimum state duration
    {
        // Random behavior changes for passive dinosaurs
        if (BehaviorState == EAnim_DinosaurBehaviorState::Passive && FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            if (HungerLevel > 0.6f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Feeding);
            }
            else if (bIsPackHunter && FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Socializing);
            }
        }
        
        // Return to passive state after specific behaviors
        if (BehaviorState == EAnim_DinosaurBehaviorState::Feeding && StateTransitionTimer > 10.0f)
        {
            SetBehaviorState(EAnim_DinosaurBehaviorState::Passive);
        }
    }
}

void UAnim_DinosaurAnimController::UpdateAnimationBlending(float DeltaTime)
{
    // Smooth blending between animation states
    // This would be used in the Animation Blueprint for smooth transitions
    
    // Update blend weights based on current state
    float TargetMovementBlend = 1.0f;
    float TargetBehaviorBlend = 1.0f;
    
    // Apply blending logic here
    // In a real implementation, this would update blend space parameters
}

void UAnim_DinosaurAnimController::UpdateSpeciesParameters()
{
    // Update species-specific animation parameters
    // This could adjust animation playback rates, bone scaling, etc.
    
    // Example: Larger dinosaurs move slower
    float SizeMultiplier = 1.0f;
    switch (SizeCategory)
    {
        case EAnim_DinosaurSize::Small:
            SizeMultiplier = 1.5f;
            break;
        case EAnim_DinosaurSize::Medium:
            SizeMultiplier = 1.0f;
            break;
        case EAnim_DinosaurSize::Large:
            SizeMultiplier = 0.8f;
            break;
        case EAnim_DinosaurSize::Massive:
            SizeMultiplier = 0.6f;
            break;
    }
    
    // Apply size-based animation speed adjustments
    // This would be used in the Animation Blueprint
}

bool UAnim_DinosaurAnimController::ShouldTransitionToState(EAnim_DinosaurMovementState NewState) const
{
    // Prevent rapid state changes
    if (StateTransitionTimer < BlendData.MovementBlendTime)
    {
        return false;
    }
    
    // Allow immediate transitions for critical states
    if (NewState == EAnim_DinosaurMovementState::Attacking ||
        NewState == EAnim_DinosaurMovementState::Dying ||
        NewState == EAnim_DinosaurMovementState::Dead)
    {
        return true;
    }
    
    return MovementState != NewState;
}

float UAnim_DinosaurAnimController::CalculateBlendWeight(float CurrentValue, float TargetValue, float BlendSpeed, float DeltaTime) const
{
    return FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, BlendSpeed);
}

void UAnim_DinosaurAnimController::LogAnimationState() const
{
    UE_LOG(LogTemp, Log, TEXT("DinosaurAnimController [%s]: Movement=%d, Behavior=%d, Speed=%.1f, Alert=%s"), 
           OwningCharacter ? *OwningCharacter->GetName() : TEXT("Unknown"),
           (int32)MovementState, 
           (int32)BehaviorState, 
           Speed,
           bIsAlert ? TEXT("Yes") : TEXT("No"));
}