#include "NPC_DinosaurAI.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

ANPC_DinosaurAI::ANPC_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision component
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetSphereRadius(100.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create pawn sensing component
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SightRadius = 2000.0f;
    PawnSensingComponent->HearingThreshold = 1200.0f;
    PawnSensingComponent->LOSHearingThreshold = 1500.0f;
    PawnSensingComponent->bOnlySensePlayers = false;

    // Initialize stats based on species
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Damage = 25.0f;
    Stats.Speed = 300.0f;
    Stats.DetectionRange = 2000.0f;
    Stats.AttackRange = 200.0f;
    Stats.Hunger = 50.0f;
    Stats.Fear = 0.0f;
    Stats.Aggression = 30.0f;

    // Set default state
    CurrentState = ENPC_DinosaurState::Idle;
    StateTimer = 0.0f;
}

void ANPC_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    // Store home location
    HomeLocation = GetActorLocation();

    // Generate patrol points
    GeneratePatrolPoints();

    // Bind sensing events
    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &ANPC_DinosaurAI::OnSeePawn);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &ANPC_DinosaurAI::OnHearNoise);
    }

    // Adjust stats based on species
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.Health = 300.0f;
            Stats.MaxHealth = 300.0f;
            Stats.Damage = 75.0f;
            Stats.Speed = 400.0f;
            Stats.DetectionRange = 3000.0f;
            Stats.AttackRange = 300.0f;
            Stats.Aggression = 80.0f;
            CollisionComponent->SetSphereRadius(200.0f);
            break;

        case ENPC_DinosaurSpecies::Raptor:
            Stats.Health = 80.0f;
            Stats.MaxHealth = 80.0f;
            Stats.Damage = 35.0f;
            Stats.Speed = 600.0f;
            Stats.DetectionRange = 2500.0f;
            Stats.AttackRange = 150.0f;
            Stats.Aggression = 70.0f;
            CollisionComponent->SetSphereRadius(75.0f);
            break;

        case ENPC_DinosaurSpecies::Triceratops:
            Stats.Health = 250.0f;
            Stats.MaxHealth = 250.0f;
            Stats.Damage = 50.0f;
            Stats.Speed = 200.0f;
            Stats.DetectionRange = 1500.0f;
            Stats.AttackRange = 250.0f;
            Stats.Aggression = 40.0f;
            CollisionComponent->SetSphereRadius(150.0f);
            break;

        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.Health = 500.0f;
            Stats.MaxHealth = 500.0f;
            Stats.Damage = 30.0f;
            Stats.Speed = 150.0f;
            Stats.DetectionRange = 1000.0f;
            Stats.AttackRange = 200.0f;
            Stats.Aggression = 10.0f;
            CollisionComponent->SetSphereRadius(300.0f);
            break;

        case ENPC_DinosaurSpecies::Stegosaurus:
            Stats.Health = 200.0f;
            Stats.MaxHealth = 200.0f;
            Stats.Damage = 40.0f;
            Stats.Speed = 180.0f;
            Stats.DetectionRange = 1200.0f;
            Stats.AttackRange = 180.0f;
            Stats.Aggression = 25.0f;
            CollisionComponent->SetSphereRadius(120.0f);
            break;
    }

    // Start with idle or patrolling
    if (PatrolPoints.Num() > 0)
    {
        StartPatrolling();
    }
    else
    {
        SetState(ENPC_DinosaurState::Idle);
    }
}

void ANPC_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAI(DeltaTime);
    UpdateStats(DeltaTime);
}

void ANPC_DinosaurAI::SetState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        OnStateChanged(NewState);

        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %d"), 
               *GetName(), (int32)NewState);
    }
}

void ANPC_DinosaurAI::StartPatrolling()
{
    if (PatrolPoints.Num() > 0)
    {
        SetState(ENPC_DinosaurState::Patrolling);
        CurrentPatrolIndex = 0;
    }
}

void ANPC_DinosaurAI::StartHunting(AActor* Target)
{
    if (Target)
    {
        TargetActor = Target;
        SetState(ENPC_DinosaurState::Hunting);
        OnTargetDetected(Target);
    }
}

void ANPC_DinosaurAI::StartFleeing()
{
    TargetActor = nullptr;
    SetState(ENPC_DinosaurState::Fleeing);
}

void ANPC_DinosaurAI::Attack(AActor* Target)
{
    if (Target && GetDistanceToTarget(Target) <= Stats.AttackRange)
    {
        TargetActor = Target;
        SetState(ENPC_DinosaurState::Attacking);
        OnAttackTarget(Target);

        // Apply damage if target is a pawn
        if (APawn* TargetPawn = Cast<APawn>(Target))
        {
            // Damage logic would go here
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s attacks %s for %.1f damage"), 
                   *GetName(), *Target->GetName(), Stats.Damage);
        }
    }
}

bool ANPC_DinosaurAI::CanSeeTarget(AActor* Target)
{
    if (!Target || !PawnSensingComponent)
        return false;

    FVector StartLocation = GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLocation);

    if (Distance > Stats.DetectionRange)
        return false;

    // Line trace to check for obstacles
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
    );

    return !bHit || HitResult.GetActor() == Target;
}

float ANPC_DinosaurAI::GetDistanceToTarget(AActor* Target)
{
    if (!Target)
        return FLT_MAX;

    return FVector::Dist(GetActorLocation(), Target->GetActorLocation());
}

void ANPC_DinosaurAI::UpdateAI(float DeltaTime)
{
    StateTimer += DeltaTime;

    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            HandleIdleState(DeltaTime);
            break;

        case ENPC_DinosaurState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;

        case ENPC_DinosaurState::Hunting:
            HandleHuntingState(DeltaTime);
            break;

        case ENPC_DinosaurState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;

        case ENPC_DinosaurState::Alert:
            HandleAlertState(DeltaTime);
            break;

        case ENPC_DinosaurState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
    }
}

