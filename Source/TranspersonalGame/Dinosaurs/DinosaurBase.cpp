#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // AI Perception — sight + hearing
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = Stats.DetectionRadius;
    SightConfig->LoseSightRadius = Stats.DetectionRadius * 1.25f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    PerceptionComponent->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = Stats.DetectionRadius * 0.75f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Movement defaults — overridden by species
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Territory defaults to spawn location (set in BeginPlay)
    TerritoryCenter = FVector::ZeroVector;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    TerritoryCenter = GetActorLocation();

    // Apply species-specific defaults
    InitializeSpeciesDefaults();

    // Apply movement speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Start in idle state
    SetBehaviorState(EDinosaurBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead)
    {
        return;
    }

    // Update attack cooldown
    if (TimeSinceLastAttack < AttackCooldown)
    {
        TimeSinceLastAttack += DeltaTime;
    }

    // Update hunger
    UpdateHunger(DeltaTime);
}

float ADinosaurBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead)
    {
        return 0.0f;
    }

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    // Alert pack when damaged
    if (DamageCauser && PackMembers.Num() > 0)
    {
        AlertPack(DamageCauser);
    }

    // Switch to hunting state if damaged by player
    if (DamageCauser && BehaviorState != EDinosaurBehaviorState::Hunting)
    {
        CurrentTarget = DamageCauser;
        SetBehaviorState(EDinosaurBehaviorState::Hunting);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        HandleDeath();
    }

    return DamageAmount;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || bIsDead || TimeSinceLastAttack < AttackCooldown)
    {
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
        TimeSinceLastAttack = 0.0f;
        OnAttack(Target);
    }
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    BehaviorState = NewState;

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
            case EDinosaurBehaviorState::Hunting:
                MoveComp->MaxWalkSpeed = Stats.RunSpeed;
                break;
            case EDinosaurBehaviorState::Fleeing:
                MoveComp->MaxWalkSpeed = Stats.RunSpeed * 1.1f;
                break;
            case EDinosaurBehaviorState::Resting:
                MoveComp->MaxWalkSpeed = 0.0f;
                break;
            default:
                MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
                break;
        }
    }

    OnBehaviorStateChanged(NewState);
}

bool ADinosaurBase::IsPlayerInRange(float Range) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->GetPawn())
    {
        return false;
    }

    float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
    return Dist <= Range;
}

bool ADinosaurBase::IsHungry() const
{
    return Stats.Hunger < 30.0f;
}

void ADinosaurBase::JoinPack(ADinosaurBase* Leader)
{
    if (!Leader || Leader == this)
    {
        return;
    }

    PackLeader = Leader;
    Leader->PackMembers.AddUnique(this);
}

void ADinosaurBase::AlertPack(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }

    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && Member->IsAlive() && Member != this)
        {
            Member->CurrentTarget = Threat;
            Member->SetBehaviorState(EDinosaurBehaviorState::Hunting);
        }
    }

    // Also alert leader
    if (PackLeader && PackLeader->IsAlive())
    {
        PackLeader->CurrentTarget = Threat;
        PackLeader->SetBehaviorState(EDinosaurBehaviorState::Hunting);
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return Stats.CurrentHealth / Stats.MaxHealth;
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && Stats.CurrentHealth > 0.0f;
}

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    if (!bHasTerritory)
    {
        return true;
    }
    return FVector::Dist(TerritoryCenter, Location) <= Stats.TerritoryRadius;
}

void ADinosaurBase::SetTerritoryCenter(const FVector& Center)
{
    TerritoryCenter = Center;
}

