#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Initialize default values
    AnimationData = FPlayerAnimationData();
    CurrentAnimationState = ECharacterAnimationState::Idle;
    StateTimer = 0.0f;
    
    // IK settings
    bEnableFootIK = true;
    FootIKInterpSpeed = 15.0f;
    MaxFootIKOffset = 20.0f;
    
    // Fear response settings
    FearResponseInterpSpeed = 5.0f;
    MaxFearTrembleAmount = 2.0f;
    
    // Motion Matching
    MotionMatchingBlendTime = 0.3f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
    
    // Create animation system instance
    AnimationSystem = NewObject<UAnimationSystemCore>(this);
}

void UPlayerAnimInstance::NativeBeginPlay()
{
    Super::NativeBeginPlay();
    
    // Initialize animation system
    if (AnimationSystem)
    {
        FAnimationStateData InitialState;
        InitialState.CurrentState = ECharacterAnimationState::Idle;
        InitialState.CautionLevel = 0.5f; // Base caution in prehistoric world
        AnimationSystem->UpdateAnimationState(InitialState);
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementData(DeltaTimeX);
    UpdateCharacterState(DeltaTimeX);
    UpdateEnvironmentData(DeltaTimeX);
    UpdateIKData(DeltaTimeX);
    UpdateMotionMatchingDatabase();
    
    // Update state timer
    StateTimer += DeltaTimeX;
}

void UPlayerAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Basic movement data
    AnimationData.Velocity = MovementComponent->Velocity;
    AnimationData.Speed = AnimationData.Velocity.Size2D();
    AnimationData.bIsMoving = AnimationData.Speed > 3.0f;
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    
    // Calculate movement direction relative to character
    if (AnimationData.bIsMoving && OwningCharacter)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = AnimationData.Velocity.GetSafeNormal2D();
        AnimationData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        if (FVector::DotProduct(RightVector, VelocityNormalized) < 0.0f)
        {
            AnimationData.Direction *= -1.0f;
        }
    }
    else
    {
        AnimationData.Direction = 0.0f;
    }
    
    // Calculate acceleration
    static FVector LastVelocity = FVector::ZeroVector;
    AnimationData.Acceleration = (AnimationData.Velocity - LastVelocity) / DeltaTime;
    LastVelocity = AnimationData.Velocity;
}

void UPlayerAnimInstance::UpdateCharacterState(float DeltaTime)
{
    // Update crouching state
    AnimationData.bIsCrouching = MovementComponent && MovementComponent->IsCrouching();
    
    // Determine current animation state
    ECharacterAnimationState NewState = DetermineAnimationState();
    
    if (NewState != CurrentAnimationState)
    {
        CurrentAnimationState = NewState;
        StateTimer = 0.0f;
        
        // Update animation system
        if (AnimationSystem)
        {
            FAnimationStateData StateData;
            StateData.CurrentState = CurrentAnimationState;
            StateData.MovementSpeed = AnimationData.Speed;
            StateData.FearLevel = AnimationData.FearLevel;
            StateData.CautionLevel = AnimationData.CautionLevel;
            StateData.CurrentTerrain = AnimationData.CurrentTerrain;
            StateData.bIsHidden = AnimationData.bIsHiding;
            StateData.bIsObservingDinosaur = AnimationData.bNearDinosaur;
            
            AnimationSystem->UpdateAnimationState(StateData);
        }
    }
    
    // Apply fear response
    ApplyFearResponse(DeltaTime);
}

void UPlayerAnimInstance::UpdateEnvironmentData(float DeltaTime)
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Ground slope calculation
    FVector GroundNormal = FVector::UpVector;
    if (MovementComponent && MovementComponent->IsMovingOnGround())
    {
        FHitResult HitResult;
        FVector StartLocation = OwningCharacter->GetActorLocation();
        FVector EndLocation = StartLocation - FVector(0, 0, 100);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwningCharacter);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
        {
            GroundNormal = HitResult.Normal;
        }
    }
    
    AnimationData.GroundSlope = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
    
    // Determine terrain type based on slope and surface
    if (AnimationData.GroundSlope > 25.0f)
    {
        AnimationData.CurrentTerrain = (GroundNormal.Z > FVector::UpVector.Z) ? ETerrainType::Uphill : ETerrainType::Downhill;
    }
    else if (AnimationData.GroundSlope > 15.0f)
    {
        AnimationData.CurrentTerrain = ETerrainType::Rocky;
    }
    else
    {
        AnimationData.CurrentTerrain = ETerrainType::Flat;
    }
    
    // TODO: Implement dinosaur proximity detection
    // This will be connected to the NPC Behavior Agent's output
    AnimationData.bNearDinosaur = false;
    AnimationData.DinosaurThreatLevel = 0.0f;
}

