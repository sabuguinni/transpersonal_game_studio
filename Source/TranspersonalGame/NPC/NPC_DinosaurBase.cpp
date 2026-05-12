#include "NPC_DinosaurBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

ANPC_DinosaurBase::ANPC_DinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);

    // Create dinosaur mesh
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    DinosaurMesh->SetupAttachment(RootComponent);
    DinosaurMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Create pawn sensing component
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SetSensingInterval(0.5f);
    PawnSensing->bOnlySensePlayers = false;
    PawnSensing->bSeePawns = true;
    PawnSensing->bHearNoises = true;
    PawnSensing->SightRadius = 1500.0f;
    PawnSensing->HearingThreshold = 1200.0f;
    PawnSensing->LOSHearingThreshold = 800.0f;

    // Initialize default values
    Species = ENPC_DinosaurSpecies::TRex;
    CurrentState = ENPC_DinosaurState::Idle;
    CurrentTarget = nullptr;
    LastKnownThreat = nullptr;
    LastThreatTime = 0.0f;
    PackLeader = nullptr;
    bIsPackLeader = false;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;
    LastAttackTime = 0.0f;
    PatrolWaitTime = 0.0f;

    // Set default stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.MaxHealth = 100.0f;
    DinosaurStats.AttackDamage = 25.0f;
    DinosaurStats.MovementSpeed = 400.0f;
    DinosaurStats.DetectionRange = 1500.0f;
    DinosaurStats.AttackRange = 200.0f;
    DinosaurStats.TerritoryRadius = 2000.0f;
    DinosaurStats.Aggression = 0.5f;
    DinosaurStats.bIsPackHunter = false;
    DinosaurStats.bIsCarnivore = true;

    // Set AI controller class
    AIControllerClass = AAIController::StaticClass();
}

void ANPC_DinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind pawn sensing events
    if (PawnSensing)
    {
        PawnSensing->OnSeePawn.AddDynamic(this, &ANPC_DinosaurBase::OnPawnSeen);
        PawnSensing->OnHearNoise.AddDynamic(this, &ANPC_DinosaurBase::OnPawnLost);
    }

    // Set territory center to spawn location
    TerritoryCenter = GetActorLocation();

    // Generate initial patrol points
    GeneratePatrolPoints();

    // Configure pawn sensing based on species
    if (PawnSensing)
    {
        PawnSensing->SightRadius = DinosaurStats.DetectionRange;
        PawnSensing->HearingThreshold = DinosaurStats.DetectionRange * 0.8f;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s spawned at location %s"), 
           *UEnum::GetValueAsString(Species), *GetActorLocation().ToString());
}

void ANPC_DinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAI(DeltaTime);
    StateTimer += DeltaTime;

    // Update pack behavior if part of a pack
    if (DinosaurStats.bIsPackHunter && PackMembers.Num() > 0)
    {
        // Simple pack coordination - stay within range of pack leader
        if (PackLeader && !bIsPackLeader)
        {
            float DistanceToLeader = FVector::Dist(GetActorLocation(), PackLeader->GetActorLocation());
            if (DistanceToLeader > 1000.0f) // Too far from pack
            {
                SetTarget(PackLeader);
                SetDinosaurState(ENPC_DinosaurState::Chase);
            }
        }
    }
}

void ANPC_DinosaurBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Dinosaurs don't need player input
}

void ANPC_DinosaurBase::UpdateAI(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            ProcessIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrol:
            ProcessPatrolState(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunt:
            ProcessHuntState(DeltaTime);
            break;
        case ENPC_DinosaurState::Chase:
            ProcessChaseState(DeltaTime);
            break;
        case ENPC_DinosaurState::Attack:
            ProcessAttackState(DeltaTime);
            break;
        case ENPC_DinosaurState::Flee:
            ProcessFleeState(DeltaTime);
            break;
        default:
            break;
    }
}

