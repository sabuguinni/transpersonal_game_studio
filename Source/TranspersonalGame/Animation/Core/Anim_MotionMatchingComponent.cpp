#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    BlendTime = 0.3f;
    MinMovementThreshold = 5.0f;
    MaxSearchDistance = 200.0f;
    
    PreviousMotionState = EAnim_MotionState::Idle;
    TimeSinceStateChange = 0.0f;
    MovementComponent = nullptr;
    MotionDatabase = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMotionMatching();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceStateChange += DeltaTime;

    if (MovementComponent)
    {
        // Update motion query with current movement data
        FVector CurrentVelocity = MovementComponent->Velocity;
        FVector DesiredDirection = MovementComponent->GetLastInputVector();
        
        // Calculate motion state based on current conditions
        EAnim_MotionState NewMotionState = CalculateMotionState();
        
        // Update terrain analysis
        UpdateTerrainAnalysis();
        
        // Update the motion query
        UpdateMotionQuery(CurrentVelocity, DesiredDirection, NewMotionState);
        
        // Check for state changes
        if (NewMotionState != PreviousMotionState)
        {
            SetMotionState(NewMotionState);
        }
    }
}

void UAnim_MotionMatchingComponent::InitializeMotionMatching()
{
    // Get the character movement component
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->GetCharacterMovement();
        
        if (MovementComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized for character: %s"), *Character->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No Character Movement Component found on: %s"), *Character->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching Component must be attached to a Character"));
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionQuery(const FVector& InVelocity, const FVector& InDesiredDirection, EAnim_MotionState InMotionState)
{
    CurrentQuery.Velocity = InVelocity;
    CurrentQuery.DesiredDirection = InDesiredDirection.GetSafeNormal();
    CurrentQuery.MotionState = InMotionState;
}

bool UAnim_MotionMatchingComponent::FindBestPoseMatch(float& OutPoseTime, int32& OutAnimationIndex)
{
    if (!MotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Motion Database assigned to Motion Matching Component"));
        return false;
    }

    // This would interface with UE5's Pose Search system
    // For now, we return default values and log the query
    OutPoseTime = 0.0f;
    OutAnimationIndex = 0;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Motion Query - State: %d, Speed: %.2f, Terrain Slope: %.2f"), 
           (int32)CurrentQuery.MotionState, 
           CurrentQuery.Velocity.Size(), 
           CurrentQuery.TerrainSlope);
    
    return true;
}

void UAnim_MotionMatchingComponent::SetMotionState(EAnim_MotionState NewState)
{
    EAnim_MotionState OldState = PreviousMotionState;
    PreviousMotionState = NewState;
    TimeSinceStateChange = 0.0f;
    
    // Trigger Blueprint event
    OnMotionStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Motion State Changed: %d -> %d"), (int32)OldState, (int32)NewState);
}

bool UAnim_MotionMatchingComponent::IsInTransition() const
{
    return TimeSinceStateChange < BlendTime;
}

float UAnim_MotionMatchingComponent::GetCurrentSpeed() const
{
    return CurrentQuery.Velocity.Size();
}

FVector UAnim_MotionMatchingComponent::GetMovementDirection() const
{
    return CurrentQuery.DesiredDirection;
}

void UAnim_MotionMatchingComponent::UpdateTerrainAnalysis()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    // Perform a line trace downward to analyze terrain
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate terrain slope
        FVector SurfaceNormal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
        
        CurrentQuery.TerrainSlope = SlopeAngle;
        CurrentQuery.bIsOnUnevenTerrain = SlopeAngle > 15.0f; // Consider slopes > 15 degrees as uneven
        
        // Debug visualization
        if (CVarShowDebugAnimation.GetValueOnGameThread())
        {
            DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Green, false, 0.1f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Red, false, 0.1f);
        }
    }
    else
    {
        CurrentQuery.TerrainSlope = 0.0f;
        CurrentQuery.bIsOnUnevenTerrain = false;
    }
}

EAnim_MotionState UAnim_MotionMatchingComponent::CalculateMotionState() const
{
    if (!MovementComponent)
    {
        return EAnim_MotionState::Idle;
    }

    float CurrentSpeed = GetCurrentSpeed();
    
    // Determine state based on speed and movement conditions
    if (CurrentSpeed < MinMovementThreshold)
    {
        return EAnim_MotionState::Idle;
    }
    else if (MovementComponent->IsCrouching())
    {
        return EAnim_MotionState::Crouching;
    }
    else if (MovementComponent->IsFalling())
    {
        return EAnim_MotionState::Jumping;
    }
    else if (CurrentSpeed > 400.0f) // Sprint threshold
    {
        return EAnim_MotionState::Sprinting;
    }
    else if (CurrentSpeed > 200.0f) // Run threshold
    {
        return EAnim_MotionState::Running;
    }
    else
    {
        return EAnim_MotionState::Walking;
    }
}