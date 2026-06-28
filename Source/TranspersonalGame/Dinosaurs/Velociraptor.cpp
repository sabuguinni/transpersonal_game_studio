// Velociraptor.cpp — Velociraptor implementation
// Agent #4 Performance Optimizer — Transpersonal Game Studio
// Pack predator: coordinated hunting, leap attacks, flanking maneuvers

#include "Dinosaurs/Velociraptor.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVelociraptor::AVelociraptor()
{
    PrimaryActorTick.bCanEverTick = true;

    // === SPECIES IDENTITY ===
    // Species set via DinosaurBase properties (set in BeginPlay to avoid CDO issues)

    // === PHYSICAL STATS ===
    // Health/damage set in BeginPlay

    // === CAPSULE — small, agile predator ===
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(80.0f);
        Capsule->SetCapsuleRadius(40.0f);
    }

    // === MOVEMENT — fast sprint, stealthy stalk ===
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = 600.0f;
        Movement->MaxAcceleration = 2400.0f;
        Movement->JumpZVelocity = 700.0f;
        Movement->AirControl = 0.4f;
        Movement->bCanWalkOffLedges = true;
    }

    // === PACK BEHAVIOR ===
    bIsPackLeader = false;
    PackSize = 3;
    PackCommunicationRadius = 3000.0f;

    // === COMBAT ===
    LeapAttackRange = 400.0f;
    LeapAttackDamage = 65.0f;
    LeapCooldown = 4.0f;
    ClawSlashDamage = 45.0f;
    bIsFlankingTarget = false;

    // === MOVEMENT MODES ===
    SprintSpeed = 1200.0f;
    StalkSpeed = 150.0f;
    bIsStalking = false;

    // === INTELLIGENCE ===
    FlankingAngleOffset = 90.0f;
    bCanOpenDoors = false;
    ProblemSolvingScore = 0.85f;

    // === INTERNAL STATE ===
    LeapCooldownTimer = 0.0f;
    bLeapOnCooldown = false;
    FlankingDestination = FVector::ZeroVector;
}

void AVelociraptor::BeginPlay()
{
    Super::BeginPlay();

    // Set species-specific stats after Super::BeginPlay
    // (DinosaurBase properties initialized here to avoid CDO null issues)
    MaxHealth = 350.0f;
    CurrentHealth = MaxHealth;
    AttackDamage = ClawSlashDamage;
    bIsPackAnimal = true;
    bIsAggressive = true;
    DetectionRadius = 1800.0f;
    AttackRange = 180.0f;
    WalkSpeed = 400.0f;

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor::BeginPlay — %s initialized. PackLeader=%s, PackSize=%d"),
        *GetName(),
        bIsPackLeader ? TEXT("YES") : TEXT("NO"),
        PackSize);
}

void AVelociraptor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Leap cooldown timer
    if (bLeapOnCooldown)
    {
        LeapCooldownTimer -= DeltaTime;
        if (LeapCooldownTimer <= 0.0f)
        {
            bLeapOnCooldown = false;
            LeapCooldownTimer = 0.0f;
        }
    }

    // Pack coordination update (only leader coordinates)
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        UpdatePackCoordination(DeltaTime);
    }
}

// === PACK BEHAVIOR ===

void AVelociraptor::SignalPackToAttack(AActor* Target)
{
    if (!Target) return;

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Signaling pack to attack %s"),
        *GetName(), *Target->GetName());

    // Signal all pack members to engage the same target
    for (AVelociraptor* Member : PackMembers)
    {
        if (Member && Member != this && !Member->IsPendingKill())
        {
            // Assign flanking positions at different angles
            float AngleOffset = FMath::RandRange(45.0f, 135.0f);
            Member->BeginFlankingManeuver(Target, AngleOffset);
        }
    }

    // Leader attacks directly
    PerformLeapAttack(Target);
}

void AVelociraptor::JoinPack(AVelociraptor* PackLeader)
{
    if (!PackLeader || PackLeader == this) return;

    bIsPackLeader = false;
    PackLeader->PackMembers.AddUnique(this);

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Joined pack led by %s"),
        *GetName(), *PackLeader->GetName());
}

