#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Initialize variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bIsFalling = false;
    bIsJumping = false;
    
    TensionLevel = 0.0f;
    FearLevel = 0.0f;
    bIsBeingHunted = false;
    CautionMultiplier = 1.0f;
    MovementHesitation = 0.0f;
    
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    HipOffset = 0.0f;
    
    LookAtAlpha = 0.0f;
    bShouldLookAround = false;
    HeadTurnSpeed = 2.0f;
    LookAroundTimer = 0.0f;
    NextLookTime = 2.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache references
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        
        // Find Animation System Manager
        AnimationManager = OwnerCharacter->FindComponentByClass<UAnimationSystemManager>();
        if (!AnimationManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerAnimInstance: No Animation System Manager found on character"));
        }
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementVariables();
    UpdateEmotionalState();
    UpdateLookAtBehavior();
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaTimeX);
    
    // Thread-safe IK calculations
    UpdateIKVariables();
}

void UPlayerAnimInstance::UpdateMovementVariables()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Basic movement variables
    Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    bIsMoving = Speed > 3.0f;
    bIsFalling = MovementComponent->IsFalling();
    bIsJumping = bIsFalling && Velocity.Z > 0.0f;
    
    // Calculate acceleration for Motion Matching
    Acceleration = (Velocity - PreviousVelocity) / GetDeltaSeconds();
    PreviousVelocity = Velocity;
    
    // Calculate movement direction relative to character
    if (bIsMoving && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
}

void UPlayerAnimInstance::UpdateEmotionalState()
{
    if (!AnimationManager)
    {
        return;
    }
    
    // Get emotional state from Animation Manager
    TensionLevel = AnimationManager->CurrentTensionLevel;
    FearLevel = AnimationManager->CurrentFearLevel;
    bIsBeingHunted = AnimationManager->bIsBeingHunted;
    
    // Calculate caution multiplier (affects movement speed and hesitation)
    CautionMultiplier = FMath::Lerp(1.0f, 0.6f, TensionLevel);
    
    // Movement hesitation increases with fear
    MovementHesitation = FearLevel * 0.3f;
    
    // Update active Motion Matching database
    CurrentDatabase = AnimationManager->GetActivePlayerDatabase();
    
    // Determine if character should look around nervously
    bShouldLookAround = (TensionLevel > 0.3f || FearLevel > 0.4f);
    HeadTurnSpeed = FMath::Lerp(1.0f, 4.0f, FearLevel);
}

void UPlayerAnimInstance::UpdateIKVariables()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Calculate foot IK for terrain adaptation
    CalculateFootIK(TEXT("foot_l"), LeftFootIKLocation, LeftFootIKRotation, LeftFootIKAlpha);
    CalculateFootIK(TEXT("foot_r"), RightFootIKLocation, RightFootIKRotation, RightFootIKAlpha);
    
    // Calculate hip offset to maintain natural pose
    float LeftOffset = LeftFootIKLocation.Z;
    float RightOffset = RightFootIKLocation.Z;
    float MaxOffset = FMath::Max(LeftOffset, RightOffset);
    
    // Only lower hips, never raise them
    HipOffset = FMath::Min(0.0f, MaxOffset * 0.5f);
}

void UPlayerAnimInstance::UpdateLookAtBehavior()
{
    if (!bShouldLookAround)
    {
        LookAtAlpha = FMath::FInterpTo(LookAtAlpha, 0.0f, GetDeltaSeconds(), 2.0f);
        return;
    }
    
    LookAroundTimer += GetDeltaSeconds();
    
    // Time to look at a new target
    if (LookAroundTimer >= NextLookTime)
    {
        // Generate random look target around the character
        float RandomAngle = FMath::RandRange(-180.0f, 180.0f);
        float RandomPitch = FMath::RandRange(-20.0f, 30.0f);
        float LookDistance = FMath::RandRange(500.0f, 1500.0f);
        
        if (OwnerCharacter)
        {
            FVector CharacterLocation = OwnerCharacter->GetActorLocation();
            FRotator LookRotation = FRotator(RandomPitch, RandomAngle, 0.0f);
            FVector LookDirection = LookRotation.Vector();
            
            CurrentLookTarget = CharacterLocation + (LookDirection * LookDistance);
        }
        
        // Reset timer with some randomness
        LookAroundTimer = 0.0f;
        NextLookTime = FMath::RandRange(1.0f, 4.0f) / FMath::Max(FearLevel, 0.3f);
        
        LookAtAlpha = 1.0f;
    }
    
    // Smoothly interpolate to look target
    LookAtTarget = FMath::VInterpTo(LookAtTarget, CurrentLookTarget, GetDeltaSeconds(), HeadTurnSpeed);
    
    // Reduce look intensity over time
    if (LookAroundTimer > NextLookTime * 0.7f)
    {
        LookAtAlpha = FMath::FInterpTo(LookAtAlpha, 0.0f, GetDeltaSeconds(), 1.0f);
    }
}

void UPlayerAnimInstance::CalculateFootIK(const FString& FootBoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
    if (!SkeletalMeshComponent || !OwnerCharacter)
    {
        OutAlpha = 0.0f;
        return;
    }
    
    // Get foot bone location in world space
    FName BoneName(*FootBoneName);
    FVector FootLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);
    
    // Perform ground trace
    FVector GroundLocation = PerformFootTrace(FootLocation);
    
    // Calculate IK offset
    FVector IKOffset = GroundLocation - FootLocation;
    OutLocation = IKOffset;
    
    // Calculate foot rotation to match ground normal
    // This would need proper ground normal calculation from trace
    OutRotation = FRotator::ZeroRotator;
    
    // IK alpha based on distance from ground and movement state
    float DistanceFromGround = FMath::Abs(IKOffset.Z);
    float MaxIKDistance = 50.0f; // Maximum IK reach
    
    if (DistanceFromGround < MaxIKDistance && !bIsFalling)
    {
        OutAlpha = FMath::Clamp(1.0f - (DistanceFromGround / MaxIKDistance), 0.0f, 1.0f);
    }
    else
    {
        OutAlpha = 0.0f;
    }
}

FVector UPlayerAnimInstance::PerformFootTrace(const FVector& FootLocation) const
{
    if (!OwnerCharacter)
    {
        return FootLocation;
    }
    
    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return FootLocation;
    }
    
    // Trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    // Perform line trace
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return FootLocation;
}