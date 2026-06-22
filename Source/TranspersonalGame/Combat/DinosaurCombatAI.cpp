#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz for AI tick — performance friendly
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    InitializeForSpecies(Species);
    if (PatrolWaypoints.Num() > 0)
    {
        SetCombatState(ECombat_DinoState::Patrol);
    }
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        SetCombatState(ECombat_DinoState::Dead);
        return;
    }

    AttackTimer = FMath::Max(0.0f, AttackTimer - DeltaTime);

    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:    UpdateIdle(DeltaTime);    break;
        case ECombat_DinoState::Patrol:  UpdatePatrol(DeltaTime);  break;
        case ECombat_DinoState::Alert:   UpdateAlert(DeltaTime);   break;
        case ECombat_DinoState::Chase:   UpdateChase(DeltaTime);   break;
        case ECombat_DinoState::Attack:  UpdateAttack(DeltaTime);  break;
        case ECombat_DinoState::Flank:   UpdateFlank(DeltaTime);   break;
        case ECombat_DinoState::Retreat: UpdateRetreat(DeltaTime); break;
        default: break;
    }
}

void UDinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    UE_LOG(LogTemp, Verbose, TEXT("DinosaurCombatAI [%s]: State -> %d"),
        *GetOwner()->GetActorLabel(), (int32)NewState);
}

void UDinosaurCombatAI::DetectPlayer(AActor* PlayerActor)
{
    if (!PlayerActor || !IsAlive()) return;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    if (Dist <= DinoStats.DetectionRadius)
    {
        CurrentTarget = PlayerActor;
        if (DinoStats.bIsPackHunter)
        {
            NotifyPackMembers(PlayerActor);
            SetCombatState(ECombat_DinoState::Flank);
        }
        else
        {
            SetCombatState(ECombat_DinoState::Chase);
        }
    }
}

void UDinosaurCombatAI::ExecuteAttack()
{
    if (!CurrentTarget || AttackTimer > 0.0f) return;

    if (IsPlayerInRange(DinoStats.AttackRange))
    {
        // Apply damage to player
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            DinoStats.AttackDamage,
            nullptr,
            GetOwner(),
            nullptr
        );
        AttackTimer = DinoStats.AttackCooldown;
        UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Attack hit for %.1f damage"), DinoStats.AttackDamage);
    }
}

void UDinosaurCombatAI::TakeDamage_Combat(float DamageAmount)
{
    if (!IsAlive()) return;

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Took %.1f damage, HP: %.1f/%.1f"),
        DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    if (!IsAlive())
    {
        SetCombatState(ECombat_DinoState::Dead);
        return;
    }

    // Retreat if critically wounded (below 20% HP)
    float HealthPct = DinoStats.CurrentHealth / DinoStats.MaxHealth;
    if (HealthPct < 0.20f && CurrentState != ECombat_DinoState::Retreat)
    {
        SetCombatState(ECombat_DinoState::Retreat);
    }
    else if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrol)
    {
        SetCombatState(ECombat_DinoState::Alert);
    }
}

void UDinosaurCombatAI::NotifyPackMembers(AActor* Threat)
{
    if (!Threat) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find nearby pack members (same species within 2000 units)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    int32 PackIndex = 0;
    int32 PackCount = 0;

    // Count pack members first
    for (AActor* Actor : AllActors)
    {
        if (Actor == GetOwner()) continue;
        UDinosaurCombatAI* OtherAI = Actor->FindComponentByClass<UDinosaurCombatAI>();
        if (OtherAI && OtherAI->Species == Species)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Dist < 2000.0f) PackCount++;
        }
    }

    // Assign flank positions
    for (AActor* Actor : AllActors)
    {
        if (Actor == GetOwner()) continue;
        UDinosaurCombatAI* OtherAI = Actor->FindComponentByClass<UDinosaurCombatAI>();
        if (OtherAI && OtherAI->Species == Species)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Dist < 2000.0f)
            {
                OtherAI->CurrentTarget = Threat;
                OtherAI->AssignFlankPosition(PackIndex, PackCount + 1);
                OtherAI->SetCombatState(ECombat_DinoState::Flank);
                PackIndex++;
            }
        }
    }
}

