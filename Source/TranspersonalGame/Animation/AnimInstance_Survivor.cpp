#include "AnimInstance_Survivor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnimInstance_Survivor::UAnimInstance_Survivor()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;

    // IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    PelvisOffset = 0.0f;
    FootIKAlpha = 1.0f;
    PelvisInterpSpeed = 15.0f;
    FootTraceDistance = 80.0f;

    // Survival defaults
    Stamina = 100.0f;
    Health = 100.0f;
    bIsInjured = false;
    bIsCarryingHeavy = false;

    // Combat defaults
    bIsInCombatStance = false;
    bIsAttacking = false;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnimInstance_Survivor::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner character and movement component
    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UAnimInstance_Survivor::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // ─── Locomotion Update ─────────────────────────────────────────────────────

    // Speed from velocity magnitude (XY plane only — ignore vertical)
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    bIsMoving = Speed > 10.0f;

    // Direction: angle between character forward and velocity direction
    if (bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNorm = Velocity.GetSafeNormal2D();
        Direction = FMath::RadiansToDegrees(FMath::Atan2(
            FVector::DotProduct(OwnerCharacter->GetActorRightVector(), VelocityNorm),
            FVector::DotProduct(ForwardVector, VelocityNorm)
        ));
    }
    else
    {
        Direction = 0.0f;
    }

    // Air state
    bIsInAir = MovementComponent->IsFalling();

    // Crouch state
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Sprint detection: speed threshold
    bIsSprinting = Speed > 400.0f && !bIsCrouching && !bIsInAir;

    // ─── Survival State Update ─────────────────────────────────────────────────

    // Injured threshold: health below 30%
    bIsInjured = Health < 30.0f;

    // Reduce foot IK alpha when in air (no ground contact)
    FootIKAlpha = bIsInAir ? 0.0f : 1.0f;

    // ─── IK Foot Placement ─────────────────────────────────────────────────────
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset IK when airborne
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, PelvisInterpSpeed);
    }
}

void UAnimInstance_Survivor::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return;
    }

    // Get the skeletal mesh component for bone locations
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    FCollisionQueryParams TraceParams(FName(TEXT("FootIK")), true, OwnerCharacter);

    // ─── Left Foot Trace ───────────────────────────────────────────────────────
    FVector LeftFootBoneLocation = MeshComp->GetBoneLocation(FName("foot_l"));
    FVector LeftTraceStart = LeftFootBoneLocation + FVector(0, 0, FootTraceDistance);
    FVector LeftTraceEnd   = LeftFootBoneLocation - FVector(0, 0, FootTraceDistance);

    FHitResult LeftHit;
    if (World->LineTraceSingleByChannel(LeftHit, LeftTraceStart, LeftTraceEnd, ECC_Visibility, TraceParams))
    {
        LeftFootIKLocation = LeftHit.ImpactPoint;
    }
    else
    {
        LeftFootIKLocation = LeftFootBoneLocation;
    }

    // ─── Right Foot Trace ──────────────────────────────────────────────────────
    FVector RightFootBoneLocation = MeshComp->GetBoneLocation(FName("foot_r"));
    FVector RightTraceStart = RightFootBoneLocation + FVector(0, 0, FootTraceDistance);
    FVector RightTraceEnd   = RightFootBoneLocation - FVector(0, 0, FootTraceDistance);

    FHitResult RightHit;
    if (World->LineTraceSingleByChannel(RightHit, RightTraceStart, RightTraceEnd, ECC_Visibility, TraceParams))
    {
        RightFootIKLocation = RightHit.ImpactPoint;
    }
    else
    {
        RightFootIKLocation = RightFootBoneLocation;
    }

    // ─── Pelvis Offset Calculation ─────────────────────────────────────────────
    float TargetPelvisOffset = CalculatePelvisOffset();
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, PelvisInterpSpeed);
}

float UAnimInstance_Survivor::CalculatePelvisOffset() const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }

    // Pelvis drops to accommodate the lower foot
    float LeftZ  = LeftFootIKLocation.Z;
    float RightZ = RightFootIKLocation.Z;
    float CharZ  = OwnerCharacter->GetActorLocation().Z;

    // The pelvis offset is the difference between the lower foot and the character base
    float LowerFoot = FMath::Min(LeftZ, RightZ);
    float Offset = LowerFoot - CharZ;

    // Clamp to reasonable range to prevent extreme deformation
    return FMath::Clamp(Offset, -30.0f, 10.0f);
}

void UAnimInstance_Survivor::PlaySurvivorMontage(const FName& MontageName)
{
    // Montage lookup is handled by the Animation Blueprint via named slots.
    // This function serves as the Blueprint-callable trigger point.
    // The actual montage asset is assigned in the Animation Blueprint.
    UE_LOG(LogTemp, Log, TEXT("AnimInstance_Survivor: PlaySurvivorMontage called — %s"), *MontageName.ToString());
}

void UAnimInstance_Survivor::StopCurrentMontage(float BlendOutTime)
{
    // Stop any currently playing montage with blend-out
    Montage_StopGroupByName(BlendOutTime, FName("DefaultGroup"));
    UE_LOG(LogTemp, Log, TEXT("AnimInstance_Survivor: StopCurrentMontage — BlendOut=%.2f"), BlendOutTime);
}