void UPlayerAnimInstance::UpdateIKData(float DeltaTime)
{
    if (!bEnableFootIK || !OwningCharacter)
    {
        return;
    }
    
    // Calculate IK offsets for both feet
    FVector NewLeftFootOffset = CalculateFootIKOffset(TEXT("foot_l"), DeltaTime);
    FVector NewRightFootOffset = CalculateFootIKOffset(TEXT("foot_r"), DeltaTime);
    
    // Interpolate to smooth IK movement
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, NewLeftFootOffset, DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, NewRightFootOffset, DeltaTime, FootIKInterpSpeed);
    
    // Calculate foot rotations
    LeftFootIKRotation = CalculateFootIKRotation(TEXT("foot_l"), LeftFootIKOffset);
    RightFootIKRotation = CalculateFootIKRotation(TEXT("foot_r"), RightFootIKOffset);
}

void UPlayerAnimInstance::UpdateMotionMatchingDatabase()
{
    if (!AnimationSystem)
    {
        return;
    }
    
    // Get the appropriate database from animation system
    CurrentDatabase = AnimationSystem->GetActiveDatabase();
    
    // Calculate blend time based on state transition
    if (AnimationSystem)
    {
        MotionMatchingBlendTime = AnimationSystem->CalculateBlendTime(
            CurrentAnimationState, 
            AnimationSystem->CurrentAnimationState.CurrentState
        );
    }
}

ECharacterAnimationState UPlayerAnimInstance::DetermineAnimationState() const
{
    // Priority 1: Fear-based states
    if (AnimationData.FearLevel > 0.8f && AnimationData.bIsMoving)
    {
        return ECharacterAnimationState::Running;
    }
    
    if (AnimationData.bIsHiding)
    {
        return ECharacterAnimationState::Hiding;
    }
    
    if (AnimationData.bNearDinosaur && !AnimationData.bIsMoving)
    {
        return ECharacterAnimationState::Observing;
    }
    
    // Priority 2: Movement states
    if (AnimationData.bIsCrouching && AnimationData.bIsMoving)
    {
        return ECharacterAnimationState::Sneaking;
    }
    
    if (AnimationData.bIsMoving)
    {
        if (AnimationData.Speed > 400.0f) // Running threshold
        {
            return (AnimationData.FearLevel > 0.5f) ? ECharacterAnimationState::Running : ECharacterAnimationState::Jogging;
        }
        else if (AnimationData.Speed > 150.0f) // Walking threshold
        {
            return (AnimationData.CautionLevel > 0.7f) ? ECharacterAnimationState::Cautious : ECharacterAnimationState::Walking;
        }
        else // Slow movement
        {
            return ECharacterAnimationState::Cautious;
        }
    }
    
    // Priority 3: Idle states
    if (AnimationData.CautionLevel > 0.8f || AnimationData.FearLevel > 0.3f)
    {
        return ECharacterAnimationState::Cautious;
    }
    
    return ECharacterAnimationState::Idle;
}

FVector UPlayerAnimInstance::CalculateFootIKOffset(const FName& SocketName, float DeltaTime)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot socket location
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartTrace = SocketLocation + FVector(0, 0, IK_FOOT_HEIGHT);
    FVector EndTrace = SocketLocation - FVector(0, 0, IK_TRACE_DISTANCE);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
    {
        float IKOffset = (HitResult.Location.Z - SocketLocation.Z) + IK_FOOT_HEIGHT;
        IKOffset = FMath::Clamp(IKOffset, -MaxFootIKOffset, MaxFootIKOffset);
        
        return FVector(0, 0, IKOffset);
    }
    
    return FVector::ZeroVector;
}

FRotator UPlayerAnimInstance::CalculateFootIKRotation(const FName& SocketName, const FVector& IKOffset)
{
    // TODO: Implement foot rotation based on ground normal
    // This will rotate the foot to match the ground angle
    return FRotator::ZeroRotator;
}

float UPlayerAnimInstance::CalculateFearTrembleAmount() const
{
    // Calculate trembling based on fear level
    float TrembleAmount = AnimationData.FearLevel * MaxFearTrembleAmount;
    
    // Add some randomness for natural trembling
    TrembleAmount *= (1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) * 0.3f);
    
    return TrembleAmount;
}

void UPlayerAnimInstance::ApplyFearResponse(float DeltaTime)
{
    // Decay fear over time when no immediate threat
    if (AnimationData.DinosaurThreatLevel < 0.1f)
    {
        AnimationData.FearLevel = FMath::FInterpTo(AnimationData.FearLevel, 0.0f, DeltaTime, FearDecayRate);
    }
    
    // Caution level also gradually returns to baseline
    float BaseCautionLevel = 0.5f; // Always somewhat cautious in prehistoric world
    AnimationData.CautionLevel = FMath::FInterpTo(AnimationData.CautionLevel, BaseCautionLevel, DeltaTime, 0.2f);
}