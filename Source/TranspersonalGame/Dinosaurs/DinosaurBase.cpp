// DinosaurBase.cpp — Engine Architect #02 — PROD_CYCLE_AUTO_20260701_002
// Full implementation of the DinosaurBase actor class for the prehistoric survival game.
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule sizing — default for medium dinosaur (Raptor-sized)
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Skeletal mesh setup
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());

    // Default species stats
    DinosaurSpecies = EDinosaurSpecies::Raptor;
    DinosaurBehavior = EDinosaurBehavior::Neutral;
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    AttackDamage = 25.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsAlive = true;
    bIsAggressive = false;
    bIsInPack = false;
    PackSize = 1;
    HungerLevel = 50.0f;
    FearLevel = 0.0f;
    TerritoryRadius = 2000.0f;
    PatrolRadius = 800.0f;
    CurrentState = EDinosaurAIState::Idle;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Start patrol timer
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ADinosaurBase::UpdatePatrolBehavior,
        3.0f,
        true,
        FMath::RandRange(0.5f, 2.0f)
    );

    // Start hunger timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        10.0f,
        true,
        5.0f
    );

    HomeLocation = GetActorLocation();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    // Update AI state machine
    UpdateAIStateMachine(DeltaTime);
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
        case EDinosaurSpecies::TRex:
            MaxHealth = 500.0f;
            CurrentHealth = 500.0f;
            AttackDamage = 120.0f;
            AttackRange = 250.0f;
            WalkSpeed = 200.0f;
            RunSpeed = 550.0f;
            DetectionRadius = 3000.0f;
            TerritoryRadius = 5000.0f;
            PatrolRadius = 1500.0f;
            bIsAggressive = true;
            DinosaurBehavior = EDinosaurBehavior::Predator;
            GetCapsuleComponent()->InitCapsuleSize(120.0f, 280.0f);
            break;

        case EDinosaurSpecies::Raptor:
            MaxHealth = 120.0f;
            CurrentHealth = 120.0f;
            AttackDamage = 35.0f;
            AttackRange = 130.0f;
            WalkSpeed = 350.0f;
            RunSpeed = 800.0f;
            DetectionRadius = 2000.0f;
            TerritoryRadius = 2500.0f;
            PatrolRadius = 1000.0f;
            bIsAggressive = true;
            bIsInPack = true;
            PackSize = 3;
            DinosaurBehavior = EDinosaurBehavior::Predator;
            GetCapsuleComponent()->InitCapsuleSize(45.0f, 100.0f);
            break;

        case EDinosaurSpecies::Brachiosaurus:
            MaxHealth = 800.0f;
            CurrentHealth = 800.0f;
            AttackDamage = 60.0f; // Stomp damage
            AttackRange = 200.0f;
            WalkSpeed = 150.0f;
            RunSpeed = 300.0f;
            DetectionRadius = 1000.0f;
            TerritoryRadius = 3000.0f;
            PatrolRadius = 2000.0f;
            bIsAggressive = false;
            DinosaurBehavior = EDinosaurBehavior::Herbivore;
            GetCapsuleComponent()->InitCapsuleSize(200.0f, 500.0f);
            break;

        case EDinosaurSpecies::Triceratops:
            MaxHealth = 350.0f;
            CurrentHealth = 350.0f;
            AttackDamage = 80.0f;
            AttackRange = 180.0f;
            WalkSpeed = 250.0f;
            RunSpeed = 500.0f;
            DetectionRadius = 1200.0f;
            TerritoryRadius = 2000.0f;
            PatrolRadius = 800.0f;
            bIsAggressive = false; // Defensive, not aggressive unless threatened
            DinosaurBehavior = EDinosaurBehavior::Herbivore;
            GetCapsuleComponent()->InitCapsuleSize(110.0f, 200.0f);
            break;

        case EDinosaurSpecies::Pterodactyl:
            MaxHealth = 80.0f;
            CurrentHealth = 80.0f;
            AttackDamage = 20.0f;
            AttackRange = 100.0f;
            WalkSpeed = 200.0f;
            RunSpeed = 1200.0f; // Flying speed
            DetectionRadius = 4000.0f;
            TerritoryRadius = 8000.0f;
            PatrolRadius = 3000.0f;
            bIsAggressive = false;
            DinosaurBehavior = EDinosaurBehavior::Neutral;
            GetCapsuleComponent()->InitCapsuleSize(50.0f, 80.0f);
            GetCharacterMovement()->SetMovementMode(MOVE_Flying);
            break;

        case EDinosaurSpecies::Stegosaurus:
            MaxHealth = 280.0f;
            CurrentHealth = 280.0f;
            AttackDamage = 55.0f;
            AttackRange = 160.0f;
            WalkSpeed = 180.0f;
            RunSpeed = 380.0f;
            DetectionRadius = 900.0f;
            TerritoryRadius = 1500.0f;
            PatrolRadius = 600.0f;
            bIsAggressive = false;
            DinosaurBehavior = EDinosaurBehavior::Herbivore;
            GetCapsuleComponent()->InitCapsuleSize(90.0f, 180.0f);
            break;

        default:
            break;
    }

    // Apply movement speeds
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADinosaurBase::UpdateAIStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
        case EDinosaurAIState::Idle:
            // Handled by patrol timer
            break;

        case EDinosaurAIState::Patrolling:
            // Movement handled by AI controller
            break;

        case EDinosaurAIState::Chasing:
            if (TargetActor)
            {
                float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
                if (DistToTarget <= AttackRange)
                {
                    SetAIState(EDinosaurAIState::Attacking);
                }
                else if (DistToTarget > DetectionRadius * 1.5f)
                {
                    // Lost target
                    TargetActor = nullptr;
                    SetAIState(EDinosaurAIState::Returning);
                }
            }
            else
            {
                SetAIState(EDinosaurAIState::Returning);
            }
            break;

        case EDinosaurAIState::Attacking:
            if (TargetActor)
            {
                float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
                if (DistToTarget > AttackRange * 1.5f)
                {
                    SetAIState(EDinosaurAIState::Chasing);
                }
            }
            else
            {
                SetAIState(EDinosaurAIState::Returning);
            }
            break;

        case EDinosaurAIState::Fleeing:
            // Check if threat is gone
            if (!TargetActor || FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation()) > DetectionRadius * 2.0f)
            {
                FearLevel = FMath::Max(0.0f, FearLevel - 20.0f);
                if (FearLevel < 20.0f)
                {
                    SetAIState(EDinosaurAIState::Returning);
                }
            }
            break;

        case EDinosaurAIState::Returning:
        {
            float DistToHome = FVector::Dist(GetActorLocation(), HomeLocation);
            if (DistToHome < 200.0f)
            {
                SetAIState(EDinosaurAIState::Idle);
            }
            break;
        }

        case EDinosaurAIState::Dead:
            break;
    }
}

