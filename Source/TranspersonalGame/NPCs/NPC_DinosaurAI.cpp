#include "NPC_DinosaurAI.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ANPC_DinosaurAI::ANPC_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);

    // Create mesh component
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    DinosaurMesh->SetupAttachment(RootComponent);

    // Create pawn sensing component
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 2000.0f;
    PawnSensing->HearingThreshold = 1.0f;
    PawnSensing->LOSHearingThreshold = 1.5f;
    PawnSensing->SensingInterval = 0.5f;
    PawnSensing->SetPeripheralVisionAngle(90.0f);

    // Bind perception events
    PawnSensing->OnSeePawn.AddDynamic(this, &ANPC_DinosaurAI::OnPlayerSeen);
    PawnSensing->OnHearNoise.AddDynamic(this, &ANPC_DinosaurAI::OnPlayerHeard);

    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 50.0f;
    Stats.Stamina = 100.0f;
    Stats.AttackDamage = 25.0f;
    Stats.MovementSpeed = 300.0f;
    Stats.DetectionRange = 2000.0f;
    Stats.AttackRange = 200.0f;

    CurrentTarget = nullptr;
    StateChangeTimer = 0.0f;
    IdleTime = 5.0f;
    PatrolRadius = 1500.0f;
}

void ANPC_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Store home location
    HomeLocation = GetActorLocation();
    PatrolDestination = HomeLocation;
    
    // Initialize species-specific stats
    InitializeSpeciesStats();
    
    // Start with idle state
    SetDinosaurState(ENPC_DinosaurState::Idle);
}

void ANPC_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAIBehavior(DeltaTime);
}

void ANPC_DinosaurAI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANPC_DinosaurAI::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %d"), 
               *GetName(), (int32)NewState);
    }
}

void ANPC_DinosaurAI::StartPatrolling()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    PatrolDestination = GetRandomPatrolPoint();
}

void ANPC_DinosaurAI::StartHunting(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        SetDinosaurState(ENPC_DinosaurState::Hunting);
    }
}

void ANPC_DinosaurAI::StartFleeing(AActor* Threat)
{
    if (Threat)
    {
        CurrentTarget = Threat;
        SetDinosaurState(ENPC_DinosaurState::Fleeing);
        
        // Calculate flee direction (opposite to threat)
        FVector FleeDirection = (GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
        PatrolDestination = GetActorLocation() + (FleeDirection * PatrolRadius);
    }
}

void ANPC_DinosaurAI::AttackTarget()
{
    if (CurrentTarget && GetDistanceToPlayer() <= Stats.AttackRange)
    {
        // Apply damage to target if it's a character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
        {
            // In a real implementation, this would use a damage system
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks %s for %.1f damage!"), 
                   *GetName(), *CurrentTarget->GetName(), Stats.AttackDamage);
        }
    }
}

void ANPC_DinosaurAI::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health -= DamageAmount;
    
    if (Stats.Health <= 0.0f)
    {
        Stats.Health = 0.0f;
        SetDinosaurState(ENPC_DinosaurState::Dead);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s has died!"), *GetName());
    }
    else if (DamageSource)
    {
        // React to damage based on species
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            StartHunting(DamageSource);
        }
        else
        {
            StartFleeing(DamageSource);
        }
    }
}

bool ANPC_DinosaurAI::IsPlayerInRange() const
{
    return GetDistanceToPlayer() <= Stats.DetectionRange;
}

float ANPC_DinosaurAI::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            }
        }
    }
    return 9999.0f;
}

void ANPC_DinosaurAI::InitializeSpeciesStats()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            ApplyTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            ApplyVelociraptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            ApplyTriceratopsBehavior();
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            ApplyBrachiosaurusBehavior();
            break;
        case ENPC_DinosaurSpecies::Ankylosaurus:
            ApplyAnkylosaurusBehavior();
            break;
    }
}

void ANPC_DinosaurAI::UpdateAIBehavior(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            UpdateIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            UpdatePatrollingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunting:
            UpdateHuntingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Fleeing:
            UpdateFleeingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Feeding:
            UpdateFeedingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Dead:
            // Dead dinosaurs don't move
            break;
    }
}

void ANPC_DinosaurAI::OnPlayerSeen(APawn* SeenPawn)
{
    if (SeenPawn && CurrentState != ENPC_DinosaurState::Dead)
    {
        float Distance = FVector::Dist(GetActorLocation(), SeenPawn->GetActorLocation());
        
        // React based on species and distance
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            if (Distance <= Stats.DetectionRange)
            {
                StartHunting(SeenPawn);
            }
        }
        else if (Distance <= Stats.DetectionRange * 0.5f)
        {
            // Herbivores flee when player gets too close
            StartFleeing(SeenPawn);
        }
    }
}

