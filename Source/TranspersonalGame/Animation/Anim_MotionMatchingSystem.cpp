#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
    
    // Set default thresholds for prehistoric human movement
    WalkSpeedThreshold = 120.0f;     // Slow tribal walk
    RunSpeedThreshold = 280.0f;      // Normal running
    SprintSpeedThreshold = 450.0f;   // Emergency sprint
    StateTransitionSmoothness = 0.15f; // Quick transitions for survival
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("MotionMatchingSystem: Owner is not a Character!"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem initialized for: %s"), *OwnerCharacter->GetName());
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
        return;
    
    // Store previous frame data
    PreviousMotionData = CurrentMotionData;
    
    // Update motion data for this frame
    UpdateMotionData(DeltaTime);
    
    // Update movement state based on new data
    UpdateMovementState();
    
    // Debug visualization in development builds
    #if WITH_EDITOR
    if (GEngine && GEngine->bIsPlayInEditorWorld)
    {
        FString DebugString = FString::Printf(TEXT("State: %s | Speed: %.1f | Dir: %.1f"), 
            *UEnum::GetValueAsString(CurrentMotionData.CurrentState),
            CurrentMotionData.Speed,
            CurrentMotionData.Direction);
        
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugString);
    }
    #endif
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
        return;
    
    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    
    // Update basic motion data
    CurrentMotionData.Velocity = Movement->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.bIsInAir = Movement->IsFalling();
    
    // Calculate movement direction relative to character facing
    CurrentMotionData.Direction = CalculateDirection();
    
    // Perform ground distance trace
    PerformGroundTrace();
    
    // Update state transition timing
    if (CurrentMotionData.CurrentState == PreviousMotionData.CurrentState)
    {
        CurrentMotionData.StateTransitionTime += DeltaTime;
    }
    else
    {
        CurrentMotionData.StateTransitionTime = 0.0f;
    }
}

void UAnim_MotionMatchingSystem::UpdateMovementState()
{
    EAnim_MovementState NewState = DetermineMovementState();
    
    if (NewState != CurrentMotionData.CurrentState)
    {
        HandleStateTransition(NewState);
    }
}

EAnim_MovementState UAnim_MotionMatchingSystem::DetermineMovementState() const
{
    if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
        return EAnim_MovementState::Idle;
    
    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    
    // Air states have priority
    if (Movement->IsFalling())
    {
        // Distinguish between jumping up and falling down
        if (CurrentMotionData.Velocity.Z > 100.0f)
            return EAnim_MovementState::Jumping;
        else
            return EAnim_MovementState::Falling;
    }
    
    // Just landed
    if (PreviousMotionData.bIsInAir && !CurrentMotionData.bIsInAir)
    {
        return EAnim_MovementState::Landing;
    }
    
    // Crouching
    if (Movement->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Swimming
    if (Movement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Ground movement based on speed
    float Speed = CurrentMotionData.Speed;
    
    if (Speed < 10.0f) // Nearly stationary
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UAnim_MotionMatchingSystem::HandleStateTransition(EAnim_MovementState NewState)
{
    EAnim_MovementState OldState = CurrentMotionData.CurrentState;
    CurrentMotionData.CurrentState = NewState;
    
    // Trigger specific animations for certain transitions
    switch (NewState)
    {
        case EAnim_MovementState::Jumping:
            PlayJumpAnimation();
            break;
            
        case EAnim_MovementState::Landing:
            PlayLandingAnimation();
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: State transition %s -> %s"), 
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(NewState));
}

float UAnim_MotionMatchingSystem::CalculateDirection() const
{
    if (!OwnerCharacter || CurrentMotionData.Speed < 10.0f)
        return 0.0f;
    
    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Get movement direction (normalized velocity)
    FVector MovementDirection = CurrentMotionData.Velocity.GetSafeNormal();
    
    // Calculate angle between forward and movement direction
    float DotProduct = FVector::DotProduct(ForwardVector, MovementDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, MovementDirection).Z;
    
    // Convert to degrees (-180 to 180)
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    
    return Angle;
}

void UAnim_MotionMatchingSystem::PerformGroundTrace()
{
    if (!OwnerCharacter)
        return;
    
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
        CurrentMotionData.DistanceToGround = FVector::Dist(StartLocation, HitResult.Location);
    }
    else
    {
        CurrentMotionData.DistanceToGround = 1000.0f; // Max trace distance
    }
}

void UAnim_MotionMatchingSystem::SetLocomotionBlendSpace(UBlendSpace* NewBlendSpace)
{
    LocomotionBlendSpace = NewBlendSpace;
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Locomotion BlendSpace updated"));
}

void UAnim_MotionMatchingSystem::PlayJumpAnimation()
{
    if (!OwnerCharacter || !JumpMontage)
        return;
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Play(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Playing jump animation"));
    }
}

void UAnim_MotionMatchingSystem::PlayLandingAnimation()
{
    if (!OwnerCharacter || !LandingMontage)
        return;
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Play(LandingMontage);
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Playing landing animation"));
    }
}

bool UAnim_MotionMatchingSystem::IsTransitioningStates() const
{
    return CurrentMotionData.StateTransitionTime < StateTransitionSmoothness;
}