void ADinosaurBase::UpdatePatrolBehavior()
{
    if (!bIsAlive || CurrentState == EDinosaurAIState::Dead) return;
    if (CurrentState == EDinosaurAIState::Chasing ||
        CurrentState == EDinosaurAIState::Attacking ||
        CurrentState == EDinosaurAIState::Fleeing) return;

    // Random patrol point within radius
    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.0f
    );
    PatrolTargetLocation = HomeLocation + RandomOffset;
    SetAIState(EDinosaurAIState::Patrolling);
}

void ADinosaurBase::UpdateHunger()
{
    if (!bIsAlive) return;

    HungerLevel = FMath::Min(100.0f, HungerLevel + 5.0f);

    // Hungry predators become aggressive
    if (HungerLevel > 70.0f && DinosaurBehavior == EDinosaurBehavior::Predator)
    {
        bIsAggressive = true;
        DetectionRadius *= 1.2f; // Wider search when hungry
    }
}

void ADinosaurBase::SetAIState(EDinosaurAIState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    // Adjust movement speed based on state
    switch (NewState)
    {
        case EDinosaurAIState::Chasing:
        case EDinosaurAIState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
            break;
        case EDinosaurAIState::Patrolling:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinosaurAIState::Idle:
        case EDinosaurAIState::Returning:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.6f;
            break;
        default:
            break;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // React to damage
    if (DamageCauser && DamageCauser != this)
    {
        TargetActor = DamageCauser;

        if (DinosaurBehavior == EDinosaurBehavior::Predator || bIsAggressive)
        {
            SetAIState(EDinosaurAIState::Chasing);
        }
        else
        {
            // Herbivores flee
            FearLevel = FMath::Min(100.0f, FearLevel + 40.0f);
            SetAIState(EDinosaurAIState::Fleeing);
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentState = EDinosaurAIState::Dead;

    // Stop timers
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Notify Blueprint
    OnDinosaurDied();

    // Destroy after delay (allow death animation)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Blueprint implementable — default: ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool ADinosaurBase::CanAttackTarget(AActor* Target) const
{
    if (!Target || !bIsAlive) return false;
    if (DinosaurBehavior == EDinosaurBehavior::Herbivore && !bIsAggressive) return false;

    float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Dist <= AttackRange;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!CanAttackTarget(Target)) return;

    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    HungerLevel = FMath::Max(0.0f, HungerLevel - 30.0f); // Eating reduces hunger
}

FVector ADinosaurBase::GetPatrolTarget() const
{
    return PatrolTargetLocation;
}

FVector ADinosaurBase::GetHomeLocation() const
{
    return HomeLocation;
}

void ADinosaurBase::SetHomeLocation(FVector NewHome)
{
    HomeLocation = NewHome;
}
