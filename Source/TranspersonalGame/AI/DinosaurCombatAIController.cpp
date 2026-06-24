#include "DinosaurCombatAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz — sufficient for combat AI

    // Default profile — generic dinosaur
    CombatProfile.Species = ECombat_DinoSpecies::Generic;
    CombatProfile.DetectionRadius = 1500.0f;
    CombatProfile.ChaseRadius = 2500.0f;
    CombatProfile.AttackRadius = 250.0f;
    CombatProfile.AttackDamage = 30.0f;
    CombatProfile.AttackCooldown = 2.0f;
    CombatProfile.MoveSpeed_Patrol = 150.0f;
    CombatProfile.MoveSpeed_Chase = 500.0f;
    CombatProfile.FleeHealthThreshold = 0.0f;
    CombatProfile.bCanFlanK = false;
    CombatProfile.bIsPackHunter = false;
    CombatProfile.PrimaryAttack = ECombat_AttackType::Bite;
    CombatProfile.SecondaryAttack = ECombat_AttackType::None;
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn) return;

    // Grab NPCBehaviorComponent if the pawn has one
    BehaviorComp = InPawn->FindComponentByClass<UNPCBehaviorComponent>();

    // Set initial patrol speed
    SetMovementSpeed(CombatProfile.MoveSpeed_Patrol);
}

void ADinosaurCombatAIController::OnUnPossess()
{
    Super::OnUnPossess();
    BehaviorComp = nullptr;
    CurrentTarget = nullptr;
    bIsInCombat = false;
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Attack cooldown timer
    if (!bCanAttackNow)
    {
        TimeSinceLastAttack += DeltaTime;
        if (TimeSinceLastAttack >= CombatProfile.AttackCooldown)
        {
            bCanAttackNow = true;
            TimeSinceLastAttack = 0.0f;
        }
    }

    // Periodic target scan
    ScanTimer += DeltaTime;
    if (ScanTimer >= ScanInterval)
    {
        ScanTimer = 0.0f;
        ScanForTargets();
    }

    // Main combat state machine
    UpdateCombatBehavior(DeltaTime);
}

// ============================================================
// Target Management
// ============================================================

void ADinosaurCombatAIController::SetTarget(AActor* NewTarget)
{
    if (!NewTarget) return;

    CurrentTarget = NewTarget;
    bIsInCombat = true;

    // Notify NPCBehaviorComponent of threat
    if (BehaviorComp)
    {
        BehaviorComp->OnThreatDetected(NewTarget, ENPC_ThreatLevel::High);
    }

    // Notify pack if pack hunter
    if (CombatProfile.bIsPackHunter && TacticalRole == ECombat_TacticalRole::PackLeader)
    {
        NotifyPackOfTarget(NewTarget);
    }

    SetMovementSpeed(CombatProfile.MoveSpeed_Chase);
}

void ADinosaurCombatAIController::ClearTarget()
{
    CurrentTarget = nullptr;
    bIsInCombat = false;

    if (BehaviorComp)
    {
        BehaviorComp->OnThreatLost();
    }

    SetMovementSpeed(CombatProfile.MoveSpeed_Patrol);
}

// ============================================================
// Combat State Machine
// ============================================================

void ADinosaurCombatAIController::UpdateCombatBehavior(float DeltaTime)
{
    if (ShouldFlee())
    {
        // Move away from target
        if (CurrentTarget && GetPawn())
        {
            FVector AwayDir = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
            FVector FleeTarget = GetPawn()->GetActorLocation() + AwayDir * 2000.0f;
            MoveToLocation(FleeTarget, 100.0f);
        }
        return;
    }

    if (!CurrentTarget || !bIsInCombat)
    {
        UpdatePatrol(DeltaTime);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist <= CombatProfile.AttackRadius)
    {
        UpdateAttack(DeltaTime);
    }
    else if (Dist <= CombatProfile.ChaseRadius)
    {
        // Flankers go to flank position, others chase directly
        if (TacticalRole == ECombat_TacticalRole::Flanker && !FlankData.bFlankPositionReached)
        {
            UpdateFlanking(DeltaTime);
        }
        else
        {
            UpdateChase(DeltaTime);
        }
    }
    else
    {
        // Target out of chase range — lose it
        ClearTarget();
    }
}

void ADinosaurCombatAIController::UpdatePatrol(float DeltaTime)
{
    if (!BehaviorComp) return;

    FVector PatrolTarget = BehaviorComp->GetNextPatrolTarget();
    if (PatrolTarget != FVector::ZeroVector)
    {
        MoveToLocation(PatrolTarget, 50.0f);
    }
}

void ADinosaurCombatAIController::UpdateChase(float DeltaTime)
{
    if (!CurrentTarget) return;
    MoveToActor(CurrentTarget, CombatProfile.AttackRadius * 0.8f);
}

