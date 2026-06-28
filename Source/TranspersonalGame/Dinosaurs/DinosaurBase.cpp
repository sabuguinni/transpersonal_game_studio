#include "DinosaurBase.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // PawnSensing component — detects players by sight and sound
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 1500.0f;
    PawnSensing->HearingThreshold = 800.0f;
    PawnSensing->LOSHearingThreshold = 1200.0f;
    PawnSensing->SetPeripheralVisionAngle(60.0f);
    PawnSensing->bSeePawns = true;
    PawnSensing->bHearNoises = true;

    // Default movement
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    bUseControllerRotationYaw = false;

    // Initial state
    CurrentState = EEng_DinoState::Idle;
    CurrentTarget = nullptr;
    Species = EEng_DinoSpecies::TyrannosaurusRex;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();
    PatrolTarget = SpawnLocation;

    // Bind sensing delegates
    if (PawnSensing)
    {
        PawnSensing->OnSeePawn.AddDynamic(this, &ADinosaurBase::OnSeePawn);
        PawnSensing->OnHearNoise.AddDynamic(this, &ADinosaurBase::OnHearNoise);
    }

    // Start patrolling after a short delay
    FTimerHandle StartTimer;
    GetWorld()->GetTimerManager().SetTimer(StartTimer, [this]()
    {
        SetDinoState(EEng_DinoState::Patrolling);
        ChooseNewPatrolPoint();
    }, 2.0f, false);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    StateTimer += DeltaTime;

    // Hunger decay
    DinoStats.Hunger = FMath::Max(0.0f, DinoStats.Hunger - DinoStats.HungerDecayRate * DeltaTime);

    switch (CurrentState)
    {
        case EEng_DinoState::Idle:
            TickIdle(DeltaTime);
            break;
        case EEng_DinoState::Patrolling:
            TickPatrol(DeltaTime);
            break;
        case EEng_DinoState::Chasing:
            TickChase(DeltaTime);
            break;
        case EEng_DinoState::Attacking:
            TickAttack(DeltaTime);
            break;
        case EEng_DinoState::Fleeing:
            TickFlee(DeltaTime);
            break;
        default:
            break;
    }
}

void ADinosaurBase::SetDinoState(EEng_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    // Adjust movement speed based on state
    switch (NewState)
    {
        case EEng_DinoState::Chasing:
        case EEng_DinoState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed;
            break;
        case EEng_DinoState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed * 1.2f;
            break;
        default:
            GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
            break;
    }
}

void ADinosaurBase::TickIdle(float DeltaTime)
{
    // After 5 seconds of idle, start patrolling
    if (StateTimer > 5.0f)
    {
        SetDinoState(EEng_DinoState::Patrolling);
        ChooseNewPatrolPoint();
    }
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    // Move toward patrol target
    FVector CurrentLoc = GetActorLocation();
    float DistToTarget = FVector::Dist2D(CurrentLoc, PatrolTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — idle briefly then pick new point
        SetDinoState(EEng_DinoState::Idle);
    }
    else
    {
        // Move toward patrol target using AI controller
        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            AIC->MoveToLocation(PatrolTarget, 150.0f);
        }
    }

    // If hungry and carnivore, be more aggressive
    if (DinoStats.bIsCarnivore && DinoStats.Hunger < 30.0f && CurrentTarget)
    {
        SetDinoState(EEng_DinoState::Chasing);
    }
}

void ADinosaurBase::TickChase(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive())
    {
        SetDinoState(EEng_DinoState::Patrolling);
        return;
    }

    // Check if target is still valid
    if (CurrentTarget->IsActorBeingDestroyed())
    {
        CurrentTarget = nullptr;
        SetDinoState(EEng_DinoState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Too far — give up chase
    if (DistToTarget > DinoStats.DetectionRange * 1.5f)
    {
        CurrentTarget = nullptr;
        SetDinoState(EEng_DinoState::Patrolling);
        ChooseNewPatrolPoint();
        return;
    }

    // In attack range — switch to attacking
    if (IsTargetInAttackRange())
    {
        SetDinoState(EEng_DinoState::Attacking);
        return;
    }

    // Check flee condition
    if (GetHealthPercent() < FleeHealthThreshold)
    {
        SetDinoState(EEng_DinoState::Fleeing);
        return;
    }

    // Move toward target
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->MoveToActor(CurrentTarget, DinoStats.AttackRange * 0.8f);
    }
}