void ANPC_DinosaurBase::ProcessIdleState(float DeltaTime)
{
    // After 3-5 seconds of idle, start patrolling
    if (StateTimer > FMath::RandRange(3.0f, 5.0f))
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

void ANPC_DinosaurBase::ProcessPatrolState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints();
        return;
    }

    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
    float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPoint);

    if (DistanceToTarget < 150.0f) // Reached patrol point
    {
        PatrolWaitTime += DeltaTime;
        if (PatrolWaitTime >= PATROL_WAIT_TIME)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
            PatrolWaitTime = 0.0f;
        }
    }
    else
    {
        // Move towards patrol point
        FVector Direction = (TargetPoint - GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetActorLocation() + Direction * DinosaurStats.MovementSpeed * DeltaTime;
        SetActorLocation(NewLocation);

        // Face movement direction
        FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPoint);
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 2.0f));
    }
}

void ANPC_DinosaurBase::ProcessHuntState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget <= DinosaurStats.AttackRange)
    {
        SetDinosaurState(ENPC_DinosaurState::Attack);
    }
    else if (DistanceToTarget > DinosaurStats.DetectionRange * 1.5f)
    {
        // Lost target
        SetTarget(nullptr);
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Chase);
    }
}

void ANPC_DinosaurBase::ProcessChaseState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget <= DinosaurStats.AttackRange)
    {
        SetDinosaurState(ENPC_DinosaurState::Attack);
        return;
    }

    // Move towards target
    FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float ChaseSpeed = DinosaurStats.MovementSpeed * 1.2f; // Slightly faster when chasing
    FVector NewLocation = GetActorLocation() + Direction * ChaseSpeed * DeltaTime;
    SetActorLocation(NewLocation);

    // Face target
    FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTarget->GetActorLocation());
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 3.0f));
}

void ANPC_DinosaurBase::ProcessAttackState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget > DinosaurStats.AttackRange * 1.2f)
    {
        SetDinosaurState(ENPC_DinosaurState::Chase);
        return;
    }

    // Face target
    FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTarget->GetActorLocation());
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 5.0f));

    // Attack if cooldown is over
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= ATTACK_COOLDOWN)
    {
        AttackTarget();
        LastAttackTime = CurrentTime;
    }

    // Return to hunting after attack
    if (StateTimer > 1.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Hunt);
    }
}

void ANPC_DinosaurBase::ProcessFleeState(float DeltaTime)
{
    if (!LastKnownThreat)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
        return;
    }

    // Run away from threat
    FVector FleeDirection = (GetActorLocation() - LastKnownThreat->GetActorLocation()).GetSafeNormal();
    float FleeSpeed = DinosaurStats.MovementSpeed * 1.5f;
    FVector NewLocation = GetActorLocation() + FleeDirection * FleeSpeed * DeltaTime;
    SetActorLocation(NewLocation);

    // Stop fleeing after some time or if far enough
    float DistanceToThreat = FVector::Dist(GetActorLocation(), LastKnownThreat->GetActorLocation());
    if (StateTimer > 5.0f || DistanceToThreat > DinosaurStats.DetectionRange * 2.0f)
    {
        LastKnownThreat = nullptr;
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

void ANPC_DinosaurBase::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        PatrolWaitTime = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %s"), 
               *UEnum::GetValueAsString(Species), *UEnum::GetValueAsString(CurrentState));
    }
}

void ANPC_DinosaurBase::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s acquired target: %s"), 
               *UEnum::GetValueAsString(Species), *NewTarget->GetName());
    }
}

bool ANPC_DinosaurBase::IsTargetInRange(AActor* Target, float Range) const
{
    if (!Target) return false;
    return GetDistanceToTarget(Target) <= Range;
}

bool ANPC_DinosaurBase::IsInTerritory(FVector Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= DinosaurStats.TerritoryRadius;
}

void ANPC_DinosaurBase::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 4-6 random patrol points within territory
    int32 NumPoints = FMath::RandRange(4, 6);
    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector RandomPoint = GetRandomPointInTerritory();
        PatrolPoints.Add(RandomPoint);
    }

    CurrentPatrolIndex = 0;
}

FVector ANPC_DinosaurBase::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints();
    }
    
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}

