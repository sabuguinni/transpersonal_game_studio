#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize thresholds
    WalkThreshold = 10.0f;
    RunThreshold = 200.0f;
    SprintThreshold = 400.0f;
    StateChangeSmoothing = 2.0f;

    // Initialize state
    CurrentMovementState = EAnim_MovementState::Idle;
    SmoothedSpeed = 0.0f;
    SmoothedVelocity = FVector::ZeroVector;

    // Initialize references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandingMontage = nullptr;
    IdleAnimation = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeReferences();
}

void UAnim_MotionMatchingComponent::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching Component: Owner is not a Character"));
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && MovementComponent)
    {
        UpdateMotionData(DeltaTime);
        CurrentMovementState = DetermineMovementState();
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionData(float DeltaTime)
{
    if (!MovementComponent || !OwnerCharacter)
        return;

    // Get raw motion data
    FVector RawVelocity = MovementComponent->Velocity;
    float RawSpeed = RawVelocity.Size();

    // Smooth the motion data
    SmoothMotionData(DeltaTime);

    // Update motion data structure
    CurrentMotionData.Speed = SmoothedSpeed;
    CurrentMotionData.Velocity = SmoothedVelocity;
    
    // Calculate normalized direction
    if (SmoothedSpeed > 1.0f)
    {
        CurrentMotionData.Direction = SmoothedVelocity.GetSafeNormal();
    }
    else
    {
        CurrentMotionData.Direction = OwnerCharacter->GetActorForwardVector();
    }

    // Update air state
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate ground distance
    CalculateGroundDistance();

    // Calculate turn rate
    FVector CurrentForward = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = CurrentMotionData.Direction;
    float DotProduct = FVector::DotProduct(CurrentForward, VelocityDirection);
    CurrentMotionData.TurnRate = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
}

void UAnim_MotionMatchingComponent::SmoothMotionData(float DeltaTime)
{
    if (!MovementComponent)
        return;

    FVector TargetVelocity = MovementComponent->Velocity;
    float TargetSpeed = TargetVelocity.Size();

    // Smooth speed
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, TargetSpeed, DeltaTime, StateChangeSmoothing);

    // Smooth velocity
    SmoothedVelocity = FMath::VInterpTo(SmoothedVelocity, TargetVelocity, DeltaTime, StateChangeSmoothing);
}

void UAnim_MotionMatchingComponent::CalculateGroundDistance()
{
    if (!OwnerCharacter)
    {
        CurrentMotionData.GroundDistance = 0.0f;
        return;
    }

    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 1000.0f); // Trace down 10 meters

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        CurrentMotionData.GroundDistance = FVector::Dist(StartLocation, HitResult.Location);
    }
    else
    {
        CurrentMotionData.GroundDistance = 1000.0f; // Max distance if no ground found
    }
}

EAnim_MovementState UAnim_MotionMatchingComponent::DetermineMovementState()
{
    if (!MovementComponent)
        return EAnim_MovementState::Idle;

    // Check air state first
    if (CurrentMotionData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 50.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else if (MovementComponent->Velocity.Z < -50.0f)
        {
            return EAnim_MovementState::Falling;
        }
    }

    // Check crouching
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }

    // Check movement speed
    float Speed = CurrentMotionData.Speed;
    
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

void UAnim_MotionMatchingComponent::PlayJumpAnimation()
{
    if (!OwnerCharacter || !JumpMontage)
        return;

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Play(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
    }
}

void UAnim_MotionMatchingComponent::PlayLandingAnimation()
{
    if (!OwnerCharacter || !LandingMontage)
        return;

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Play(LandingMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing landing animation"));
    }
}

bool UAnim_MotionMatchingComponent::IsMoving() const
{
    return CurrentMotionData.Speed > WalkThreshold;
}

float UAnim_MotionMatchingComponent::GetNormalizedSpeed() const
{
    if (SprintThreshold <= 0.0f)
        return 0.0f;

    return FMath::Clamp(CurrentMotionData.Speed / SprintThreshold, 0.0f, 1.0f);
}