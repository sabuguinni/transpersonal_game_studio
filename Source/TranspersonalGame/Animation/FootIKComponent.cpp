// FootIKComponent.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Foot IK component: adapts character feet to uneven prehistoric terrain
// Uses line traces per foot bone, pelvis adjustment, and smooth interpolation
// Works in tandem with PlayerAnimInstance for runtime foot placement

#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UFootIKComponent::UFootIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Every frame for smooth IK

    // Bone names (match the survivor skeleton rig)
    LeftFootBoneName  = FName(TEXT("foot_l"));
    RightFootBoneName = FName(TEXT("foot_r"));
    PelvisBoneName    = FName(TEXT("pelvis"));

    // Trace settings
    TraceDistance     = 60.0f;
    TraceRadius       = 5.0f;
    InterpSpeed       = 15.0f;
    MaxFootOffset     = 40.0f;
    MaxPelvisOffset   = 30.0f;

    // Runtime state
    LeftFootTargetOffset  = FVector::ZeroVector;
    RightFootTargetOffset = FVector::ZeroVector;
    LeftFootCurrentOffset = FVector::ZeroVector;
    RightFootCurrentOffset= FVector::ZeroVector;
    PelvisCurrentOffset   = 0.0f;
    PelvisTargetOffset    = 0.0f;

    bDebugDraw        = false;
    bIsEnabled        = true;
}

void UFootIKComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CachedMesh = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("FootIKComponent: Initialized on %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FootIKComponent: Owner is not ACharacter — disabling"));
        bIsEnabled = false;
    }
}

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsEnabled || !OwnerCharacter || !CachedMesh) return;

    // Skip IK when airborne
    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (MoveComp && MoveComp->IsFalling())
    {
        // Blend offsets back to zero when airborne
        LeftFootCurrentOffset  = FMath::VInterpTo(LeftFootCurrentOffset,  FVector::ZeroVector, DeltaTime, InterpSpeed);
        RightFootCurrentOffset = FMath::VInterpTo(RightFootCurrentOffset, FVector::ZeroVector, DeltaTime, InterpSpeed);
        PelvisCurrentOffset    = FMath::FInterpTo(PelvisCurrentOffset,    0.0f,                DeltaTime, InterpSpeed);
        return;
    }

    // Perform foot traces
    LeftFootTargetOffset  = PerformFootTrace(LeftFootBoneName);
    RightFootTargetOffset = PerformFootTrace(RightFootBoneName);

    // Smooth interpolation
    LeftFootCurrentOffset  = FMath::VInterpTo(LeftFootCurrentOffset,  LeftFootTargetOffset,  DeltaTime, InterpSpeed);
    RightFootCurrentOffset = FMath::VInterpTo(RightFootCurrentOffset, RightFootTargetOffset, DeltaTime, InterpSpeed);

    // Pelvis adjustment — lower pelvis to accommodate the lowest foot
    float LowestFoot   = FMath::Min(LeftFootCurrentOffset.Z, RightFootCurrentOffset.Z);
    PelvisTargetOffset = FMath::Clamp(LowestFoot, -MaxPelvisOffset, 0.0f);
    PelvisCurrentOffset= FMath::FInterpTo(PelvisCurrentOffset, PelvisTargetOffset, DeltaTime, InterpSpeed * 0.5f);

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(),
            CachedMesh->GetSocketLocation(LeftFootBoneName) + LeftFootCurrentOffset,
            8.0f, 8, FColor::Green, false, -1.0f, 0, 1.0f);
        DrawDebugSphere(GetWorld(),
            CachedMesh->GetSocketLocation(RightFootBoneName) + RightFootCurrentOffset,
            8.0f, 8, FColor::Blue, false, -1.0f, 0, 1.0f);
    }
}

FVector UFootIKComponent::PerformFootTrace(const FName& BoneName)
{
    if (!CachedMesh || !GetWorld()) return FVector::ZeroVector;

    FVector BoneWorldLocation = CachedMesh->GetSocketLocation(BoneName);
    FVector TraceStart = BoneWorldLocation + FVector(0.0f, 0.0f, TraceDistance);
    FVector TraceEnd   = BoneWorldLocation - FVector(0.0f, 0.0f, TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(TraceRadius),
        QueryParams
    );

    if (bHit)
    {
        FVector Offset = HitResult.ImpactPoint - BoneWorldLocation;
        // Clamp to max offset to prevent extreme deformation
        Offset.Z = FMath::Clamp(Offset.Z, -MaxFootOffset, MaxFootOffset);

        if (bDebugDraw)
        {
            DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Yellow, false, -1.0f, 0, 0.5f);
        }

        return Offset;
    }

    return FVector::ZeroVector;
}

FVector UFootIKComponent::GetLeftFootOffset() const
{
    return LeftFootCurrentOffset;
}

FVector UFootIKComponent::GetRightFootOffset() const
{
    return RightFootCurrentOffset;
}

float UFootIKComponent::GetPelvisOffset() const
{
    return PelvisCurrentOffset;
}

void UFootIKComponent::SetEnabled(bool bEnabled)
{
    bIsEnabled = bEnabled;
}

void UFootIKComponent::SetDebugDraw(bool bDraw)
{
    bDebugDraw = bDraw;
}