void ANPC_DinosaurAI::UpdateStats(float DeltaTime)
{
    // Hunger increases over time
    Stats.Hunger = FMath::Clamp(Stats.Hunger + DeltaTime * 2.0f, 0.0f, 100.0f);

    // Fear decreases over time
    Stats.Fear = FMath::Clamp(Stats.Fear - DeltaTime * 5.0f, 0.0f, 100.0f);

    // Health regeneration when not in combat
    if (CurrentState != ENPC_DinosaurState::Attacking && 
        CurrentState != ENPC_DinosaurState::Fleeing)
    {
        Stats.Health = FMath::Clamp(Stats.Health + DeltaTime * 1.0f, 0.0f, Stats.MaxHealth);
    }
}

void ANPC_DinosaurAI::HandleIdleState(float DeltaTime)
{
    if (StateTimer >= IdleTime)
    {
        if (PatrolPoints.Num() > 0)
        {
            StartPatrolling();
        }
        else
        {
            StateTimer = 0.0f;
        }
    }

    // Check for nearby players
    if (IsPlayerNearby())
    {
        SetState(ENPC_DinosaurState::Alert);
    }
}

void ANPC_DinosaurAI::HandlePatrollingState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0)
    {
        SetState(ENPC_DinosaurState::Idle);
        return;
    }

    FVector TargetLocation = PatrolPoints[CurrentPatrolIndex];
    float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetLocation);

    if (DistanceToTarget < 100.0f)
    {
        // Reached patrol point, move to next
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        SetState(ENPC_DinosaurState::Idle);
    }
    else
    {
        MoveToLocation(TargetLocation);
    }

    // Check for threats
    if (IsPlayerNearby())
    {
        SetState(ENPC_DinosaurState::Alert);
    }
}

void ANPC_DinosaurAI::HandleHuntingState(float DeltaTime)
{
    if (!TargetActor || !CanSeeTarget(TargetActor))
    {
        TargetActor = nullptr;
        OnTargetLost();
        SetState(ENPC_DinosaurState::Alert);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(TargetActor);

    if (DistanceToTarget <= Stats.AttackRange)
    {
        Attack(TargetActor);
    }
    else
    {
        MoveToLocation(TargetActor->GetActorLocation());
    }
}

void ANPC_DinosaurAI::HandleFleeingState(float DeltaTime)
{
    // Move away from threats toward home
    FVector FleeDirection = (HomeLocation - GetActorLocation()).GetSafeNormal();
    FVector FleeLocation = GetActorLocation() + FleeDirection * 1000.0f;
    
    MoveToLocation(FleeLocation);

    // Stop fleeing after some time
    if (StateTimer >= 10.0f)
    {
        SetState(ENPC_DinosaurState::Alert);
    }
}

void ANPC_DinosaurAI::HandleAlertState(float DeltaTime)
{
    if (StateTimer >= AlertTime)
    {
        if (TargetActor && CanSeeTarget(TargetActor))
        {
            if (Stats.Aggression > Stats.Fear)
            {
                StartHunting(TargetActor);
            }
            else
            {
                StartFleeing();
            }
        }
        else
        {
            StartPatrolling();
        }
    }
}

void ANPC_DinosaurAI::HandleAttackingState(float DeltaTime)
{
    if (!TargetActor)
    {
        SetState(ENPC_DinosaurState::Alert);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(TargetActor);

    if (DistanceToTarget > Stats.AttackRange * 1.5f)
    {
        StartHunting(TargetActor);
    }
    else if (StateTimer >= 2.0f)
    {
        // Attack cooldown
        SetState(ENPC_DinosaurState::Hunting);
    }
}

FVector ANPC_DinosaurAI::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
        return GetActorLocation();

    return PatrolPoints[CurrentPatrolIndex];
}

void ANPC_DinosaurAI::GeneratePatrolPoints()
{
    PatrolPoints.Empty();

    // Generate patrol points in a circle around home location
    int32 NumPoints = FMath::RandRange(3, 6);
    float AngleStep = 360.0f / NumPoints;

    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep;
        float Distance = FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        PatrolPoints.Add(HomeLocation + Offset);
    }
}

bool ANPC_DinosaurAI::IsPlayerNearby()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = GetDistanceToTarget(PlayerPawn);
            return Distance <= Stats.DetectionRange;
        }
    }
    return false;
}

void ANPC_DinosaurAI::MoveToLocation(FVector Location)
{
    FVector CurrentLocation = GetActorLocation();
    FVector Direction = (Location - CurrentLocation).GetSafeNormal();
    
    // Simple movement - in a real game you'd use AI movement component
    FVector NewLocation = CurrentLocation + Direction * Stats.Speed * GetWorld()->GetDeltaSeconds();
    SetActorLocation(NewLocation);

    // Face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(NewRotation);
    }
}

void ANPC_DinosaurAI::OnSeePawn(APawn* Pawn)
{
    if (Pawn && Pawn != this)
    {
        // Check if it's a player
        if (Pawn->IsPlayerControlled())
        {
            TargetActor = Pawn;
            
            if (CurrentState == ENPC_DinosaurState::Idle || 
                CurrentState == ENPC_DinosaurState::Patrolling)
            {
                SetState(ENPC_DinosaurState::Alert);
            }
        }
    }
}

void ANPC_DinosaurAI::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (NoiseInstigator && Volume > 0.5f)
    {
        // React to loud noises
        if (CurrentState == ENPC_DinosaurState::Idle)
        {
            SetState(ENPC_DinosaurState::Alert);
        }
    }
}