#include "AnimationSystemCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnimationSystemCore::UAnimationSystemCore()
{
}

FMotionMatchingData UAnimationSystemCore::CalculateMotionMatchingData(AActor* Character)
{
    FMotionMatchingData Data;
    
    if (!Character)
    {
        return Data;
    }

    if (ACharacter* CharacterPawn = Cast<ACharacter>(Character))
    {
        UCharacterMovementComponent* MovementComp = CharacterPawn->GetCharacterMovement();
        if (MovementComp)
        {
            // Get current velocity and acceleration
            Data.Velocity = MovementComp->Velocity;
            Data.Acceleration = MovementComp->GetCurrentAcceleration();
            Data.Speed = Data.Velocity.Size();
            
            // Calculate movement direction relative to character facing
            Data.Direction = CalculateMovementDirection(Data.Velocity, Character->GetActorRotation());
            
            // Determine movement states
            Data.bIsMoving = Data.Speed > 10.0f;
            Data.bIsInAir = MovementComp->IsFalling();
        }
    }
    
    return Data;
}

float UAnimationSystemCore::CalculateMovementDirection(const FVector& Velocity, const FRotator& ActorRotation)
{
    if (Velocity.SizeSquared() < 0.01f)
    {
        return 0.0f;
    }

    // Convert velocity to local space
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    
    // Calculate angle in degrees (-180 to 180)
    float Angle = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X);
    return FMath::RadiansToDegrees(Angle);
}

FIKFootData UAnimationSystemCore::CalculateFootIK(USkeletalMeshComponent* SkeletalMesh, FName FootBone, float TraceDistance)
{
    FIKFootData FootData;
    
    if (!SkeletalMesh || !SkeletalMesh->GetWorld())
    {
        return FootData;
    }

    // Get foot bone location in world space
    FVector FootLocation = SkeletalMesh->GetBoneLocation(FootBone);
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, TraceDistance);

    // Perform line trace to find ground
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(SkeletalMesh->GetOwner());

    bool bHit = SkeletalMesh->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        FootData.FootLocation = HitResult.Location;
        FootData.FootRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();
        FootData.IKAlpha = 1.0f;
        FootData.bIsPlanted = true;
    }
    else
    {
        FootData.FootLocation = FootLocation;
        FootData.IKAlpha = 0.0f;
        FootData.bIsPlanted = false;
    }

    return FootData;
}

FVector UAnimationSystemCore::CalculateHipOffset(const FIKFootData& LeftFoot, const FIKFootData& RightFoot)
{
    if (!LeftFoot.bIsPlanted && !RightFoot.bIsPlanted)
    {
        return FVector::ZeroVector;
    }

    float LeftFootZ = LeftFoot.bIsPlanted ? LeftFoot.FootLocation.Z : 0.0f;
    float RightFootZ = RightFoot.bIsPlanted ? RightFoot.FootLocation.Z : 0.0f;
    
    // Calculate the lowest foot position
    float LowestFootZ = FMath::Min(LeftFootZ, RightFootZ);
    
    // Offset hip to maintain natural pose
    return FVector(0, 0, LowestFootZ * 0.5f);
}

bool UAnimationSystemCore::TraceForGround(UWorld* World, const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!World)
    {
        return false;
    }

    FVector TraceStart = StartLocation + FVector(0, 0, 10.0f);
    FVector TraceEnd = StartLocation - FVector(0, 0, IK_TRACE_DISTANCE);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }

    return false;
}

float UAnimationSystemCore::CalculateBlendWeight(float CurrentValue, float TargetValue, float BlendSpeed, float DeltaTime)
{
    return FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, BlendSpeed);
}