void ADinosaurCombatAIController::UpdateAttack(float DeltaTime)
{
    if (!CurrentTarget) return;

    // Face the target
    FVector ToTarget = (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FRotator FaceRot = ToTarget.Rotation();
    GetPawn()->SetActorRotation(FaceRot);

    // Stop moving — we're in melee range
    StopMovement();

    // Execute attack if cooldown ready
    if (bCanAttackNow)
    {
        ExecuteAttack();
    }
}

void ADinosaurCombatAIController::UpdateFlanking(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn()) return;

    // Move to assigned flank position
    float DistToFlank = FVector::Dist(GetPawn()->GetActorLocation(), FlankData.FlankTargetLocation);
    if (DistToFlank < 150.0f)
    {
        FlankData.bFlankPositionReached = true;
        // Once in position, switch to direct chase/attack
        UpdateChase(DeltaTime);
    }
    else
    {
        MoveToLocation(FlankData.FlankTargetLocation, 100.0f);
    }
}

// ============================================================
// Attack Execution
// ============================================================

void ADinosaurCombatAIController::ExecuteAttack()
{
    if (!CurrentTarget || !bCanAttackNow) return;

    ECombat_AttackType Attack = SelectBestAttack();

    // Apply damage to target
    float FinalDamage = CombatProfile.AttackDamage;

    // Charge attack does 2x damage
    if (Attack == ECombat_AttackType::Charge)
    {
        FinalDamage *= 2.0f;
    }

    // Pounce does 1.5x and knocks target
    if (Attack == ECombat_AttackType::Pounce)
    {
        FinalDamage *= 1.5f;
    }

    UGameplayStatics::ApplyDamage(CurrentTarget, FinalDamage, this, GetPawn(), nullptr);

    // Start cooldown
    bCanAttackNow = false;
    TimeSinceLastAttack = 0.0f;
}

ECombat_AttackType ADinosaurCombatAIController::SelectBestAttack() const
{
    if (!CurrentTarget || !GetPawn()) return CombatProfile.PrimaryAttack;

    float Dist = GetDistanceToTarget();

    // Use charge if target is at medium range and we have it
    if (CombatProfile.SecondaryAttack == ECombat_AttackType::Charge && Dist > CombatProfile.AttackRadius * 0.5f)
    {
        return ECombat_AttackType::Charge;
    }

    // Raptors pounce when target is slightly out of melee
    if (CombatProfile.PrimaryAttack == ECombat_AttackType::Pounce && Dist > CombatProfile.AttackRadius * 0.3f)
    {
        return ECombat_AttackType::Pounce;
    }

    return CombatProfile.PrimaryAttack;
}

// ============================================================
// Pack Coordination
// ============================================================

void ADinosaurCombatAIController::NotifyPackOfTarget(AActor* Target)
{
    if (!Target) return;

    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ADinosaurCombatAIController* Member = PackMembers[i];
        if (!Member || Member == this) continue;

        Member->SetTarget(Target);

        // Assign flanking roles to pack members
        if (Member->TacticalRole == ECombat_TacticalRole::Flanker)
        {
            float FlankAngle = (i % 2 == 0) ? 90.0f : -90.0f;
            Member->AssignFlankPosition(FlankAngle, CombatProfile.AttackRadius * 2.0f);
        }
        else if (Member->TacticalRole == ECombat_TacticalRole::Distractor)
        {
            // Distractor charges directly to draw attention
            Member->TacticalRole = ECombat_TacticalRole::PackMember;
        }
    }
}

void ADinosaurCombatAIController::AssignFlankPosition(float AngleDegrees, float FlankRadius)
{
    if (!CurrentTarget) return;

    FVector TargetLoc = CurrentTarget->GetActorLocation();
    float AngleRad = FMath::DegreesToRadians(AngleDegrees);

    FlankData.FlankTargetLocation = FVector(
        TargetLoc.X + FMath::Cos(AngleRad) * FlankRadius,
        TargetLoc.Y + FMath::Sin(AngleRad) * FlankRadius,
        TargetLoc.Z
    );
    FlankData.FlankAngleDegrees = AngleDegrees;
    FlankData.bFlankPositionReached = false;
    FlankData.Role = ECombat_TacticalRole::Flanker;
}

// ============================================================
// Target Scanning
// ============================================================

void ADinosaurCombatAIController::ScanForTargets()
{
    if (bIsInCombat && CurrentTarget) return; // Already has a target

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= CombatProfile.DetectionRadius)
    {
        SetTarget(PlayerPawn);
    }
}

// ============================================================
// Utility
// ============================================================

bool ADinosaurCombatAIController::IsTargetInAttackRange() const
{
    return GetDistanceToTarget() <= CombatProfile.AttackRadius;
}

bool ADinosaurCombatAIController::IsTargetInDetectionRange() const
{
    return GetDistanceToTarget() <= CombatProfile.DetectionRadius;
}

float ADinosaurCombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ADinosaurCombatAIController::ShouldFlee() const
{
    if (CombatProfile.FleeHealthThreshold <= 0.0f) return false;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;

    // Check health via damage interface if available
    // For now use BehaviorComp health if available
    if (BehaviorComp)
    {
        float HealthPct = BehaviorComp->GetCurrentHealthPercent();
        return HealthPct <= CombatProfile.FleeHealthThreshold;
    }

    return false;
}

void ADinosaurCombatAIController::SetMovementSpeed(float Speed)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* Char = Cast<ACharacter>(ControlledPawn);
    if (Char && Char->GetCharacterMovement())
    {
        Char->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}

void ADinosaurCombatAIController::ApplySpeciesDefaults(ECombat_DinoSpecies Species)
{
    CombatProfile.Species = Species;

    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        CombatProfile.DetectionRadius = 2000.0f;
        CombatProfile.ChaseRadius = 3000.0f;
        CombatProfile.AttackRadius = 300.0f;
        CombatProfile.AttackDamage = 80.0f;
        CombatProfile.AttackCooldown = 3.0f;
        CombatProfile.MoveSpeed_Patrol = 200.0f;
        CombatProfile.MoveSpeed_Chase = 600.0f;
        CombatProfile.FleeHealthThreshold = 0.0f; // T-Rex never flees
        CombatProfile.bCanFlanK = false;
        CombatProfile.bIsPackHunter = false;
        CombatProfile.PrimaryAttack = ECombat_AttackType::Bite;
        CombatProfile.SecondaryAttack = ECombat_AttackType::Stomp;
        TacticalRole = ECombat_TacticalRole::Lone;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        CombatProfile.DetectionRadius = 1200.0f;
        CombatProfile.ChaseRadius = 2000.0f;
        CombatProfile.AttackRadius = 180.0f;
        CombatProfile.AttackDamage = 25.0f;
        CombatProfile.AttackCooldown = 1.2f;
        CombatProfile.MoveSpeed_Patrol = 250.0f;
        CombatProfile.MoveSpeed_Chase = 700.0f;
        CombatProfile.FleeHealthThreshold = 0.15f; // Raptors retreat at 15% HP
        CombatProfile.bCanFlanK = true;
        CombatProfile.bIsPackHunter = true;
        CombatProfile.PrimaryAttack = ECombat_AttackType::Pounce;
        CombatProfile.SecondaryAttack = ECombat_AttackType::Claw;
        TacticalRole = ECombat_TacticalRole::PackMember;
        break;

    case ECombat_DinoSpecies::Triceratops:
        CombatProfile.DetectionRadius = 800.0f;
        CombatProfile.ChaseRadius = 1500.0f;
        CombatProfile.AttackRadius = 350.0f;
        CombatProfile.AttackDamage = 50.0f;
        CombatProfile.AttackCooldown = 4.0f;
        CombatProfile.MoveSpeed_Patrol = 180.0f;
        CombatProfile.MoveSpeed_Chase = 450.0f;
        CombatProfile.FleeHealthThreshold = 0.0f;
        CombatProfile.bCanFlanK = false;
        CombatProfile.bIsPackHunter = false;
        CombatProfile.PrimaryAttack = ECombat_AttackType::Charge;
        CombatProfile.SecondaryAttack = ECombat_AttackType::TailSwipe;
        TacticalRole = ECombat_TacticalRole::Lone;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        // Herbivore — only attacks if cornered
        CombatProfile.DetectionRadius = 500.0f;
        CombatProfile.ChaseRadius = 600.0f;
        CombatProfile.AttackRadius = 400.0f;
        CombatProfile.AttackDamage = 60.0f;
        CombatProfile.AttackCooldown = 5.0f;
        CombatProfile.MoveSpeed_Patrol = 100.0f;
        CombatProfile.MoveSpeed_Chase = 200.0f;
        CombatProfile.FleeHealthThreshold = 0.5f; // Flees at 50% HP
        CombatProfile.bCanFlanK = false;
        CombatProfile.bIsPackHunter = false;
        CombatProfile.PrimaryAttack = ECombat_AttackType::Stomp;
        CombatProfile.SecondaryAttack = ECombat_AttackType::TailSwipe;
        TacticalRole = ECombat_TacticalRole::Lone;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        CombatProfile.DetectionRadius = 3000.0f;
        CombatProfile.ChaseRadius = 4000.0f;
        CombatProfile.AttackRadius = 200.0f;
        CombatProfile.AttackDamage = 20.0f;
        CombatProfile.AttackCooldown = 1.5f;
        CombatProfile.MoveSpeed_Patrol = 400.0f;
        CombatProfile.MoveSpeed_Chase = 900.0f;
        CombatProfile.FleeHealthThreshold = 0.2f;
        CombatProfile.bCanFlanK = false;
        CombatProfile.bIsPackHunter = false;
        CombatProfile.PrimaryAttack = ECombat_AttackType::Claw;
        CombatProfile.SecondaryAttack = ECombat_AttackType::None;
        TacticalRole = ECombat_TacticalRole::Lone;
        break;

    default:
        break;
    }
}
