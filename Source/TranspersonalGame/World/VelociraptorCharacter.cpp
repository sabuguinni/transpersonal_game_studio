// VelociraptorCharacter.cpp
// Performance Optimizer #4 — Cycle 006
// Pack hunter implementation with tick-throttled AI and bounded sphere overlap scans.
// Performance contract: PackSync runs on 2s timer (not Tick). Sphere overlap bounded to PackRadius.
// Tick interval = 0.1s (set in BeginPlay). URO handles animation interpolation.

#include "World/VelociraptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

AVelociraptorCharacter::AVelociraptorCharacter()
{
    // Performance: throttle tick — AI state machine doesn't need 60Hz
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz — adequate for raptor AI

    // Species identity
    SpeciesName = FName("Velociraptor");
    DinoSize = EDino_Size::Small;

    // Combat stats — fast, fragile pack hunter
    MaxHealth = 180.0f;
    CurrentHealth = 180.0f;
    AttackDamage = 55.0f;
    AttackRange = 180.0f;
    AttackCooldown = 1.8f;

    // Movement — fastest dinosaur in the pack
    MoveSpeed = 750.0f;
    DetectionRadius = 2000.0f;
    FleeHealthThreshold = 0.25f; // Raptors retreat at 25% health

    // Mass
    Mass = 15.0f; // kg — historically accurate for Velociraptor mongoliensis

    // Movement component setup
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = MoveSpeed;
        MoveComp->JumpZVelocity = 600.0f; // Raptors could leap
        MoveComp->GravityScale = 1.1f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // Fast turning
    }
}

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitializeSpeciesStats();

    // Start pack sync timer — runs every PackSyncInterval seconds (not every frame)
    // This is the key performance optimization: bounded sphere overlap at 0.5Hz
    GetWorldTimerManager().SetTimer(
        PackSyncTimerHandle,
        this,
        &AVelociraptorCharacter::SyncPackTarget,
        PackSyncInterval,
        true // looping
    );

    UE_LOG(LogTemp, Log, TEXT("VelociraptorCharacter: %s spawned. PackSync timer started at %.1fs interval."),
        *GetActorLabel(), PackSyncInterval);
}

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Base class handles AI state machine (Idle/Wander/Hunt/Flee/Attack)
    // Raptor-specific: check leap opportunity when in Attack state
    // (Leap is triggered from SyncPackTarget or direct attack logic)
}

void AVelociraptorCharacter::InitializeSpeciesStats()
{
    // Called after BeginPlay so Blueprint subclass overrides are respected
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = MoveSpeed;
    }

    // Ensure health is clamped to max
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("VelociraptorCharacter: Stats initialized — HP:%.0f ATK:%.0f SPD:%.0f DetR:%.0f"),
        MaxHealth, AttackDamage, MoveSpeed, DetectionRadius);
}

TArray<AVelociraptorCharacter*> AVelociraptorCharacter::GetNearbyPackMembers() const
{
    TArray<AVelociraptorCharacter*> Members;
    UWorld* World = GetWorld();
    if (!World) return Members;

    // Performance: use sphere overlap (bounded) instead of GetAllActorsOfClass (world-wide)
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = World->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
        FCollisionShape::MakeSphere(PackRadius),
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AVelociraptorCharacter* OtherRaptor = Cast<AVelociraptorCharacter>(Overlap.GetActor());
            if (OtherRaptor && OtherRaptor != this)
            {
                Members.Add(OtherRaptor);
            }
        }
    }

    return Members;
}

void AVelociraptorCharacter::SyncPackTarget()
{
    // Refresh cached pack members
    CachedPackMembers = GetNearbyPackMembers();

    if (CachedPackMembers.Num() == 0)
    {
        bIsPackLeader = true;
        PackLeader = nullptr;
        return;
    }

    // Elect leader among pack
    ElectPackLeader(CachedPackMembers);

    // If we have a leader and they have a target, adopt it
    if (!bIsPackLeader && PackLeader && PackLeader->CurrentTarget)
    {
        CurrentTarget = PackLeader->CurrentTarget;
        CurrentState = EDino_AIState::Hunt;

        UE_LOG(LogTemp, Verbose, TEXT("VelociraptorCharacter: %s synced target from leader %s → %s"),
            *GetActorLabel(),
            *PackLeader->GetActorLabel(),
            *CurrentTarget->GetActorLabel());
    }

    // If we have enough members, attempt coordinated flank
    if (CanFlankAttack() && CurrentTarget)
    {
        int32 MyIndex = CachedPackMembers.IndexOfByKey(this);
        FVector FlankPos = GetFlankPosition(MyIndex + 1, CachedPackMembers.Num() + 1);

        // Move toward flank position (base class movement handles actual navigation)
        // We set the target location hint for the AI movement system
        UE_LOG(LogTemp, Verbose, TEXT("VelociraptorCharacter: %s flanking at offset %.0f°"),
            *GetActorLabel(), FlankAngleOffset * (MyIndex + 1));
    }
}