void ADinosaurBase::InitializeSpeciesDefaults()
{
    // Override in subclasses or set via Blueprint
    // Base defaults are already set in FDinosaurStats constructor
    switch (SpeciesData.Species)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
            SpeciesData.SpeciesName = TEXT("Tyrannosaurus Rex");
            SpeciesData.Diet = EDinosaurDiet::Carnivore;
            SpeciesData.bIsPack = false;
            SpeciesData.PackSize = 1;
            Stats.MaxHealth = 1000.0f;
            Stats.CurrentHealth = 1000.0f;
            Stats.AttackDamage = 120.0f;
            Stats.AttackRange = 350.0f;
            Stats.DetectionRadius = 2500.0f;
            Stats.WalkSpeed = 350.0f;
            Stats.RunSpeed = 800.0f;
            Stats.Mass = 8000.0f;
            Stats.TerritoryRadius = 5000.0f;
            break;

        case EDinosaurSpecies::Velociraptor:
            SpeciesData.SpeciesName = TEXT("Velociraptor");
            SpeciesData.Diet = EDinosaurDiet::Carnivore;
            SpeciesData.bIsPack = true;
            SpeciesData.PackSize = 4;
            Stats.MaxHealth = 200.0f;
            Stats.CurrentHealth = 200.0f;
            Stats.AttackDamage = 35.0f;
            Stats.AttackRange = 150.0f;
            Stats.DetectionRadius = 1800.0f;
            Stats.WalkSpeed = 400.0f;
            Stats.RunSpeed = 900.0f;
            Stats.Mass = 80.0f;
            Stats.TerritoryRadius = 3000.0f;
            break;

        case EDinosaurSpecies::Pterodactylus:
            SpeciesData.SpeciesName = TEXT("Pterodactylus");
            SpeciesData.Diet = EDinosaurDiet::Carnivore;
            SpeciesData.bIsFlying = true;
            SpeciesData.bIsPack = false;
            Stats.MaxHealth = 150.0f;
            Stats.CurrentHealth = 150.0f;
            Stats.AttackDamage = 20.0f;
            Stats.AttackRange = 200.0f;
            Stats.DetectionRadius = 3000.0f;
            Stats.WalkSpeed = 500.0f;
            Stats.RunSpeed = 1200.0f;
            Stats.Mass = 20.0f;
            Stats.TerritoryRadius = 6000.0f;
            break;

        case EDinosaurSpecies::Triceratops:
            SpeciesData.SpeciesName = TEXT("Triceratops");
            SpeciesData.Diet = EDinosaurDiet::Herbivore;
            SpeciesData.bIsPack = true;
            SpeciesData.PackSize = 6;
            Stats.MaxHealth = 800.0f;
            Stats.CurrentHealth = 800.0f;
            Stats.AttackDamage = 80.0f;
            Stats.AttackRange = 300.0f;
            Stats.DetectionRadius = 1200.0f;
            Stats.WalkSpeed = 280.0f;
            Stats.RunSpeed = 600.0f;
            Stats.Mass = 5000.0f;
            Stats.TerritoryRadius = 4000.0f;
            break;

        case EDinosaurSpecies::Brachiosaurus:
            SpeciesData.SpeciesName = TEXT("Brachiosaurus");
            SpeciesData.Diet = EDinosaurDiet::Herbivore;
            SpeciesData.bIsPack = true;
            SpeciesData.PackSize = 3;
            Stats.MaxHealth = 2000.0f;
            Stats.CurrentHealth = 2000.0f;
            Stats.AttackDamage = 50.0f;
            Stats.AttackRange = 500.0f;
            Stats.DetectionRadius = 1000.0f;
            Stats.WalkSpeed = 200.0f;
            Stats.RunSpeed = 400.0f;
            Stats.Mass = 50000.0f;
            Stats.TerritoryRadius = 8000.0f;
            break;

        default:
            SpeciesData.SpeciesName = TEXT("Unknown Dinosaur");
            break;
    }
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    if (SpeciesData.Diet == EDinosaurDiet::Carnivore || SpeciesData.Diet == EDinosaurDiet::Omnivore)
    {
        Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - Stats.HungerDecayRate * DeltaTime);

        // Hungry carnivore becomes more aggressive
        if (IsHungry() && BehaviorState == EDinosaurBehaviorState::Idle)
        {
            SetBehaviorState(EDinosaurBehaviorState::Foraging);
        }
    }
}

void ADinosaurBase::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    SetBehaviorState(EDinosaurBehaviorState::Dead);

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Disable collision
    SetActorEnableCollision(false);

    // Notify Blueprint
    OnDinosaurDeath();

    // Remove from pack
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackLeader = nullptr;
        }
    }

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}
