#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    StateTransitionSpeed = 5.0f;
    BlendSmoothingSpeed = 8.0f;
    MinMovementThreshold = 10.0f;
    MaxTerrainAdaptationAngle = 45.0f;
    
    TerrainSlope = 0.0f;
    StateTransitionTimer = 0.0f;
    bPendingStateChange = false;
    PendingMovementState = EAnim_MovementState::Idle;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateCharacterReferences();
    InitializeBlendWeights();
    
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching Component initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMotionData(DeltaTime);
    UpdateBlendWeights(DeltaTime);
    AdaptToTerrain();
    
    // Handle pending state changes
    if (bPendingStateChange)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= (1.0f / StateTransitionSpeed))
        {
            CurrentMovementState = PendingMovementState;
            bPendingStateChange = false;
            StateTransitionTimer = 0.0f;
        }
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Store previous frame data
    PreviousMotionData = CurrentMotionData;
    
    // Update current motion data
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.bIsOnGround = MovementComponent->IsMovingOnGround();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > MinMovementThreshold;
    
    // Calculate movement direction relative to character forward
    if (CurrentMotionData.bIsMoving && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::Atan2(
            FVector::DotProduct(OwnerCharacter->GetActorRightVector(), VelocityNormalized),
            FVector::DotProduct(ForwardVector, VelocityNormalized)
        ) * 180.0f / PI;
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Update time since last movement
    if (CurrentMotionData.bIsMoving)
    {
        CurrentMotionData.TimeSinceLastMove = 0.0f;
    }
    else
    {
        CurrentMotionData.TimeSinceLastMove += DeltaTime;
    }
    
    // Auto-update movement state based on motion data
    EAnim_MovementState NewState = CalculateMovementState();
    if (NewState != CurrentMovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_MotionMatchingComponent::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState != CurrentMovementState && !bPendingStateChange)
    {
        PreviousMovementState = CurrentMovementState;
        PendingMovementState = NewState;
        bPendingStateChange = true;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Transitioning from %d to %d"), 
               (int32)CurrentMovementState, (int32)NewState);
    }
}

void UAnim_MotionMatchingComponent::SetActionState(EAnim_ActionState NewState)
{
    if (NewState != CurrentActionState)
    {
        CurrentActionState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Action state changed to %d"), (int32)NewState);
    }
}

float UAnim_MotionMatchingComponent::GetBlendWeight(EAnim_MovementState State) const
{
    const float* Weight = BlendWeights.Find(State);
    return Weight ? *Weight : 0.0f;
}

void UAnim_MotionMatchingComponent::UpdateBlendWeights(float DeltaTime)
{
    // Update blend weights for smooth transitions
    for (auto& WeightPair : BlendWeights)
    {
        EAnim_MovementState State = WeightPair.Key;
        float& CurrentWeight = WeightPair.Value;
        
        float TargetWeight = 0.0f;
        if (State == CurrentMovementState)
        {
            TargetWeight = 1.0f;
        }
        else if (bPendingStateChange && State == PendingMovementState)
        {
            TargetWeight = StateTransitionTimer * StateTransitionSpeed;
        }
        else if (bPendingStateChange && State == PreviousMovementState)
        {
            TargetWeight = 1.0f - (StateTransitionTimer * StateTransitionSpeed);
        }
        
        CurrentWeight = CalculateBlendAlpha(CurrentWeight, TargetWeight, DeltaTime);
    }
}

void UAnim_MotionMatchingComponent::AdaptToTerrain()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    PerformTerrainTrace();
}

void UAnim_MotionMatchingComponent::UpdateCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_MotionMatchingComponent::InitializeBlendWeights()
{
    BlendWeights.Empty();
    BlendWeights.Add(EAnim_MovementState::Idle, 1.0f);
    BlendWeights.Add(EAnim_MovementState::Walking, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Running, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Sprinting, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Jumping, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Falling, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Landing, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Crouching, 0.0f);
    BlendWeights.Add(EAnim_MovementState::Crawling, 0.0f);
}

EAnim_MovementState UAnim_MotionMatchingComponent::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for airborne states first
    if (!CurrentMotionData.bIsOnGround)
    {
        if (CurrentMotionData.Velocity.Z > 50.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Ground-based movement states
    if (!CurrentMotionData.bIsMoving)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for crouching
    if (MovementComponent->IsCrouching())
    {
        return CurrentMotionData.Speed > 50.0f ? EAnim_MovementState::Crawling : EAnim_MovementState::Crouching;
    }
    
    // Speed-based states
    if (CurrentMotionData.Speed < 150.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else if (CurrentMotionData.Speed < 400.0f)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UAnim_MotionMatchingComponent::PerformTerrainTrace()
{
    if (!OwnerCharacter || !GetWorld())
    {
        return;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        FVector SurfaceNormal = HitResult.Normal;
        FVector UpVector = FVector::UpVector;
        
        float DotProduct = FVector::DotProduct(SurfaceNormal, UpVector);
        TerrainSlope = FMath::Acos(DotProduct) * 180.0f / PI;
        
        // Clamp terrain slope to max adaptation angle
        TerrainSlope = FMath::Clamp(TerrainSlope, 0.0f, MaxTerrainAdaptationAngle);
    }
    else
    {
        TerrainSlope = 0.0f;
    }
}

float UAnim_MotionMatchingComponent::CalculateBlendAlpha(float CurrentValue, float TargetValue, float DeltaTime) const
{
    return FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, BlendSmoothingSpeed);
}