void AVelociraptorCharacter::ElectPackLeader(const TArray<AVelociraptorCharacter*>& PackMembers)
{
    // Leader = raptor with highest current health (most capable of leading charge)
    AVelociraptorCharacter* HighestHealthRaptor = const_cast<AVelociraptorCharacter*>(this);
    float HighestHealth = CurrentHealth;

    for (AVelociraptorCharacter* Member : PackMembers)
    {
        if (Member && Member->CurrentHealth > HighestHealth)
        {
            HighestHealth = Member->CurrentHealth;
            HighestHealthRaptor = Member;
        }
    }

    bIsPackLeader = (HighestHealthRaptor == this);
    PackLeader = bIsPackLeader ? nullptr : HighestHealthRaptor;

    if (bIsPackLeader)
    {
        UE_LOG(LogTemp, Verbose, TEXT("VelociraptorCharacter: %s elected as pack leader (HP:%.0f)"),
            *GetActorLabel(), CurrentHealth);
    }
}

FVector AVelociraptorCharacter::GetFlankPosition(int32 MemberIndex, int32 TotalMembers) const
{
    if (!CurrentTarget) return GetActorLocation();

    // Distribute raptors evenly around the target in a semicircle
    // MemberIndex 0 = direct frontal (leader), 1+ = flanking angles
    float AngleDeg = FlankAngleOffset * MemberIndex;
    float AngleRad = FMath::DegreesToRadians(AngleDeg);

    FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector RightVec = FVector::CrossProduct(ToTarget, FVector::UpVector);

    // Rotate around target at AttackRange distance
    FVector FlankDir = ToTarget * FMath::Cos(AngleRad) + RightVec * FMath::Sin(AngleRad);
    return CurrentTarget->GetActorLocation() - FlankDir * AttackRange;
}

bool AVelociraptorCharacter::CanFlankAttack() const
{
    return CachedPackMembers.Num() >= (MinPackSizeForFlank - 1); // -1 because we count ourselves
}

void AVelociraptorCharacter::PerformLeapAttack()
{
    if (!CurrentTarget) return;

    UWorld* World = GetWorld();
    if (!World) return;

    float Now = World->GetTimeSeconds();
    if (Now - LastLeapTime < LeapCooldown) return; // Cooldown not expired

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > LeapRange) return; // Too far to leap

    // Mark leap active
    bLeapActive = true;
    LastLeapTime = Now;

    // Launch character toward target
    FVector LaunchDir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    LaunchDir.Z = 0.4f; // Arc upward slightly
    LaunchDir.Normalize();

    LaunchCharacter(LaunchDir * 1200.0f, true, true);

    // Apply damage after brief delay (simulates landing on prey)
    FTimerHandle DamageDelay;
    GetWorldTimerManager().SetTimer(DamageDelay, [this]()
    {
        if (CurrentTarget && IsValid(CurrentTarget))
        {
            UGameplayStatics::ApplyDamage(
                CurrentTarget,
                LeapDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );

            UE_LOG(LogTemp, Log, TEXT("VelociraptorCharacter: %s leap attack hit %s for %.0f damage"),
                *GetActorLabel(),
                *CurrentTarget->GetActorLabel(),
                LeapDamage);
        }
        ResetLeapActive();
    }, 0.35f, false);

    // Debug visualization
#if WITH_EDITOR
    DrawDebugLine(World, GetActorLocation(), CurrentTarget->GetActorLocation(),
        FColor::Orange, false, 1.5f, 0, 3.0f);
#endif
}

void AVelociraptorCharacter::ResetLeapActive()
{
    bLeapActive = false;
}
