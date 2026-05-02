#include "DinosaurBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogDinosaurBase, Log, All);

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create capsule component
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleSize(50.0f, 100.0f);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = 300.0f;
    MovementComponent->Acceleration = 1000.0f;
    MovementComponent->Deceleration = 2000.0f;

    // Initialize default values
    Species = EEng_DinosaurSpecies::TRex;
    SizeCategory = EEng_DinosaurSize::Large;
    DietType = EEng_DinosaurDiet::Carnivore;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MaxHunger = 100.0f;
    CurrentHunger = MaxHunger;
    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
    
    CurrentBehaviorState = EEng_DinosaurBehaviorState::Idle;
    DetectionRange = 1000.0f;
    AttackRange = 200.0f;
    FleeThreshold = 30.0f;
    
    TerritoryRadius = 2000.0f;
    bIsPackAnimal = false;
    PackLeader = nullptr;
    
    StateTimer = 0.0f;
    CurrentTarget = nullptr;
    HealthRegenTimer = 0.0f;
    StaminaRegenTimer = 0.0f;
    HungerDecayTimer = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDinosaurBase, Log, TEXT("DinosaurBase %s spawned with species %d"), 
           *GetName(), (int32)Species);
    
    // Initialize stats and territory
    InitializeStats();
    TerritoryCenter = GetActorLocation();
    
    // Set initial behavior state
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update behavior and stats
    UpdateBehavior(DeltaTime);
    UpdateStats(DeltaTime);
    
    StateTimer += DeltaTime;
}

void ADinosaurBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Dinosaurs don't need player input
}

