// DinosaurRaptor.cpp
// Performance Optimizer #04 — Pack hunter Raptor implementation
// Transpersonal Game Studio — Prehistoric Survival Game

#include "Dinosaurs/DinosaurRaptor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ADinosaurRaptor::ADinosaurRaptor()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Raptor vital stats ──────────────────────────────────────────────────
    // Raptors are fast, fragile, pack hunters
    MaxHealth       = 300.0f;
    AttackDamage    = 80.0f;
    MoveSpeed       = 700.0f;   // Fastest dino — 7 m/s sprint
    DetectionRadius = 2000.0f;
    AttackRange     = 200.0f;

    // ── Capsule — slender, low profile ─────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(40.0f, 90.0f);  // 1.8m tall / 0.8m wide

    // ── Movement ────────────────────────────────────────────────────────────
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed          = MoveSpeed;
        Move->MaxAcceleration       = 3000.0f;   // Snappy acceleration
        Move->BrakingDecelerationWalking = 2000.0f;
        Move->RotationRate          = FRotator(0.0f, 360.0f, 0.0f);  // Very agile turning
        Move->bOrientRotationToMovement = true;
        Move->Mass                  = 80.0f;     // kg — lightweight
        Move->JumpZVelocity         = 600.0f;    // Can leap
    }

    // ── Pack defaults ───────────────────────────────────────────────────────
    PackRole    = EPerf_RaptorPackRole::Lone;
    MaxPackSize = 4;
    bIsPackHunter = true;
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::BeginPlay()
{
    Super::BeginPlay();

    // Start pack coordination timer at PackTickInterval (default 0.25s)
    // This is cheaper than running pack logic every frame
    if (bIsPackHunter && PackRole == EPerf_RaptorPackRole::Alpha)
    {
        GetWorldTimerManager().SetTimer(
            PackTickHandle,
            this,
            &ADinosaurRaptor::PackCoordinationTick,
            PackTickInterval,
            true  // looping
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Flankers move toward their cached flanking destination
    // (destination is updated by Alpha's PackCoordinationTick, not every frame)
    if (PackRole == EPerf_RaptorPackRole::Flanker && !CachedFlankDestination.IsZero())
    {
        // Simple move-toward — in full implementation this feeds into BehaviorTree
        // For now, a direct AddMovementInput toward destination
        FVector Dir = (CachedFlankDestination - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Dir, 1.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::PackCoordinationTick()
{
    // Only Alpha runs this
    if (PackRole != EPerf_RaptorPackRole::Alpha)
    {
        return;
    }

    // Clean up dead/invalid pack members
    PackMembers.RemoveAll([](const TWeakObjectPtr<ADinosaurRaptor>& M)
    {
        return !M.IsValid() || M->IsPendingKillPending();
    });

    // Recruit if pack is under max size
    if (PackMembers.Num() < MaxPackSize)
    {
        RecruitPackMembers();
    }

    // Assign flanking destinations to flankers based on current target
    // (CurrentTarget is inherited from DinosaurBase)
    if (CurrentTarget)
    {
        int32 FlankIndex = 0;
        for (TWeakObjectPtr<ADinosaurRaptor>& MemberPtr : PackMembers)
        {
            if (!MemberPtr.IsValid()) continue;
            ADinosaurRaptor* Member = MemberPtr.Get();

            if (Member->PackRole == EPerf_RaptorPackRole::Flanker)
            {
                // Offset each flanker by a different angle
                float AngleOffset = FlankingAngle + (FlankIndex * 60.0f);
                FVector FlankPos = ComputeFlankingDestination(CurrentTarget);
                // Rotate offset by index
                FVector Rotated = FlankPos.RotateAngleAxis(AngleOffset * FlankIndex, FVector::UpVector);
                Member->CachedFlankDestination = Rotated;
                FlankIndex++;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::RecruitPackMembers()
{
    if (!GetWorld()) return;

    // Sphere overlap to find nearby raptors
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PackRecruitRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    for (const FOverlapResult& Hit : Overlaps)
    {
        if (PackMembers.Num() >= MaxPackSize) break;

        ADinosaurRaptor* Candidate = Cast<ADinosaurRaptor>(Hit.GetActor());
        if (!Candidate || Candidate == this) continue;
        if (Candidate->PackAlpha.IsValid()) continue;  // Already in a pack

        // Recruit
        Candidate->PackAlpha = this;
        Candidate->PackRole  = (PackMembers.Num() % 2 == 0)
            ? EPerf_RaptorPackRole::Flanker
            : EPerf_RaptorPackRole::Distractor;

        PackMembers.AddUnique(Candidate);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::CallPackToAttack(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;

    for (TWeakObjectPtr<ADinosaurRaptor>& MemberPtr : PackMembers)
    {
        if (!MemberPtr.IsValid()) continue;
        MemberPtr->CurrentTarget = Target;
        MemberPtr->CurrentBehavior = EDinosaurBehavior::Attacking;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::PerformLeap(AActor* Target)
{
    if (!Target || !CanLeap()) return;

    LastLeapTime = GetWorld()->GetTimeSeconds();

    // Direction toward target
    FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // Apply launch impulse — upward arc + forward
    FVector LaunchVelocity = Dir * LeapImpulseForce + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage on arrival — simplified: immediate damage at leap start
    // Full implementation: damage on overlap with target capsule during leap
    UGameplayStatics::ApplyDamage(
        Target,
        LeapDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

#if WITH_EDITOR
    DrawDebugLine(
        GetWorld(),
        GetActorLocation(),
        Target->GetActorLocation(),
        FColor::Orange,
        false,
        1.5f,
        0,
        2.0f
    );
#endif
}

// ─────────────────────────────────────────────────────────────────────────────

bool ADinosaurRaptor::CanLeap() const
{
    if (!GetWorld()) return false;
    float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastLeapTime) >= LeapCooldown;
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::AssignPackRole(EPerf_RaptorPackRole NewRole)
{
    PackRole = NewRole;

    // If promoted to Alpha, start pack coordination timer
    if (NewRole == EPerf_RaptorPackRole::Alpha && bIsPackHunter)
    {
        GetWorldTimerManager().SetTimer(
            PackTickHandle,
            this,
            &ADinosaurRaptor::PackCoordinationTick,
            PackTickInterval,
            true
        );
    }
    else
    {
        // Non-alpha raptors don't run the coordination tick
        GetWorldTimerManager().ClearTimer(PackTickHandle);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

FVector ADinosaurRaptor::ComputeFlankingDestination(AActor* Target) const
{
    if (!Target) return GetActorLocation();

    // Compute a point FlankingRadius away from target, offset by FlankingAngle
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Right    = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();

    // Flanking position: behind-and-to-the-side of target
    FVector FlankPos = Target->GetActorLocation()
        + (-ToTarget * FlankingRadius)          // Behind target
        + (Right * FlankingRadius * 0.5f);      // To the side

    return FlankPos;
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::OnDeath_Implementation()
{
    Super::OnDeath_Implementation();  // Ragdoll from DinosaurBase

    // Clear pack timer
    GetWorldTimerManager().ClearTimer(PackTickHandle);

    // If this was the alpha, elect a new one
    if (PackRole == EPerf_RaptorPackRole::Alpha)
    {
        ElectNewAlpha();
    }
    else if (PackAlpha.IsValid())
    {
        // Remove self from alpha's pack list
        PackAlpha->PackMembers.RemoveAll([this](const TWeakObjectPtr<ADinosaurRaptor>& M)
        {
            return M.Get() == this;
        });
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurRaptor::ElectNewAlpha()
{
    // Promote the first valid pack member to Alpha
    for (TWeakObjectPtr<ADinosaurRaptor>& MemberPtr : PackMembers)
    {
        if (!MemberPtr.IsValid()) continue;
        ADinosaurRaptor* NewAlpha = MemberPtr.Get();

        // Transfer pack membership to new alpha
        NewAlpha->PackMembers = PackMembers;
        NewAlpha->PackMembers.Remove(MemberPtr);  // Remove self from own list
        NewAlpha->PackAlpha.Reset();

        // Assign alpha role (starts coordination timer)
        NewAlpha->AssignPackRole(EPerf_RaptorPackRole::Alpha);

        // Update all members to point to new alpha
        for (TWeakObjectPtr<ADinosaurRaptor>& OtherMember : NewAlpha->PackMembers)
        {
            if (OtherMember.IsValid())
            {
                OtherMember->PackAlpha = NewAlpha;
            }
        }

        break;  // Only one new alpha
    }
}