void ANPC_DinosaurBase::JoinPack(ANPC_DinosaurBase* Leader)
{
    if (!Leader || !DinosaurStats.bIsPackHunter) return;

    PackLeader = Leader;
    bIsPackLeader = false;
    
    if (Leader != this)
    {
        Leader->PackMembers.AddUnique(this);
    }
}

void ANPC_DinosaurBase::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    PackMembers.Empty();
    bIsPackLeader = false;
}

void ANPC_DinosaurBase::BroadcastToPackMembers(const FString& Message)
{
    for (ANPC_DinosaurBase* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            // Pack communication logic here
            UE_LOG(LogTemp, Log, TEXT("Pack message from %s to %s: %s"), 
                   *GetName(), *Member->GetName(), *Message);
        }
    }
}

void ANPC_DinosaurBase::AttackTarget()
{
    if (!CurrentTarget || !CanAttackTarget(CurrentTarget)) return;

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks %s for %f damage!"), 
           *UEnum::GetValueAsString(Species), *CurrentTarget->GetName(), DinosaurStats.AttackDamage);

    // Apply damage if target has health system
    // This would integrate with the damage system from other agents
}

void ANPC_DinosaurBase::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    DinosaurStats.Health = FMath::Clamp(DinosaurStats.Health - DamageAmount, 0.0f, DinosaurStats.MaxHealth);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s takes %f damage! Health: %f/%f"), 
           *UEnum::GetValueAsString(Species), DamageAmount, DinosaurStats.Health, DinosaurStats.MaxHealth);

    if (DinosaurStats.Health <= 0.0f)
    {
        // Death logic
        SetDinosaurState(ENPC_DinosaurState::Idle);
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
    else if (DamageSource)
    {
        // React to damage
        LastKnownThreat = DamageSource;
        LastThreatTime = GetWorld()->GetTimeSeconds();
        
        if (DinosaurStats.Aggression > 0.7f)
        {
            SetTarget(DamageSource);
            SetDinosaurState(ENPC_DinosaurState::Hunt);
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Flee);
        }
    }
}

bool ANPC_DinosaurBase::CanAttackTarget(AActor* Target) const
{
    return Target && IsTargetInRange(Target, DinosaurStats.AttackRange) && HasLineOfSightToTarget(Target);
}

void ANPC_DinosaurBase::OnPawnSeen(APawn* SeenPawn)
{
    if (!SeenPawn || SeenPawn == this) return;

    float DistanceToPawn = GetDistanceToTarget(SeenPawn);
    if (DistanceToPawn > DinosaurStats.DetectionRange) return;

    // React based on species and aggression
    if (DinosaurStats.bIsCarnivore && DinosaurStats.Aggression > 0.3f)
    {
        // Potential prey or threat
        if (!CurrentTarget || GetDistanceToTarget(CurrentTarget) > DistanceToPawn)
        {
            SetTarget(SeenPawn);
            SetDinosaurState(ENPC_DinosaurState::Hunt);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s spotted pawn: %s at distance %f"), 
           *UEnum::GetValueAsString(Species), *SeenPawn->GetName(), DistanceToPawn);
}

void ANPC_DinosaurBase::OnPawnLost(APawn* LostPawn)
{
    if (LostPawn == CurrentTarget)
    {
        // Lost current target
        SetTarget(nullptr);
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

FVector ANPC_DinosaurBase::GetRandomPointInTerritory() const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(200.0f, DinosaurStats.TerritoryRadius * 0.8f);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
    
    return TerritoryCenter + RandomOffset;
}

bool ANPC_DinosaurBase::HasLineOfSightToTarget(AActor* Target) const
{
    if (!Target) return false;

    FHitResult HitResult;
    FVector Start = GetActorLocation() + FVector(0, 0, 50); // Eye level
    FVector End = Target->GetActorLocation() + FVector(0, 0, 50);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    return !bHit; // No obstruction = line of sight
}

float ANPC_DinosaurBase::GetDistanceToTarget(AActor* Target) const
{
    if (!Target) return FLT_MAX;
    return FVector::Dist(GetActorLocation(), Target->GetActorLocation());
}