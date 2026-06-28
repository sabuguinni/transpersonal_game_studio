// FootIKComponent.cpp
// Agent #10 — Animation Agent | PROD_CYCLE_AUTO_20260628_009
// Per-foot IK solving: traces terrain, adjusts foot placement and pelvis offset

#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UFootIKComponent::UFootIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Default socket names for a humanoid skeleton
    LeftFootSocketName  = FName("foot_l");
    RightFootSocketName = FName("foot_r");

    // Trace configuration
    TraceStartOffset  = 50.0f;
    TraceEndOffset    = 75.0f;
    TraceChannel      = ECollisionChannel::ECC_Visibility;

    // Interpolation speeds
    FootInterpSpeed   = 15.0f;
    PelvisInterpSpeed = 8.0f;

    // Limits
    MaxPelvisOffset   = 25.0f;
    MaxFootOffset     = 15.0f;

    // Debug
    bShowDebugTraces  = false;

    // Internal state
    CachedCharacter       = nullptr;
    CachedSkeletalMesh    = nullptr;
    CurrentPelvisOffset   = 0.0f;
}

void UFootIKComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedCharacter = Cast<ACharacter>(GetOwner());
    if (CachedCharacter)
    {
        CachedSkeletalMesh = CachedCharacter->GetMesh();
    }
}

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CachedCharacter || !CachedSkeletalMesh)
    {
        return;
    }

    // Skip IK when airborne — let the anim instance handle that
    UCharacterMovementComponent* MovComp = CachedCharacter->GetCharacterMovement();
    if (MovComp && MovComp->IsFalling())
    {
        // Smoothly blend out foot IK while airborne
        LeftFootData.BlendAlpha  = FMath::FInterpTo(LeftFootData.BlendAlpha,  0.0f, DeltaTime, FootInterpSpeed);
        RightFootData.BlendAlpha = FMath::FInterpTo(RightFootData.BlendAlpha, 0.0f, DeltaTime, FootInterpSpeed);
        CurrentPelvisOffset      = FMath::FInterpTo(CurrentPelvisOffset, 0.0f, DeltaTime, PelvisInterpSpeed);
        return;
    }

    // Solve each foot
    SolveFootIK(LeftFootSocketName,  LeftFootData,  DeltaTime);
    SolveFootIK(RightFootSocketName, RightFootData, DeltaTime);

    // Pelvis offset = push pelvis down so the lower foot stays grounded
    float DesiredPelvisOffset = FMath::Min(LeftFootData.PelvisOffset, RightFootData.PelvisOffset);
    DesiredPelvisOffset = FMath::Clamp(DesiredPelvisOffset, -MaxPelvisOffset, 0.0f);
    CurrentPelvisOffset = FMath::FInterpTo(CurrentPelvisOffset, DesiredPelvisOffset, DeltaTime, PelvisInterpSpeed);
}

void UFootIKComponent::SolveFootIK(const FName& SocketName, FAnim_FootIKData& OutData, float DeltaTime)
{
    if (!CachedSkeletalMesh)
    {
        return;
    }

    // Get current socket world location
    FVector SocketLocation = CachedSkeletalMesh->GetSocketLocation(SocketName);

    // Build trace start/end (vertical line trace)
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f,  TraceStartOffset);
    FVector TraceEnd   = SocketLocation + FVector(0.0f, 0.0f, -TraceEndOffset);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(CachedCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        TraceChannel,
        QueryParams
    );

#if ENABLE_DRAW_DEBUG
    if (bShowDebugTraces)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd,
            bHit ? FColor::Green : FColor::Red,
            false, -1.0f, 0, 1.5f);
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.0f, 8, FColor::Yellow, false, -1.0f);
        }
    }
#endif

    if (bHit)
    {
        // How far does the foot need to move from its current position?
        float FootHeightDelta = HitResult.ImpactPoint.Z - SocketLocation.Z;
        FootHeightDelta = FMath::Clamp(FootHeightDelta, -MaxFootOffset, MaxFootOffset);

        // Target foot location in component space (only Z matters for basic IK)
        FVector TargetWorld = SocketLocation;
        TargetWorld.Z = HitResult.ImpactPoint.Z;

        // Smooth interpolation
        OutData.TargetLocation = FMath::VInterpTo(OutData.TargetLocation, TargetWorld, DeltaTime, FootInterpSpeed);
        OutData.SurfaceNormal  = FMath::VInterpTo(OutData.SurfaceNormal, HitResult.ImpactNormal, DeltaTime, FootInterpSpeed);
        OutData.PelvisOffset   = FootHeightDelta < 0.0f ? FootHeightDelta : 0.0f;
        OutData.bIsGrounded    = true;
        OutData.BlendAlpha     = FMath::FInterpTo(OutData.BlendAlpha, 1.0f, DeltaTime, FootInterpSpeed);
    }
    else
    {
        // No ground found — blend out
        OutData.bIsGrounded = false;
        OutData.BlendAlpha  = FMath::FInterpTo(OutData.BlendAlpha, 0.0f, DeltaTime, FootInterpSpeed);
        OutData.PelvisOffset = 0.0f;
    }
}

float UFootIKComponent::GetPelvisOffset() const
{
    return CurrentPelvisOffset;
}

FAnim_FootIKData UFootIKComponent::GetLeftFootData() const
{
    return LeftFootData;
}

FAnim_FootIKData UFootIKComponent::GetRightFootData() const
{
    return RightFootData;
}