void ADinosaurBase::InitializeStats()
{
    // Adjust stats based on species and size
    switch (SizeCategory)
    {
        case EEng_DinosaurSize::Small:
            MaxHealth = 50.0f;
            MaxStamina = 150.0f;
            MovementComponent->MaxSpeed = 500.0f;
            break;
        case EEng_DinosaurSize::Medium:
            MaxHealth = 100.0f;
            MaxStamina = 100.0f;
            MovementComponent->MaxSpeed = 300.0f;
            break;
        case EEng_DinosaurSize::Large:
            MaxHealth = 200.0f;
            MaxStamina = 80.0f;
            MovementComponent->MaxSpeed = 200.0f;
            break;
        case EEng_DinosaurSize::Massive:
            MaxHealth = 500.0f;
            MaxStamina = 50.0f;
            MovementComponent->MaxSpeed = 100.0f;
            break;
    }
    
    // Set current values to max
    CurrentHealth = MaxHealth;
    CurrentHunger = MaxHunger;
    CurrentStamina = MaxStamina;
    
    UE_LOG(LogDinosaurBase, Log, TEXT("Stats initialized - Health: %.1f, Stamina: %.1f, Speed: %.1f"), 
           MaxHealth, MaxStamina, MovementComponent->MaxSpeed);
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
        case EEng_DinosaurBehaviorState::Idle:
            ExecuteIdleBehavior(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Patrolling:
            ExecutePatrolBehavior(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Hunting:
            ExecuteHuntingBehavior(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Fleeing:
            ExecuteFleeingBehavior(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Resting:
            ExecuteRestingBehavior(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Feeding:
            ExecuteFeedingBehavior(DeltaTime);
            break;
    }
}

void ADinosaurBase::UpdateStats(float DeltaTime)
{
    // Health regeneration
    HealthRegenTimer += DeltaTime;
    if (HealthRegenTimer >= 5.0f && CurrentHealth < MaxHealth)
    {
        RestoreHealth(1.0f);
        HealthRegenTimer = 0.0f;
    }
    
    // Stamina regeneration
    StaminaRegenTimer += DeltaTime;
    if (StaminaRegenTimer >= 1.0f && CurrentStamina < MaxStamina)
    {
        ModifyStamina(10.0f);
        StaminaRegenTimer = 0.0f;
    }
    
    // Hunger decay
    HungerDecayTimer += DeltaTime;
    if (HungerDecayTimer >= 10.0f)
    {
        ModifyHunger(-1.0f);
        HungerDecayTimer = 0.0f;
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EEng_DinosaurBehaviorState OldState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        StateTimer = 0.0f;
        
        HandleStateTransition(OldState, NewState);
        OnStateChanged.Broadcast(NewState);
        
        UE_LOG(LogDinosaurBase, Log, TEXT("%s state changed from %d to %d"), 
               *GetName(), (int32)OldState, (int32)NewState);
    }
}

void ADinosaurBase::TakeDamage(float Damage, AActor* DamageSource)
{
    if (Damage <= 0.0f) return;
    
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    OnDamaged.Broadcast(Damage, DamageSource);
    
    UE_LOG(LogDinosaurBase, Log, TEXT("%s took %.1f damage from %s. Health: %.1f/%.1f"), 
           *GetName(), Damage, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"), 
           CurrentHealth, MaxHealth);
    
    // React to damage
    if (ShouldFlee())
    {
        StartFleeing(DamageSource);
    }
    else if (DamageSource && DietType == EEng_DinosaurDiet::Carnivore)
    {
        StartHunting(DamageSource);
    }
}

void ADinosaurBase::RestoreHealth(float Amount)
{
    if (Amount > 0.0f)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
    }
}

void ADinosaurBase::ModifyHunger(float Amount)
{
    CurrentHunger = FMath::Clamp(CurrentHunger + Amount, 0.0f, MaxHunger);
    
    // Check if we need to hunt for food
    if (CurrentHunger < 30.0f && ShouldHunt())
    {
        AActor* Prey = FindNearestPrey();
        if (Prey)
        {
            StartHunting(Prey);
        }
    }
}

void ADinosaurBase::ModifyStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
}

float ADinosaurBase::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetHungerPercentage() const
{
    return MaxHunger > 0.0f ? (CurrentHunger / MaxHunger) : 0.0f;
}

float ADinosaurBase::GetStaminaPercentage() const
{
    return MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
}

bool ADinosaurBase::CanDetectTarget(AActor* Target) const
{
    if (!Target) return false;
    
    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Distance <= DetectionRange;
}

bool ADinosaurBase::IsInAttackRange(AActor* Target) const
{
    if (!Target) return false;
    
    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ADinosaurBase::StartHunting(AActor* Target)
{
    if (Target && CanDetectTarget(Target))
    {
        CurrentTarget = Target;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }
}

void ADinosaurBase::StartFleeing(AActor* Threat)
{
    if (Threat)
    {
        LastKnownThreatLocation = Threat->GetActorLocation();
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
    }
}

void ADinosaurBase::StartPatrolling()
{
    PatrolDestination = GetRandomPatrolLocation();
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
}

void ADinosaurBase::StartResting()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Resting);
}

void ADinosaurBase::SetTerritory(FVector Center, float Radius)
{
    TerritoryCenter = Center;
    TerritoryRadius = Radius;
}

bool ADinosaurBase::IsInTerritory(FVector Location) const
{
    float Distance = FVector::Dist(TerritoryCenter, Location);
    return Distance <= TerritoryRadius;
}

FVector ADinosaurBase::GetRandomLocationInTerritory() const
{
    FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
    float RandomDistance = FMath::RandRange(0.0f, TerritoryRadius * 0.8f);
    return TerritoryCenter + (RandomDirection * RandomDistance);
}

void ADinosaurBase::JoinPack(ADinosaurBase* Leader)
{
    if (Leader && Leader != this)
    {
        LeavePack(); // Leave current pack first
        PackLeader = Leader;
        Leader->AddPackMember(this);
    }
}

void ADinosaurBase::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->RemovePackMember(this);
        PackLeader = nullptr;
    }
    
    // If this was a leader, disband the pack
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackLeader = nullptr;
        }
    }
    PackMembers.Empty();
}

void ADinosaurBase::AddPackMember(ADinosaurBase* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
    }
}

void ADinosaurBase::RemovePackMember(ADinosaurBase* Member)
{
    PackMembers.Remove(Member);
}

bool ADinosaurBase::IsPackLeader() const
{
    return PackMembers.Num() > 0;
}

void ADinosaurBase::HandleStateTransition(EEng_DinosaurBehaviorState OldState, EEng_DinosaurBehaviorState NewState)
{
    // Handle specific state transitions
    switch (NewState)
    {
        case EEng_DinosaurBehaviorState::Hunting:
            ModifyStamina(-10.0f); // Hunting costs stamina
            break;
        case EEng_DinosaurBehaviorState::Fleeing:
            ModifyStamina(-20.0f); // Fleeing costs more stamina
            break;
        case EEng_DinosaurBehaviorState::Resting:
            // Stop movement when resting
            MovementComponent->StopMovementImmediately();
            break;
    }
}