void ANPC_DinosaurAI::OnPlayerHeard(APawn* HeardPawn, const FVector& Location, float Volume)
{
    if (HeardPawn && CurrentState == ENPC_DinosaurState::Idle && Volume > 0.5f)
    {
        // Look towards the sound
        FVector LookDirection = (Location - GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = LookDirection.Rotation();
        SetActorRotation(NewRotation);
        
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
    }
}

void ANPC_DinosaurAI::UpdateIdleState(float DeltaTime)
{
    if (StateChangeTimer >= IdleTime)
    {
        StartPatrolling();
    }
}

void ANPC_DinosaurAI::UpdatePatrollingState(float DeltaTime)
{
    MoveTowardsTarget(PatrolDestination, DeltaTime);
    
    if (HasReachedDestination())
    {
        SetDinosaurState(ENPC_DinosaurState::Idle);
    }
    
    // Check for player
    if (IsPlayerInRange())
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    OnPlayerSeen(PlayerPawn);
                }
            }
        }
    }
}

void ANPC_DinosaurAI::UpdateHuntingState(float DeltaTime)
{
    if (CurrentTarget)
    {
        float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (DistanceToTarget <= Stats.AttackRange)
        {
            AttackTarget();
        }
        else if (DistanceToTarget <= Stats.DetectionRange)
        {
            MoveTowardsTarget(CurrentTarget->GetActorLocation(), DeltaTime);
        }
        else
        {
            // Lost target, return to patrolling
            CurrentTarget = nullptr;
            StartPatrolling();
        }
    }
    else
    {
        StartPatrolling();
    }
}

void ANPC_DinosaurAI::UpdateFleeingState(float DeltaTime)
{
    MoveTowardsTarget(PatrolDestination, DeltaTime);
    
    if (HasReachedDestination() || StateChangeTimer >= 10.0f)
    {
        CurrentTarget = nullptr;
        SetDinosaurState(ENPC_DinosaurState::Idle);
    }
}

void ANPC_DinosaurAI::UpdateFeedingState(float DeltaTime)
{
    if (StateChangeTimer >= 8.0f)
    {
        Stats.Hunger = FMath::Min(Stats.Hunger + 25.0f, 100.0f);
        SetDinosaurState(ENPC_DinosaurState::Idle);
    }
}

FVector ANPC_DinosaurAI::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    return HomeLocation + (RandomDirection * RandomDistance);
}

bool ANPC_DinosaurAI::HasReachedDestination() const
{
    return FVector::Dist(GetActorLocation(), PatrolDestination) <= 150.0f;
}

void ANPC_DinosaurAI::MoveTowardsTarget(const FVector& TargetLocation, float DeltaTime)
{
    FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetActorLocation() + (Direction * Stats.MovementSpeed * DeltaTime);
    
    SetActorLocation(NewLocation);
    
    // Rotate to face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(NewRotation);
    }
}

void ANPC_DinosaurAI::ApplyTRexBehavior()
{
    Stats.MaxHealth = 200.0f;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage = 50.0f;
    Stats.MovementSpeed = 250.0f;
    Stats.DetectionRange = 3000.0f;
    Stats.AttackRange = 300.0f;
    PatrolRadius = 2000.0f;
    IdleTime = 3.0f;
}

void ANPC_DinosaurAI::ApplyVelociraptorBehavior()
{
    Stats.MaxHealth = 80.0f;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage = 30.0f;
    Stats.MovementSpeed = 450.0f;
    Stats.DetectionRange = 2500.0f;
    Stats.AttackRange = 150.0f;
    PatrolRadius = 1800.0f;
    IdleTime = 2.0f;
}

void ANPC_DinosaurAI::ApplyTriceratopsBehavior()
{
    Stats.MaxHealth = 150.0f;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage = 40.0f;
    Stats.MovementSpeed = 200.0f;
    Stats.DetectionRange = 1500.0f;
    Stats.AttackRange = 250.0f;
    PatrolRadius = 1200.0f;
    IdleTime = 8.0f;
}

void ANPC_DinosaurAI::ApplyBrachiosaurusBehavior()
{
    Stats.MaxHealth = 300.0f;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage = 20.0f;
    Stats.MovementSpeed = 150.0f;
    Stats.DetectionRange = 1000.0f;
    Stats.AttackRange = 400.0f;
    PatrolRadius = 800.0f;
    IdleTime = 12.0f;
}

void ANPC_DinosaurAI::ApplyAnkylosaurusBehavior()
{
    Stats.MaxHealth = 180.0f;
    Stats.Health = Stats.MaxHealth;
    Stats.AttackDamage = 35.0f;
    Stats.MovementSpeed = 180.0f;
    Stats.DetectionRange = 1200.0f;
    Stats.AttackRange = 200.0f;
    PatrolRadius = 1000.0f;
    IdleTime = 10.0f;
}