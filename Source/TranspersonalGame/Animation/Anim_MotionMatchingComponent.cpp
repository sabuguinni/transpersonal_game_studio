#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default values
    MotionSmoothingSpeed = 10.0f;
    DirectionSmoothingSpeed = 15.0f;
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;

    // Initialize IK offsets
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    PelvisIKOffset = 0.0f;

    // Initialize cached references
    OwnerCharacter = nullptr;
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
    SetupBlendSpaces();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && AnimInstance)
    {
        // Calculate and update motion data
        FAnim_MotionData NewMotionData = CalculateMotionData();
        SmoothMotionData(NewMotionData, DeltaTime);
        UpdateAnimationState(NewMotionData);

        // Update foot IK if enabled
        if (bEnableFootIK)
        {
            UpdateFootIK();
            ApplyFootIK(DeltaTime);
        }

        // Store previous frame data
        PreviousMotionData = CurrentMotionData;
        CurrentMotionData = NewMotionData;
    }
}

FAnim_MotionData UAnim_MotionMatchingComponent::CalculateMotionData()
{
    FAnim_MotionData MotionData;

    if (!OwnerCharacter)
    {
        return MotionData;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return MotionData;
    }

    // Calculate speed
    FVector Velocity = MovementComp->Velocity;
    MotionData.Speed = Velocity.Size2D();

    // Calculate direction relative to character forward
    if (MotionData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        MotionData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        MotionData.Direction = 0.0f;
    }

    // Check if character is in air
    MotionData.bIsInAir = MovementComp->IsFalling();

    // Check if character is crouching
    MotionData.bIsCrouching = MovementComp->IsCrouching();

    // Calculate turn rate
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    if (PreviousMotionData.Speed > 0.0f) // Only calculate turn rate when moving
    {
        float DeltaYaw = FMath::FindDeltaAngleDegrees(PreviousMotionData.Direction, MotionData.Direction);
        MotionData.TurnRate = DeltaYaw / GetWorld()->GetDeltaSeconds();
    }
    else
    {
        MotionData.TurnRate = 0.0f;
    }

    return MotionData;
}

void UAnim_MotionMatchingComponent::SmoothMotionData(FAnim_MotionData& MotionData, float DeltaTime)
{
    // Smooth speed changes
    MotionData.Speed = FMath::FInterpTo(CurrentMotionData.Speed, MotionData.Speed, DeltaTime, MotionSmoothingSpeed);

    // Smooth direction changes
    MotionData.Direction = FMath::FInterpAngle(CurrentMotionData.Direction, MotionData.Direction, DeltaTime, DirectionSmoothingSpeed);

    // Smooth turn rate
    MotionData.TurnRate = FMath::FInterpTo(CurrentMotionData.TurnRate, MotionData.TurnRate, DeltaTime, DirectionSmoothingSpeed);
}

void UAnim_MotionMatchingComponent::UpdateAnimationState(const FAnim_MotionData& MotionData)
{
    if (!AnimInstance)
    {
        return;
    }

    // Set animation variables that can be read by Animation Blueprint
    AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);

    // These would typically be set via Animation Blueprint variables
    // For now, we log the motion data for debugging
    UE_LOG(LogTemp, VeryVerbose, TEXT("Motion Data - Speed: %f, Direction: %f, InAir: %s, Crouching: %s, TurnRate: %f"),
        MotionData.Speed,
        MotionData.Direction,
        MotionData.bIsInAir ? TEXT("true") : TEXT("false"),
        MotionData.bIsCrouching ? TEXT("true") : TEXT("false"),
        MotionData.TurnRate
    );
}

void UAnim_MotionMatchingComponent::SetupBlendSpaces()
{
    // This would typically load blend space assets from content
    // For now, we initialize the structure
    BlendSpaceData = FAnim_BlendSpaceData();
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching Component: Blend spaces setup complete"));
}

void UAnim_MotionMatchingComponent::TriggerJumpAnimation()
{
    if (AnimInstance && BlendSpaceData.JumpStartAnimation)
    {
        // Play jump start animation
        AnimInstance->Montage_Play(nullptr, 1.0f); // Would use actual jump montage
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component: Jump animation triggered"));
    }
}

void UAnim_MotionMatchingComponent::EndJumpAnimation()
{
    if (AnimInstance && BlendSpaceData.JumpEndAnimation)
    {
        // Play jump end animation
        AnimInstance->Montage_Play(nullptr, 1.0f); // Would use actual land montage
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component: Jump end animation triggered"));
    }
}

void UAnim_MotionMatchingComponent::UpdateFootIK()
{
    if (!SkeletalMeshComponent || !bEnableFootIK)
    {
        return;
    }

    // Calculate foot IK offsets
    LeftFootIKOffset = CalculateFootIKOffset(TEXT("foot_l"));
    RightFootIKOffset = CalculateFootIKOffset(TEXT("foot_r"));

    // Calculate pelvis offset to keep character grounded
    float LeftOffset = FMath::Min(LeftFootIKOffset, 0.0f);
    float RightOffset = FMath::Min(RightFootIKOffset, 0.0f);
    PelvisIKOffset = FMath::Min(LeftOffset, RightOffset);
}

float UAnim_MotionMatchingComponent::CalculateFootIKOffset(const FName& SocketName)
{
    if (!SkeletalMeshComponent || !OwnerCharacter)
    {
        return 0.0f;
    }

    // Get foot socket location
    FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
    
    // Trace down from foot to find ground
    FVector TraceStart = SocketLocation;
    FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        float DistanceToGround = (TraceStart - HitResult.Location).Z;
        return FootIKTraceDistance - DistanceToGround;
    }

    return 0.0f;
}

void UAnim_MotionMatchingComponent::ApplyFootIK(float DeltaTime)
{
    // Interpolate IK offsets for smooth transitions
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, CalculateFootIKOffset(TEXT("foot_l")), DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, CalculateFootIKOffset(TEXT("foot_r")), DeltaTime, FootIKInterpSpeed);

    // These values would typically be passed to the Animation Blueprint
    // for actual bone manipulation
    UE_LOG(LogTemp, VeryVerbose, TEXT("Foot IK - Left: %f, Right: %f, Pelvis: %f"),
        LeftFootIKOffset, RightFootIKOffset, PelvisIKOffset);
}

void UAnim_MotionMatchingComponent::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component: Playing animation montage"));
    }
}

void UAnim_MotionMatchingComponent::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Motion Matching Component: Components cached - Character: %s, Mesh: %s, AnimInstance: %s"),
        OwnerCharacter ? TEXT("Valid") : TEXT("Invalid"),
        SkeletalMeshComponent ? TEXT("Valid") : TEXT("Invalid"),
        AnimInstance ? TEXT("Valid") : TEXT("Invalid")
    );
}