void ADinosaurBase::ExecuteIdleBehavior(float DeltaTime)
{
    // Look for threats or prey
    AActor* Threat = FindNearestThreat();
    if (Threat && ShouldFlee())
    {
        StartFleeing(Threat);
        return;
    }
    
    AActor* Prey = FindNearestPrey();
    if (Prey && ShouldHunt())
    {
        StartHunting(Prey);
        return;
    }
    
    // Start patrolling after being idle for a while
    if (StateTimer > 5.0f)
    {
        StartPatrolling();
    }
}

void ADinosaurBase::ExecutePatrolBehavior(float DeltaTime)
{
    // Move towards patrol destination
    FVector CurrentLocation = GetActorLocation();
    FVector Direction = (PatrolDestination - CurrentLocation).GetSafeNormal();
    
    MovementComponent->AddInputVector(Direction);
    
    // Check if we reached the destination
    if (FVector::Dist(CurrentLocation, PatrolDestination) < 100.0f)
    {
        PatrolDestination = GetRandomPatrolLocation();
    }
    
    // Check for threats while patrolling
    AActor* Threat = FindNearestThreat();
    if (Threat && ShouldFlee())
    {
        StartFleeing(Threat);
    }
}

void ADinosaurBase::ExecuteHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
        return;
    }
    
    // Move towards target
    FVector CurrentLocation = GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    MovementComponent->AddInputVector(Direction);
    
    // Check if in attack range
    if (IsInAttackRange(CurrentTarget))
    {
        // Attack logic would go here
        SetBehaviorState(EEng_DinosaurBehaviorState::Feeding);
    }
    
    // Give up if target is too far or we're too tired
    if (!CanDetectTarget(CurrentTarget) || CurrentStamina < 20.0f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
    }
}

void ADinosaurBase::ExecuteFleeingBehavior(float DeltaTime)
{
    // Move away from threat
    FVector CurrentLocation = GetActorLocation();
    FVector FleeDirection = (CurrentLocation - LastKnownThreatLocation).GetSafeNormal();
    
    MovementComponent->AddInputVector(FleeDirection);
    
    // Stop fleeing after some time or when far enough
    if (StateTimer > 10.0f || FVector::Dist(CurrentLocation, LastKnownThreatLocation) > DetectionRange * 2.0f)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
    }
}

void ADinosaurBase::ExecuteRestingBehavior(float DeltaTime)
{
    // Regenerate stamina faster while resting
    ModifyStamina(5.0f * DeltaTime);
    
    // Stop resting when stamina is full or after some time
    if (CurrentStamina >= MaxStamina || StateTimer > 15.0f)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
    }
}

void ADinosaurBase::ExecuteFeedingBehavior(float DeltaTime)
{
    // Restore hunger while feeding
    ModifyHunger(20.0f * DeltaTime);
    
    // Stop feeding when full or after some time
    if (CurrentHunger >= MaxHunger || StateTimer > 5.0f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
    }
}

AActor* ADinosaurBase::FindNearestThreat() const
{
    // Simple threat detection - find players or larger predators
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = DetectionRange;
    
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Pawn->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestThreat = Pawn;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestThreat;
}

AActor* ADinosaurBase::FindNearestPrey() const
{
    // Only carnivores hunt
    if (DietType != EEng_DinosaurDiet::Carnivore) return nullptr;
    
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    AActor* NearestPrey = nullptr;
    float NearestDistance = DetectionRange;
    
    for (TActorIterator<ADinosaurBase> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADinosaurBase* OtherDinosaur = *ActorItr;
        if (OtherDinosaur && OtherDinosaur != this && 
            OtherDinosaur->SizeCategory < SizeCategory)
        {
            float Distance = FVector::Dist(GetActorLocation(), OtherDinosaur->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestPrey = OtherDinosaur;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestPrey;
}

FVector ADinosaurBase::GetRandomPatrolLocation() const
{
    return GetRandomLocationInTerritory();
}

bool ADinosaurBase::ShouldFlee() const
{
    return GetHealthPercentage() < (FleeThreshold / 100.0f) || CurrentStamina < 30.0f;
}

bool ADinosaurBase::ShouldHunt() const
{
    return DietType == EEng_DinosaurDiet::Carnivore && 
           GetHungerPercentage() < 0.5f && 
           CurrentStamina > 50.0f;
}

bool ADinosaurBase::ShouldRest() const
{
    return CurrentStamina < 30.0f;
}