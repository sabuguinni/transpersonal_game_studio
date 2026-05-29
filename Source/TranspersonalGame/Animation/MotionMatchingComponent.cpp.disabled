#include "MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UMotionMatchingComponent::UMotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    FootIKTraceDistance = 50.0f;
    TerrainAnalysisRadius = 100.0f;
    SlopeThreshold = 15.0f;
    IKInterpSpeed = 10.0f;

    LastVelocity = FVector::ZeroVector;
    LastUpdateTime = 0.0f;
    bInitialized = false;

    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
    MotionDatabase = nullptr;
}

void UMotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UMotionMatchingComponent::InitializeComponent()
{
    CacheReferences();
    
    if (OwnerCharacter)
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent initialized for %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent failed to initialize - no character owner"));
    }
}

void UMotionMatchingComponent::CacheReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh();
        if (SkeletalMesh)
        {
            AnimInstance = SkeletalMesh->GetAnimInstance();
        }
    }
}

void UMotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bInitialized || !OwnerCharacter)
    {
        return;
    }

    UpdateMotionMatchingState(DeltaTime);
    UpdateTerrainAdaptation(DeltaTime);
    PerformFootIK(DeltaTime);
}

void UMotionMatchingComponent::UpdateMotionMatchingState(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        return;
    }

    // Update basic movement state
    CurrentState.Velocity = MovementComponent->Velocity;
    CurrentState.Speed = CalculateMovementSpeed();
    CurrentState.Direction = CalculateMovementDirection();
    CurrentState.bIsInAir = MovementComponent->IsFalling();
    CurrentState.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate ground distance for landing prediction
    if (CurrentState.bIsInAir)
    {
        FVector Start = OwnerCharacter->GetActorLocation();
        FVector End = Start - FVector(0, 0, 1000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerCharacter);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
        {
            CurrentState.GroundDistance = HitResult.Distance;
        }
        else
        {
            CurrentState.GroundDistance = 1000.0f;
        }
    }
    else
    {
        CurrentState.GroundDistance = 0.0f;
    }

    LastVelocity = CurrentState.Velocity;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UMotionMatchingComponent::UpdateTerrainAdaptation(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }

    AnalyzeTerrain();
    
    // Update terrain state
    TerrainState.bOnUnevenTerrain = IsOnUnevenTerrain();
    
    // Calculate slope angle
    FVector UpVector = FVector::UpVector;
    float DotProduct = FVector::DotProduct(TerrainState.SurfaceNormal, UpVector);
    TerrainState.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
}

float UMotionMatchingComponent::CalculateMovementSpeed() const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }

    return CurrentState.Velocity.Size();
}

float UMotionMatchingComponent::CalculateMovementDirection() const
{
    if (!OwnerCharacter || CurrentState.Speed < 1.0f)
    {
        return 0.0f;
    }

    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = CurrentState.Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct);
}

bool UMotionMatchingComponent::IsOnUnevenTerrain() const
{
    return TerrainState.SlopeAngle > SlopeThreshold;
}

void UMotionMatchingComponent::PerformFootIK(float DeltaTime)
{
    if (!OwnerCharacter || CurrentState.bIsInAir)
    {
        return;
    }

    // Get foot bone locations (approximated from character mesh)
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();

    // Approximate foot positions
    FVector LeftFootLocation = CharacterLocation - (RightVector * 15.0f);
    FVector RightFootLocation = CharacterLocation + (RightVector * 15.0f);

    // Trace for foot IK
    float LeftFootOffset = TraceForFootIK(LeftFootLocation, -FVector::UpVector);
    float RightFootOffset = TraceForFootIK(RightFootLocation, -FVector::UpVector);

    // Smooth interpolation
    TerrainState.FootIKLeftOffset = FMath::FInterpTo(
        TerrainState.FootIKLeftOffset, 
        LeftFootOffset, 
        DeltaTime, 
        IKInterpSpeed
    );
    
    TerrainState.FootIKRightOffset = FMath::FInterpTo(
        TerrainState.FootIKRightOffset, 
        RightFootOffset, 
        DeltaTime, 
        IKInterpSpeed
    );
}

float UMotionMatchingComponent::TraceForFootIK(const FVector& FootLocation, const FVector& TraceDirection) const
{
    if (!GetWorld())
    {
        return 0.0f;
    }

    FVector Start = FootLocation + (FVector::UpVector * 20.0f);
    FVector End = Start + (TraceDirection * FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        float DistanceFromGround = HitResult.Distance - 20.0f;
        return -DistanceFromGround; // Negative for foot lowering, positive for raising
    }

    return 0.0f;
}

void UMotionMatchingComponent::AnalyzeTerrain()
{
    if (!OwnerCharacter)
    {
        return;
    }

    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    TerrainState.SurfaceNormal = GetSurfaceNormal(CharacterLocation);
}

FVector UMotionMatchingComponent::GetSurfaceNormal(const FVector& Location) const
{
    if (!GetWorld())
    {
        return FVector::UpVector;
    }

    FVector Start = Location + (FVector::UpVector * 50.0f);
    FVector End = Start - (FVector::UpVector * 200.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Normal;
    }

    return FVector::UpVector;
}