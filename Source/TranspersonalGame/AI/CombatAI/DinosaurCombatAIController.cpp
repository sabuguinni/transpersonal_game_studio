#include "DinosaurCombatAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2500.0f;
    SightConfig->LoseSightRadius = 3000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Defaults
    CurrentState = ECombat_DinoState::Patrol;
    Species = ECombat_DinoSpecies::TRex;
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    AttackCooldownTimer = 0.0f;
    bIsPackMember = false;
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAIController::OnTargetPerceptionUpdated);
    }

    if (GetPawn())
    {
        PatrolOrigin = GetPawn()->GetActorLocation();
    }

    TransitionToState(ECombat_DinoState::Patrol);
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PatrolOrigin = InPawn->GetActorLocation();

    // Apply species-specific stats
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        DinoStats.MaxHealth = 1000.0f;
        DinoStats.CurrentHealth = 1000.0f;
        DinoStats.AttackDamage = 150.0f;
        DinoStats.AttackRange = 400.0f;
        DinoStats.ChaseRange = 4000.0f;
        DinoStats.MoveSpeed = 500.0f;
        break;
    case ECombat_DinoSpecies::Raptor:
        DinoStats.MaxHealth = 250.0f;
        DinoStats.CurrentHealth = 250.0f;
        DinoStats.AttackDamage = 45.0f;
        DinoStats.AttackRange = 180.0f;
        DinoStats.ChaseRange = 3500.0f;
        DinoStats.MoveSpeed = 900.0f;
        DinoStats.AttackCooldown = 1.0f;
        break;
    case ECombat_DinoSpecies::Brachiosaurus:
        DinoStats.MaxHealth = 2000.0f;
        DinoStats.CurrentHealth = 2000.0f;
        DinoStats.AttackDamage = 200.0f;
        DinoStats.AttackRange = 600.0f;
        DinoStats.ChaseRange = 1500.0f;
        DinoStats.MoveSpeed = 350.0f;
        DinoStats.RetreatHealthThreshold = 0.0f; // Never retreats
        break;
    case ECombat_DinoSpecies::Triceratops:
        DinoStats.MaxHealth = 700.0f;
        DinoStats.CurrentHealth = 700.0f;
        DinoStats.AttackDamage = 100.0f;
        DinoStats.AttackRange = 350.0f;
        DinoStats.ChaseRange = 2500.0f;
        DinoStats.MoveSpeed = 650.0f;
        break;
    }
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    // Cooldown timer
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // State machine update
    switch (CurrentState)
    {
    case ECombat_DinoState::Patrol:
        UpdatePatrol(DeltaTime);
        break;
    case ECombat_DinoState::Chase:
        UpdateChase(DeltaTime);
        break;
    case ECombat_DinoState::Attack:
        UpdateAttack(DeltaTime);
        break;
    case ECombat_DinoState::Retreat:
        UpdateRetreat(DeltaTime);
        break;
    default:
        break;
    }
}

void ADinosaurCombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only target player characters
    ACharacter* AsCharacter = Cast<ACharacter>(Actor);
    if (!AsCharacter) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;
        if (CurrentState == ECombat_DinoState::Patrol || CurrentState == ECombat_DinoState::Alert)
        {
            TransitionToState(ECombat_DinoState::Chase);
        }
    }
    else
    {
        // Lost sight — return to patrol after brief alert
        if (CurrentState == ECombat_DinoState::Chase)
        {
            TransitionToState(ECombat_DinoState::Patrol);
            CurrentTarget = nullptr;
        }
    }
}

void ADinosaurCombatAIController::TransitionToState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    switch (NewState)
    {
    case ECombat_DinoState::Patrol:
        {
            FVector PatrolPoint = GetRandomPatrolPoint();
            MoveToLocation(PatrolPoint, 50.0f);
        }
        break;
    case ECombat_DinoState::Chase:
        if (CurrentTarget)
        {
            MoveToActor(CurrentTarget, DinoStats.AttackRange * 0.8f);
        }
        break;
    case ECombat_DinoState::Attack:
        StopMovement();
        break;
    case ECombat_DinoState::Retreat:
        {
            // Move away from target
            FVector AwayDir = (MyPawn->GetActorLocation() - (CurrentTarget ? CurrentTarget->GetActorLocation() : PatrolOrigin)).GetSafeNormal();
            FVector RetreatPoint = MyPawn->GetActorLocation() + AwayDir * 3000.0f;
            MoveToLocation(RetreatPoint, 100.0f);
        }
        break;
    case ECombat_DinoState::Dead:
        StopMovement();
        HandleDeath();
        break;
    default:
        break;
    }
}