void UDinosaurCombatAI::AssignFlankPosition(int32 PackIndex, int32 TotalPackSize)
{
    if (TotalPackSize <= 0 || !CurrentTarget) return;

    // Distribute flankers evenly around the target in a circle
    float AngleStep = 360.0f / FMath::Max(TotalPackSize, 1);
    float Angle = FMath::DegreesToRadians(AngleStep * PackIndex);
    float FlankRadius = DinoStats.AttackRange * 1.5f;

    FlankOffset = FVector(
        FMath::Cos(Angle) * FlankRadius,
        FMath::Sin(Angle) * FlankRadius,
        0.0f
    );
}

void UDinosaurCombatAI::AddPatrolWaypoint(FVector Location, float WaitTime)
{
    FCombat_PatrolWaypoint WP;
    WP.Location = Location;
    WP.WaitTime = WaitTime;
    PatrolWaypoints.Add(WP);
}

void UDinosaurCombatAI::StartPatrol()
{
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentWaypointIndex = 0;
        SetCombatState(ECombat_DinoState::Patrol);
    }
}

void UDinosaurCombatAI::InitializeForSpecies(ECombat_DinoSpecies InSpecies)
{
    Species = InSpecies;
    switch (InSpecies)
    {
        case ECombat_DinoSpecies::TyrannosaurusRex:
            DinoStats.MaxHealth = 2000.0f;
            DinoStats.CurrentHealth = 2000.0f;
            DinoStats.AttackDamage = 200.0f;
            DinoStats.AttackRange = 300.0f;
            DinoStats.DetectionRadius = 3000.0f;
            DinoStats.MoveSpeed = 500.0f;
            DinoStats.ChaseSpeed = 700.0f;
            DinoStats.AttackCooldown = 3.0f;
            DinoStats.bIsPackHunter = false;
            DinoStats.PackSize = 1;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            DinoStats.MaxHealth = 300.0f;
            DinoStats.CurrentHealth = 300.0f;
            DinoStats.AttackDamage = 60.0f;
            DinoStats.AttackRange = 150.0f;
            DinoStats.DetectionRadius = 2000.0f;
            DinoStats.MoveSpeed = 800.0f;
            DinoStats.ChaseSpeed = 1100.0f;
            DinoStats.AttackCooldown = 1.2f;
            DinoStats.bIsPackHunter = true;
            DinoStats.PackSize = 3;
            break;

        case ECombat_DinoSpecies::Triceratops:
            DinoStats.MaxHealth = 1200.0f;
            DinoStats.CurrentHealth = 1200.0f;
            DinoStats.AttackDamage = 120.0f;
            DinoStats.AttackRange = 250.0f;
            DinoStats.DetectionRadius = 1200.0f;
            DinoStats.MoveSpeed = 450.0f;
            DinoStats.ChaseSpeed = 600.0f;
            DinoStats.AttackCooldown = 2.5f;
            DinoStats.bIsPackHunter = false;
            DinoStats.PackSize = 1;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            DinoStats.MaxHealth = 3000.0f;
            DinoStats.CurrentHealth = 3000.0f;
            DinoStats.AttackDamage = 50.0f; // Stomp
            DinoStats.AttackRange = 400.0f;
            DinoStats.DetectionRadius = 800.0f;
            DinoStats.MoveSpeed = 300.0f;
            DinoStats.ChaseSpeed = 350.0f;
            DinoStats.AttackCooldown = 4.0f;
            DinoStats.bIsPackHunter = false;
            DinoStats.PackSize = 1;
            break;

        case ECombat_DinoSpecies::Stegosaurus:
            DinoStats.MaxHealth = 900.0f;
            DinoStats.CurrentHealth = 900.0f;
            DinoStats.AttackDamage = 90.0f;
            DinoStats.AttackRange = 200.0f;
            DinoStats.DetectionRadius = 1000.0f;
            DinoStats.MoveSpeed = 400.0f;
            DinoStats.ChaseSpeed = 500.0f;
            DinoStats.AttackCooldown = 2.0f;
            DinoStats.bIsPackHunter = false;
            DinoStats.PackSize = 1;
            break;
    }
}

// === PRIVATE STATE UPDATES ===