void ADinosaurBase::TickAttack(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive())
    {
        SetDinoState(EEng_DinoState::Patrolling);
        return;
    }

    // Check flee condition
    if (GetHealthPercent() < FleeHealthThreshold)
    {
        SetDinoState(EEng_DinoState::Fleeing);
        return;
    }

    // Target moved out of range
    if (!IsTargetInAttackRange())
    {
        SetDinoState(EEng_DinoState::Chasing);
        return;
    }

    // Attack on cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        AttackTarget(CurrentTarget);
        LastAttackTime = CurrentTime;
    }
}

void ADinosaurBase::TickFlee(float DeltaTime)
{
    // Flee away from the threat
    if (CurrentTarget)
    {
        FVector FleeDir = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetActorLocation() + FleeDir * 2000.0f;

        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            AIC->MoveToLocation(FleeTarget, 100.0f);
        }
    }

    // After fleeing far enough or recovering health, resume patrol
    if (StateTimer > 10.0f || GetHealthPercent() > 0.5f)
    {
        CurrentTarget = nullptr;
        SetDinoState(EEng_DinoState::Patrolling);
        ChooseNewPatrolPoint();
    }
}

void ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageSource)
{
    if (!IsAlive()) return;

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    // React to damage — chase attacker
    if (DamageSource && DinoStats.bIsCarnivore)
    {
        CurrentTarget = DamageSource;
        SetDinoState(EEng_DinoState::Chasing);
    }

    if (!IsAlive())
    {
        Die();
    }
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!Target) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, DinoStats.AttackDamage, GetController(), this, nullptr);
}

bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

void ADinosaurBase::SetPatrolTarget(FVector NewTarget)
{
    PatrolTarget = NewTarget;
}

void ADinosaurBase::OnSeePawn(APawn* SeenPawn)
{
    if (!SeenPawn || !IsAlive()) return;

    // Only react to player character
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (SeenPawn != PlayerPawn) return;

    // Carnivores chase on sight; herbivores flee
    if (DinoStats.bIsCarnivore)
    {
        CurrentTarget = SeenPawn;
        if (CurrentState != EEng_DinoState::Attacking)
        {
            SetDinoState(EEng_DinoState::Chasing);
        }
    }
    else
    {
        // Herbivore — flee from player
        CurrentTarget = SeenPawn;
        SetDinoState(EEng_DinoState::Fleeing);
    }
}

void ADinosaurBase::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (!NoiseInstigator || !IsAlive()) return;
    if (CurrentState == EEng_DinoState::Chasing || CurrentState == EEng_DinoState::Attacking) return;

    // Investigate the sound location
    PatrolTarget = Location;
    SetDinoState(EEng_DinoState::Investigating);

    // After a moment, switch to patrolling
    FTimerHandle InvestigateTimer;
    GetWorld()->GetTimerManager().SetTimer(InvestigateTimer, [this]()
    {
        if (CurrentState == EEng_DinoState::Investigating)
        {
            SetDinoState(EEng_DinoState::Patrolling);
            ChooseNewPatrolPoint();
        }
    }, 5.0f, false);
}

void ADinosaurBase::ChooseNewPatrolPoint()
{
    // Pick a random point within patrol radius of spawn location
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation RandomPoint;
        if (NavSys->GetRandomReachablePointInRadius(SpawnLocation, PatrolRadius, RandomPoint))
        {
            PatrolTarget = RandomPoint.Location;
            return;
        }
    }

    // Fallback — random offset from spawn
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist = FMath::RandRange(200.0f, PatrolRadius);
    PatrolTarget = SpawnLocation + FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                                           FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
                                           0.0f);
}

bool ADinosaurBase::IsTargetInAttackRange() const
{
    if (!CurrentTarget) return false;
    return FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) <= DinoStats.AttackRange;
}

void ADinosaurBase::Die()
{
    // Disable collision and movement
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);

    // Destroy after 10 seconds (allow ragdoll/death animation time)
    SetLifeSpan(10.0f);
}