void ADinosaurCombatAIController::UpdatePatrol(float DeltaTime)
{
    if (!GetPawn()) return;

    // If we reached destination or have no move request, pick a new patrol point
    EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        FVector PatrolPoint = GetRandomPatrolPoint();
        MoveToLocation(PatrolPoint, 50.0f);
    }
}

void ADinosaurCombatAIController::UpdateChase(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn()) return;

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // Check if in attack range
    if (DistToTarget <= DinoStats.AttackRange)
    {
        TransitionToState(ECombat_DinoState::Attack);
        return;
    }

    // Check if target escaped chase range
    if (DistToTarget > DinoStats.ChaseRange * 1.5f)
    {
        CurrentTarget = nullptr;
        TransitionToState(ECombat_DinoState::Patrol);
        return;
    }

    // Check retreat condition
    if (DinoStats.RetreatHealthThreshold > 0.0f && GetHealthPercent() < DinoStats.RetreatHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Retreat);
        return;
    }

    // Continue chasing
    MoveToActor(CurrentTarget, DinoStats.AttackRange * 0.8f);
}

void ADinosaurCombatAIController::UpdateAttack(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn()) return;

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // Target moved out of attack range — resume chase
    if (DistToTarget > DinoStats.AttackRange * 1.2f)
    {
        TransitionToState(ECombat_DinoState::Chase);
        return;
    }

    // Execute attack if cooldown expired
    if (AttackCooldownTimer <= 0.0f)
    {
        ExecuteAttack(CurrentTarget);
    }
}

void ADinosaurCombatAIController::UpdateRetreat(float DeltaTime)
{
    // If health recovered above threshold (e.g., future healing mechanic), resume patrol
    if (GetHealthPercent() > DinoStats.RetreatHealthThreshold + 0.1f)
    {
        CurrentTarget = nullptr;
        TransitionToState(ECombat_DinoState::Patrol);
    }
}

void ADinosaurCombatAIController::ExecuteAttack(AActor* Target)
{
    if (!Target || !GetPawn()) return;

    AttackCooldownTimer = DinoStats.AttackCooldown;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        DinoStats.AttackDamage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s attacked %s for %.1f damage"),
        *GetPawn()->GetName(), *Target->GetName(), DinoStats.AttackDamage);
}

void ADinosaurCombatAIController::TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator)
{
    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Took %.1f damage. Health: %.1f/%.1f"),
        DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    if (DinoStats.CurrentHealth <= 0.0f)
    {
        TransitionToState(ECombat_DinoState::Dead);
        return;
    }

    // Aggro: set attacker as target if not already chasing
    if (DamageInstigator && CurrentState != ECombat_DinoState::Attack && CurrentState != ECombat_DinoState::Chase)
    {
        CurrentTarget = DamageInstigator;
        TransitionToState(ECombat_DinoState::Chase);
    }

    // Retreat check
    if (DinoStats.RetreatHealthThreshold > 0.0f && GetHealthPercent() < DinoStats.RetreatHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Retreat);
    }
}

void ADinosaurCombatAIController::HandleDeath()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    // Enable physics for ragdoll effect on the mesh
    ACharacter* AsCharacter = Cast<ACharacter>(MyPawn);
    if (AsCharacter && AsCharacter->GetMesh())
    {
        AsCharacter->GetMesh()->SetSimulatePhysics(true);
        AsCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AsCharacter->GetCharacterMovement()->DisableMovement();
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s has died."), *MyPawn->GetName());

    // Unposess after brief delay to let ragdoll settle
    UnPossess();
}

float ADinosaurCombatAIController::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

bool ADinosaurCombatAIController::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f && CurrentState != ECombat_DinoState::Dead;
}

bool ADinosaurCombatAIController::IsTargetInRange(float Range) const
{
    if (!CurrentTarget || !GetPawn()) return false;
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()) <= Range;
}

FVector ADinosaurCombatAIController::GetRandomPatrolPoint() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return PatrolOrigin;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return PatrolOrigin;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(PatrolOrigin, DinoStats.PatrolRadius, NavLoc);
    return bFound ? NavLoc.Location : PatrolOrigin;
}

void ADinosaurCombatAIController::SetPackLeader(ADinosaurCombatAIController* Leader)
{
    PackLeader = Leader;
    bIsPackMember = (Leader != nullptr);
}
