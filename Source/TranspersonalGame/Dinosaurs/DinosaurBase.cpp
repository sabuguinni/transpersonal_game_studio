// DinosaurBase.cpp — Engine Architect #02 — PROD_CYCLE_AUTO_20260630_001
// Base class for all dinosaur actors in the prehistoric survival game.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default species and behavior
    Species = EDinosaurSpecies::Raptor;
    CurrentBehavior = EDinosaurBehavior::Idle;

    // Survival stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 300.0f;
    CurrentStamina = 300.0f;
    Hunger = 50.0f;
    Thirst = 50.0f;

    // Movement defaults (overridden per species in subclasses)
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    AttackRange = 200.0f;
    DetectionRange = 1500.0f;
    bIsPredator = true;
    bIsPackHunter = false;
    PackSize = 1;
    TerritoryRadius = 3000.0f;

    // Configure capsule
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Configure movement
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // AI Perception
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = DetectionRange * 1.5f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = DetectionRange * 0.5f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnTargetPerceptionUpdated);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina when running
    if (CurrentBehavior == EDinosaurBehavior::Chasing || CurrentBehavior == EDinosaurBehavior::Fleeing)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - DeltaTime * 20.0f);
        if (CurrentStamina <= 0.0f && CurrentBehavior == EDinosaurBehavior::Chasing)
        {
            SetBehavior(EDinosaurBehavior::Idle);
        }
    }
    else
    {
        // Recover stamina when idle/patrolling
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + DeltaTime * 10.0f);
    }

    // Passive hunger/thirst drain
    Hunger = FMath::Min(100.0f, Hunger + DeltaTime * 0.5f);
    Thirst = FMath::Min(100.0f, Thirst + DeltaTime * 0.3f);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (CurrentBehavior == EDinosaurBehavior::Idle || CurrentBehavior == EDinosaurBehavior::Patrolling)
    {
        // React to being hit — become aggressive or flee depending on predator status
        if (bIsPredator)
        {
            SetBehavior(EDinosaurBehavior::Attacking);
        }
        else
        {
            SetBehavior(EDinosaurBehavior::Fleeing);
        }
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehavior(EDinosaurBehavior NewBehavior)
{
    if (CurrentBehavior == NewBehavior) return;

    CurrentBehavior = NewBehavior;

    // Adjust movement speed based on behavior
    switch (NewBehavior)
    {
    case EDinosaurBehavior::Idle:
    case EDinosaurBehavior::Resting:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.3f;
        break;
    case EDinosaurBehavior::Patrolling:
    case EDinosaurBehavior::Foraging:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;
    case EDinosaurBehavior::Chasing:
    case EDinosaurBehavior::Fleeing:
    case EDinosaurBehavior::Attacking:
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        break;
    default:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;
    }
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (Species)
    {
    case EDinosaurSpecies::TyrannosaurusRex:
        MaxHealth = 2000.0f;
        CurrentHealth = 2000.0f;
        WalkSpeed = 250.0f;
        RunSpeed = 600.0f;
        AttackRange = 300.0f;
        DetectionRange = 2500.0f;
        bIsPredator = true;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 8000.0f;
        GetCapsuleComponent()->SetCapsuleSize(80.0f, 200.0f);
        break;

    case EDinosaurSpecies::Velociraptor:
        MaxHealth = 300.0f;
        CurrentHealth = 300.0f;
        WalkSpeed = 400.0f;
        RunSpeed = 900.0f;
        AttackRange = 150.0f;
        DetectionRange = 1800.0f;
        bIsPredator = true;
        bIsPackHunter = true;
        PackSize = 6;
        TerritoryRadius = 4000.0f;
        GetCapsuleComponent()->SetCapsuleSize(35.0f, 70.0f);
        break;

    case EDinosaurSpecies::Triceratops:
        MaxHealth = 1200.0f;
        CurrentHealth = 1200.0f;
        WalkSpeed = 280.0f;
        RunSpeed = 550.0f;
        AttackRange = 250.0f;
        DetectionRange = 1200.0f;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 3;
        TerritoryRadius = 3000.0f;
        GetCapsuleComponent()->SetCapsuleSize(70.0f, 160.0f);
        break;

    case EDinosaurSpecies::Brachiosaurus:
        MaxHealth = 3000.0f;
        CurrentHealth = 3000.0f;
        WalkSpeed = 200.0f;
        RunSpeed = 350.0f;
        AttackRange = 400.0f;
        DetectionRange = 2000.0f;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 4;
        TerritoryRadius = 6000.0f;
        GetCapsuleComponent()->SetCapsuleSize(100.0f, 280.0f);
        break;

    case EDinosaurSpecies::Pterodactyl:
        MaxHealth = 200.0f;
        CurrentHealth = 200.0f;
        WalkSpeed = 150.0f;
        RunSpeed = 1200.0f; // fly speed
        AttackRange = 100.0f;
        DetectionRange = 3000.0f;
        bIsPredator = true;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 10000.0f;
        GetCapsuleComponent()->SetCapsuleSize(30.0f, 60.0f);
        break;

    case EDinosaurSpecies::Stegosaurus:
        MaxHealth = 900.0f;
        CurrentHealth = 900.0f;
        WalkSpeed = 220.0f;
        RunSpeed = 400.0f;
        AttackRange = 200.0f;
        DetectionRange = 1000.0f;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 2;
        TerritoryRadius = 2500.0f;
        GetCapsuleComponent()->SetCapsuleSize(65.0f, 140.0f);
        break;

    case EDinosaurSpecies::Ankylosaurus:
        MaxHealth = 1500.0f;
        CurrentHealth = 1500.0f;
        WalkSpeed = 180.0f;
        RunSpeed = 320.0f;
        AttackRange = 220.0f;
        DetectionRange = 900.0f;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 2000.0f;
        GetCapsuleComponent()->SetCapsuleSize(75.0f, 130.0f);
        break;

    case EDinosaurSpecies::Spinosaurus:
        MaxHealth = 1800.0f;
        CurrentHealth = 1800.0f;
        WalkSpeed = 300.0f;
        RunSpeed = 650.0f;
        AttackRange = 280.0f;
        DetectionRange = 2200.0f;
        bIsPredator = true;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 7000.0f;
        GetCapsuleComponent()->SetCapsuleSize(85.0f, 210.0f);
        break;

    case EDinosaurSpecies::Parasaurolophus:
        MaxHealth = 700.0f;
        CurrentHealth = 700.0f;
        WalkSpeed = 350.0f;
        RunSpeed = 700.0f;
        AttackRange = 100.0f;
        DetectionRange = 2000.0f;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 8;
        TerritoryRadius = 5000.0f;
        GetCapsuleComponent()->SetCapsuleSize(55.0f, 150.0f);
        break;

    case EDinosaurSpecies::Raptor:
    default:
        // Already set in constructor
        break;
    }

    // Apply movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADinosaurBase::OnDeath()
{
    // Stop all movement
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Notify AI controller
    AController* MyController = GetController();
    if (MyController)
    {
        MyController->UnPossess();
    }

    // Enable ragdoll on skeletal mesh
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    SetLifeSpan(30.0f); // Despawn after 30 seconds
}

void ADinosaurBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // React to player detection
    APawn* PlayerPawn = Cast<APawn>(Actor);
    if (!PlayerPawn) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        if (bIsPredator && CurrentHealth > MaxHealth * 0.3f)
        {
            SetBehavior(EDinosaurBehavior::Chasing);
        }
        else if (!bIsPredator || CurrentHealth <= MaxHealth * 0.3f)
        {
            SetBehavior(EDinosaurBehavior::Fleeing);
        }
    }
    else
    {
        // Lost sight/sound — return to patrol
        if (CurrentBehavior == EDinosaurBehavior::Chasing)
        {
            SetBehavior(EDinosaurBehavior::Patrolling);
        }
    }
}
