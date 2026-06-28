#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UFootIKComponent::UFootIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork; // run after anim update
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay — cache owner references
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMesh = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }

    if (!OwnerCharacter || !OwnerMesh || !MovementComponent)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("UFootIKComponent: Owner is not a valid ACharacter — IK disabled."));
        SetComponentTickEnabled(false);
        bIKEnabled = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent — main IK update
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIKEnabled || !OwnerCharacter || !OwnerMesh || !MovementComponent)
    {
        bIKActive = false;
        return;
    }

    // Disable IK when moving fast or in the air to avoid foot-sliding artefacts
    const float CurrentSpeed = MovementComponent->Velocity.Size();
    const bool bIsGrounded = MovementComponent->IsMovingOnGround();

    bIKActive = bIsGrounded && (CurrentSpeed <= MaxSpeedForIK);

    if (!bIKActive)
    {
        // Smoothly blend out IK when we go above speed threshold
        LeftFootData.BlendAlpha  = FMath::FInterpTo(LeftFootData.BlendAlpha,  0.0f, DeltaTime, FootInterpSpeed);
        RightFootData.BlendAlpha = FMath::FInterpTo(RightFootData.BlendAlpha, 0.0f, DeltaTime, FootInterpSpeed);
        return;
    }

    SolveFootIK(LeftFootSocketName,  LeftFootData,  DeltaTime);
    SolveFootIK(RightFootSocketName, RightFootData, DeltaTime);

    // Pelvis correction: lower the pelvis by the larger of the two foot offsets
    // so the lower foot is always grounded rather than floating.
    const float TargetPelvisOffset = FMath::Min(LeftFootData.PelvisOffset,
                                                RightFootData.PelvisOffset);
    const float ClampedPelvis = FMath::Clamp(TargetPelvisOffset,
                                             -MaxPelvisOffset, 0.0f);

    // Write the same pelvis value into both structs so AnimInstance can read either
    LeftFootData.PelvisOffset  = FMath::FInterpTo(LeftFootData.PelvisOffset,
                                                   ClampedPelvis, DeltaTime,
                                                   PelvisInterpSpeed);
    RightFootData.PelvisOffset = LeftFootData.PelvisOffset;
}

// ─────────────────────────────────────────────────────────────────────────────
// SolveFootIK — trace + smooth for one foot
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::SolveFootIK(const FName& SocketName,
                                    FAnim_FootIKData& OutData,
                                    float DeltaTime)
{
    if (!OwnerMesh)
    {
        return;
    }

    const FVector SocketLocation = OwnerMesh->GetSocketLocation(SocketName);

    FHitResult Hit;
    const bool bHit = TraceForGround(SocketLocation, Hit);

    if (bHit)
    {
        // Smooth target location
        OutData.TargetLocation = FMath::VInterpTo(OutData.TargetLocation,
                                                   Hit.ImpactPoint,
                                                   DeltaTime,
                                                   FootInterpSpeed);

        // Smooth surface normal
        OutData.SurfaceNormal = FMath::VInterpTo(OutData.SurfaceNormal,
                                                  Hit.ImpactNormal,
                                                  DeltaTime,
                                                  FootInterpSpeed);

        // Height delta between hit point and socket (negative = foot needs to go down)
        const float HeightDelta = Hit.ImpactPoint.Z - SocketLocation.Z;
        OutData.PelvisOffset = FMath::Min(OutData.PelvisOffset, HeightDelta);

        // Blend in IK
        OutData.BlendAlpha = FMath::FInterpTo(OutData.BlendAlpha, 1.0f,
                                               DeltaTime, FootInterpSpeed);
        OutData.bIsGrounded = true;

        if (bDrawDebugTraces)
        {
            DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5.0f, 8,
                            FColor::Green, false, -1.0f, 0, 1.0f);
            DrawDebugLine(GetWorld(), SocketLocation, Hit.ImpactPoint,
                          FColor::Yellow, false, -1.0f, 0, 0.5f);
        }
    }
    else
    {
        // No ground found — blend out
        OutData.BlendAlpha = FMath::FInterpTo(OutData.BlendAlpha, 0.0f,
                                               DeltaTime, FootInterpSpeed);
        OutData.bIsGrounded = false;
        OutData.PelvisOffset = 0.0f;

        if (bDrawDebugTraces)
        {
            const FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, TraceEndOffset);
            DrawDebugLine(GetWorld(), SocketLocation, TraceEnd,
                          FColor::Red, false, -1.0f, 0, 0.5f);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TraceForGround — single downward line trace
// ─────────────────────────────────────────────────────────────────────────────

bool UFootIKComponent::TraceForGround(const FVector& SocketWorldLocation,
                                       FHitResult& OutHit) const
{
    if (!GetWorld())
    {
        return false;
    }

    const FVector TraceStart = SocketWorldLocation + FVector(0.0f, 0.0f,  TraceStartOffset);
    const FVector TraceEnd   = SocketWorldLocation - FVector(0.0f, 0.0f,  TraceEndOffset);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    Params.bTraceComplex = false;

    return GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd,
                                                TraceChannel, Params);
}

// ─────────────────────────────────────────────────────────────────────────────
// SetIKEnabled — runtime toggle
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    if (!bEnabled)
    {
        LeftFootData.BlendAlpha  = 0.0f;
        RightFootData.BlendAlpha = 0.0f;
        bIKActive = false;
    }
}
