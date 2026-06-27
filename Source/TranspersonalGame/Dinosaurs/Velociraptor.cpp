#include "Velociraptor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AVelociraptor::AVelociraptor()
{
    // Override base stats for raptor
    MaxHealth = 80.0f;
    CurrentHealth = 80.0f;
    AttackDamage = 30.0f;
    DetectionRadius = 1800.0f;
    AttackRange = 150.0f;
    bIsCarnivore = true;
    bIsPackHunter = true;

    // Raptor-specific defaults
    PackCoordinationRadius = 2500.0f;
    MinPackSizeForCoordination = 2;
    PounceDistance = 400.0f;
    PounceDamageMultiplier = 1.8f;
    bIsPackLeader = false;
    ActivePackSize = 1;
    FlankingAngleOffset = 60.0f;
    PackLeader = nullptr;

    // Fast movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
        GetCharacterMovement()->JumpZVelocity = 500.0f;
        GetCharacterMovement()->AirControl = 0.4f;
        GetCharacterMovement()->bCanWalkOffLedges = true;
    }

    // Smaller capsule than T-Rex
    GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
    GetCapsuleComponent()->SetCapsuleRadius(30.0f);
}

void AVelociraptor::BeginPlay()
{
    Super::BeginPlay();

    // Start pack coordination on a 1-second interval
    GetWorldTimerManager().SetTimer(
        PackCoordinationTimer,
        this,
        &AVelociraptor::RunPackCoordination,
        1.0f,
        true
    );

    // Initial leader election
    ElectPackLeader();
}

void AVelociraptor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ── Pack Coordination ──────────────────────────────────────────────────────

TArray<AVelociraptor*> AVelociraptor::GetNearbyPackMembers() const
{
    TArray<AVelociraptor*> Members;
    TArray<AActor*> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;
        AVelociraptor* Other = Cast<AVelociraptor>(Actor);
        if (!Other || Other->IsDead()) continue;

        float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        if (Dist <= PackCoordinationRadius)
        {
            Members.Add(Other);
        }
    }
    return Members;
}

void AVelociraptor::ElectPackLeader()
{
    TArray<AVelociraptor*> PackMembers = GetNearbyPackMembers();
    if (PackMembers.Num() == 0)
    {
        // Lone raptor — self-lead
        bIsPackLeader = true;
        PackLeader = nullptr;
        ActivePackSize = 1;
        return;
    }

    // Find the raptor with highest health (becomes leader)
    AVelociraptor* HealthiestRaptor = this;
    for (AVelociraptor* Member : PackMembers)
    {
        if (Member->CurrentHealth > HealthiestRaptor->CurrentHealth)
        {
            HealthiestRaptor = Member;
        }
    }

    bIsPackLeader = (HealthiestRaptor == this);
    PackLeader = bIsPackLeader ? nullptr : HealthiestRaptor;
    ActivePackSize = PackMembers.Num() + 1;
}

void AVelociraptor::RunPackCoordination()
{
    if (IsDead()) return;

    ElectPackLeader();

    // Only the leader broadcasts targets
    if (bIsPackLeader && CurrentTarget)
    {
        BroadcastTargetToPackMembers(CurrentTarget);
    }
}

void AVelociraptor::BroadcastTargetToPackMembers(AActor* NewTarget)
{
    if (!NewTarget) return;

    TArray<AVelociraptor*> PackMembers = GetNearbyPackMembers();
    ActivePackSize = PackMembers.Num() + 1;

    float AngleStep = (PackMembers.Num() > 0) ? (360.0f / (PackMembers.Num() + 1)) : 0.0f;
    int32 Index = 1;

    for (AVelociraptor* Member : PackMembers)
    {
        Member->ReceivePackTarget(NewTarget, this);
        // Assign flanking angle so raptors surround the target
        Member->FlankingAngleOffset = AngleStep * Index;
        Index++;
    }
}

void AVelociraptor::ReceivePackTarget(AActor* SharedTarget, AVelociraptor* Leader)
{
    if (!SharedTarget || IsDead()) return;

    CurrentTarget = SharedTarget;
    PackLeader = Leader;
    bIsPackLeader = false;

    // Move to flanking position around the target
    FVector FlankPos = CalculateFlankingPosition(SharedTarget, FlankingAngleOffset);
    // Navigation handled by AIController — just set target for BT
}

FVector AVelociraptor::CalculateFlankingPosition(AActor* Target, float AngleOffset) const
{
    if (!Target) return GetActorLocation();

    FVector TargetLoc = Target->GetActorLocation();
    float RadAngle = FMath::DegreesToRadians(AngleOffset);
    float FlankRadius = 300.0f;

    FVector Offset(
        FMath::Cos(RadAngle) * FlankRadius,
        FMath::Sin(RadAngle) * FlankRadius,
        0.0f
    );
    return TargetLoc + Offset;
}

// ── Combat ─────────────────────────────────────────────────────────────────

void AVelociraptor::PerformPounce()
{
    if (!CurrentTarget || IsDead()) return;

    FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= PounceDistance)
    {
        // Launch toward target
        FVector LaunchVelocity = ToTarget * 800.0f + FVector(0.0f, 0.0f, 350.0f);
        LaunchCharacter(LaunchVelocity, true, true);

        // Apply pounce damage
        float PounceDamage = AttackDamage * PounceDamageMultiplier;
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            PounceDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        UE_LOG(LogTemp, Log, TEXT("Raptor %s pounced for %.0f damage"), *GetName(), PounceDamage);
    }
}

void AVelociraptor::PerformAttack()
{
    if (!CurrentTarget || IsDead()) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Use pounce if within range, otherwise standard bite
    if (DistToTarget <= PounceDistance)
    {
        PerformPounce();
    }
    else
    {
        // Standard bite attack
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
        UE_LOG(LogTemp, Log, TEXT("Raptor %s bit for %.0f damage"), *GetName(), AttackDamage);
    }
}

void AVelociraptor::OnDeath()
{
    // Stop pack coordination timer
    GetWorldTimerManager().ClearTimer(PackCoordinationTimer);

    // If this was the pack leader, trigger re-election in remaining members
    if (bIsPackLeader)
    {
        TArray<AVelociraptor*> PackMembers = GetNearbyPackMembers();
        for (AVelociraptor* Member : PackMembers)
        {
            Member->PackLeader = nullptr;
            Member->ElectPackLeader();
        }
    }

    // Call base death logic (ragdoll, loot, etc.)
    Super::OnDeath();
}