void UDinosaurCombatAI::UpdateIdle(float DeltaTime)
{
    // Scan for player every tick
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        DetectPlayer(PlayerPawn);
    }
}

void UDinosaurCombatAI::UpdatePatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    // Check for player while patrolling
    UWorld* World = GetWorld();
    if (World)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
            if (Dist <= DinoStats.DetectionRadius)
            {
                DetectPlayer(PlayerPawn);
                return;
            }
        }
    }

    // Move to next waypoint
    if (bWaitingAtWaypoint)
    {
        WaypointWaitTimer -= DeltaTime;
        if (WaypointWaitTimer <= 0.0f)
        {
            bWaitingAtWaypoint = false;
            CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        }
        return;
    }

    FVector WPLocation = PatrolWaypoints[CurrentWaypointIndex].Location;
    float DistToWP = FVector::Dist(GetOwner()->GetActorLocation(), WPLocation);

    if (DistToWP < 100.0f)
    {
        bWaitingAtWaypoint = true;
        WaypointWaitTimer = PatrolWaypoints[CurrentWaypointIndex].WaitTime;
    }
    else
    {
        // Move toward waypoint
        FVector Dir = (WPLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetOwner()->GetActorLocation() + Dir * DinoStats.MoveSpeed * DeltaTime;
        GetOwner()->SetActorLocation(NewLocation, true);
    }
}

void UDinosaurCombatAI::UpdateAlert(float DeltaTime)
{
    // Brief alert state before committing to chase
    if (CurrentTarget)
    {
        SetCombatState(DinoStats.bIsPackHunter ? ECombat_DinoState::Flank : ECombat_DinoState::Chase);
    }
    else
    {
        SetCombatState(ECombat_DinoState::Patrol);
    }
}

void UDinosaurCombatAI::UpdateChase(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist <= DinoStats.AttackRange)
    {
        SetCombatState(ECombat_DinoState::Attack);
        return;
    }

    // Lost target — too far
    if (Dist > DinoStats.DetectionRadius * 2.0f)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    MoveTowardsTarget(DinoStats.ChaseSpeed, DeltaTime);
}

void UDinosaurCombatAI::UpdateAttack(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist > DinoStats.AttackRange * 1.5f)
    {
        SetCombatState(ECombat_DinoState::Chase);
        return;
    }

    ExecuteAttack();
}

void UDinosaurCombatAI::UpdateFlank(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    // Move to flank position
    FVector TargetFlankPos = CurrentTarget->GetActorLocation() + FlankOffset;
    float DistToFlank = FVector::Dist(GetOwner()->GetActorLocation(), TargetFlankPos);

    if (DistToFlank < 100.0f)
    {
        // Reached flank position — now attack
        SetCombatState(ECombat_DinoState::Attack);
        return;
    }

    MoveTowardsFlankPosition(DeltaTime);
}

void UDinosaurCombatAI::UpdateRetreat(float DeltaTime)
{
    if (!CurrentTarget) return;

    // Move away from target
    FVector AwayDir = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + AwayDir * DinoStats.MoveSpeed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);

    float Dist = GetDistanceToTarget();
    if (Dist > DinoStats.DetectionRadius * 1.5f)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Patrol);
    }
}

// === PRIVATE HELPERS ===

float UDinosaurCombatAI::GetDistanceToTarget() const
{
    if (!CurrentTarget) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UDinosaurCombatAI::IsPlayerInRange(float Range) const
{
    return GetDistanceToTarget() <= Range;
}

void UDinosaurCombatAI::MoveTowardsTarget(float Speed, float DeltaTime)
{
    if (!CurrentTarget) return;
    FVector Dir = (CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Dir * Speed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);

    // Face target
    FRotator LookAt = Dir.Rotation();
    GetOwner()->SetActorRotation(LookAt);
}

void UDinosaurCombatAI::MoveTowardsFlankPosition(float DeltaTime)
{
    if (!CurrentTarget) return;
    FVector TargetFlankPos = CurrentTarget->GetActorLocation() + FlankOffset;
    FVector Dir = (TargetFlankPos - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Dir * DinoStats.ChaseSpeed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);
    GetOwner()->SetActorRotation(Dir.Rotation());
}