void AVelociraptor::LeavePackFormation()
{
    bIsFlankingTarget = false;
    FlankingDestination = FVector::ZeroVector;

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Left pack formation"), *GetName());
}

// === COMBAT ===

void AVelociraptor::PerformLeapAttack(AActor* Target)
{
    if (!Target || bLeapOnCooldown) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > LeapAttackRange) return;

    // Apply leap damage
    UGameplayStatics::ApplyDamage(
        Target,
        LeapAttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Launch toward target
    FVector LaunchDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    LaunchDir.Z = 0.5f; // Arc upward
    LaunchCharacter(LaunchDir * 800.0f, true, true);

    // Start cooldown
    bLeapOnCooldown = true;
    LeapCooldownTimer = LeapCooldown;

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Leap attack on %s — %.0f damage"),
        *GetName(), *Target->GetName(), LeapAttackDamage);
}

void AVelociraptor::PerformClawSlash(AActor* Target)
{
    if (!Target) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    UGameplayStatics::ApplyDamage(
        Target,
        ClawSlashDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Claw slash on %s — %.0f damage"),
        *GetName(), *Target->GetName(), ClawSlashDamage);
}

void AVelociraptor::BeginFlankingManeuver(AActor* Target, float AngleOffset)
{
    if (!Target) return;

    bIsFlankingTarget = true;
    FlankingAngleOffset = AngleOffset;
    CalculateFlankingPosition(Target);

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Flanking at %.0f deg offset"),
        *GetName(), AngleOffset);
}

// === MOVEMENT MODES ===

void AVelociraptor::EnterStalkMode()
{
    bIsStalking = true;

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = StalkSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Entered stalk mode (%.0f cm/s)"),
        *GetName(), StalkSpeed);
}

void AVelociraptor::ExitStalkMode()
{
    bIsStalking = false;

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = SprintSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Exited stalk mode — sprint %.0f cm/s"),
        *GetName(), SprintSpeed);
}

// === OVERRIDES ===

void AVelociraptor::OnTargetDetected(AActor* Target)
{
    if (!Target) return;

    Super::OnTargetDetected(Target);

    // Pack leader coordinates the hunt
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        // Enter stalk mode first, then signal pack
        EnterStalkMode();

        UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Pack leader detected target %s — initiating coordinated hunt"),
            *GetName(), *Target->GetName());

        // After brief stalk delay, signal pack (would use timer in full implementation)
        SignalPackToAttack(Target);
    }
    else
    {
        // Solo raptor — leap attack directly
        PerformLeapAttack(Target);
    }
}

void AVelociraptor::OnDeath()
{
    Super::OnDeath();

    // Notify pack members of death
    for (AVelociraptor* Member : PackMembers)
    {
        if (Member && !Member->IsPendingKill())
        {
            Member->PackMembers.Remove(this);
            // If leader died, first member becomes new leader
            if (bIsPackLeader && Member->PackMembers.Num() > 0)
            {
                Member->bIsPackLeader = true;
                UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Promoted to pack leader after leader death"),
                    *Member->GetName());
                break;
            }
        }
    }

    PackMembers.Empty();

    UE_LOG(LogTemp, Log, TEXT("AVelociraptor [%s]: Died. Pack disbanded/reassigned."), *GetName());
}

// === PRIVATE HELPERS ===

void AVelociraptor::UpdatePackCoordination(float DeltaTime)
{
    // Remove dead/invalid pack members
    PackMembers.RemoveAll([](AVelociraptor* M) {
        return !M || M->IsPendingKill();
    });
}

void AVelociraptor::CalculateFlankingPosition(AActor* Target)
{
    if (!Target) return;

    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = (TargetLoc - GetActorLocation()).GetSafeNormal();

    // Rotate direction by flanking angle
    FRotator FlankRot(0.0f, FlankingAngleOffset, 0.0f);
    FVector FlankDir = FlankRot.RotateVector(ToTarget);

    // Position 300cm from target at flanking angle
    FlankingDestination = TargetLoc + FlankDir * (-300.0f);
    FlankingDestination.Z = TargetLoc.Z;
}
