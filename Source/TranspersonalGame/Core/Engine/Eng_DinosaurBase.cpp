#include "Eng_DinosaurBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogDinosaurBase, Log, All);

AEng_DinosaurBase::AEng_DinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default values
    DinosaurSpecies = EEng_DinosaurSpecies::TRex;
    DinosaurSize = EEng_DinosaurSize::Large;
    DinosaurDiet = EEng_DinosaurDiet::Carnivore;
    DinosaurBehavior = EEng_DinosaurBehavior::Aggressive;
    
    // Initialize stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Hunger = 50.0f;
    MaxHunger = 100.0f;
    
    // Movement settings
    MovementSpeed = 300.0f;
    TerritoryRadius = 2000.0f;
    DetectionRange = 1500.0f;
    AttackRange = 200.0f;
    
    // Behavioral settings
    AggressionLevel = 0.7f;
    FearLevel = 0.3f;
    SocialTendency = 0.5f;
    
    // Set up collision
    GetCapsuleComponent()->SetCapsuleSize(100.0f, 200.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    
    // Configure movement
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = MovementSpeed;
        MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MovementComp->bUseControllerDesiredRotation = true;
    }
    
    // Set up mesh
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -200.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
    
    // Initialize territory center to spawn location
    TerritoryCenter = GetActorLocation();
    
    UE_LOG(LogDinosaurBase, Log, TEXT("DinosaurBase created: %s"), *GetName());
}

void AEng_DinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Set territory center to current location
    TerritoryCenter = GetActorLocation();
    
    // Initialize species-specific settings
    InitializeSpeciesSettings();
    
    // Start behavioral tick
    GetWorldTimerManager().SetTimer(BehaviorTickTimer, this, &AEng_DinosaurBase::BehaviorTick, 1.0f, true);
    
    UE_LOG(LogDinosaurBase, Log, TEXT("DinosaurBase BeginPlay: %s at %s"), 
           *GetSpeciesName(), 
           *GetActorLocation().ToString());
}

void AEng_DinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update survival stats
    UpdateSurvivalStats(DeltaTime);
    
    // Check territory bounds
    CheckTerritoryBounds();
}

void AEng_DinosaurBase::InitializeSpeciesSettings()
{
    switch (DinosaurSpecies)
    {
        case EEng_DinosaurSpecies::TRex:
            MaxHealth = 500.0f;
            MovementSpeed = 400.0f;
            AttackRange = 300.0f;
            AggressionLevel = 0.9f;
            DinosaurSize = EEng_DinosaurSize::Large;
            DinosaurDiet = EEng_DinosaurDiet::Carnivore;
            DinosaurBehavior = EEng_DinosaurBehavior::Aggressive;
            break;
            
        case EEng_DinosaurSpecies::Velociraptor:
            MaxHealth = 150.0f;
            MovementSpeed = 600.0f;
            AttackRange = 150.0f;
            AggressionLevel = 0.8f;
            SocialTendency = 0.9f; // Pack hunter
            DinosaurSize = EEng_DinosaurSize::Medium;
            DinosaurDiet = EEng_DinosaurDiet::Carnivore;
            DinosaurBehavior = EEng_DinosaurBehavior::Aggressive;
            break;
            
        case EEng_DinosaurSpecies::Triceratops:
            MaxHealth = 400.0f;
            MovementSpeed = 250.0f;
            AttackRange = 200.0f;
            AggressionLevel = 0.4f;
            FearLevel = 0.3f;
            DinosaurSize = EEng_DinosaurSize::Large;
            DinosaurDiet = EEng_DinosaurDiet::Herbivore;
            DinosaurBehavior = EEng_DinosaurBehavior::Defensive;
            break;
            
        case EEng_DinosaurSpecies::Brachiosaurus:
            MaxHealth = 800.0f;
            MovementSpeed = 150.0f;
            AttackRange = 100.0f;
            AggressionLevel = 0.1f;
            FearLevel = 0.2f;
            DinosaurSize = EEng_DinosaurSize::Massive;
            DinosaurDiet = EEng_DinosaurDiet::Herbivore;
            DinosaurBehavior = EEng_DinosaurBehavior::Passive;
            break;
            
        case EEng_DinosaurSpecies::Ankylosaurus:
            MaxHealth = 350.0f;
            MovementSpeed = 200.0f;
            AttackRange = 180.0f;
            AggressionLevel = 0.3f;
            FearLevel = 0.1f; // Heavily armored
            DinosaurSize = EEng_DinosaurSize::Large;
            DinosaurDiet = EEng_DinosaurDiet::Herbivore;
            DinosaurBehavior = EEng_DinosaurBehavior::Defensive;
            break;
    }
    
    // Apply settings
    Health = MaxHealth;
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = MovementSpeed;
    }
    
    UE_LOG(LogDinosaurBase, Log, TEXT("Species settings applied: %s - Health: %.0f, Speed: %.0f"), 
           *GetSpeciesName(), MaxHealth, MovementSpeed);
}

void AEng_DinosaurBase::UpdateSurvivalStats(float DeltaTime)
{
    // Hunger increases over time
    Hunger = FMath::Clamp(Hunger + (DeltaTime * 2.0f), 0.0f, MaxHunger);
    
    // Stamina regenerates when not moving
    FVector Velocity = GetVelocity();
    if (Velocity.Size() < 50.0f)
    {
        Stamina = FMath::Clamp(Stamina + (DeltaTime * 20.0f), 0.0f, MaxStamina);
    }
    else
    {
        Stamina = FMath::Clamp(Stamina - (DeltaTime * 10.0f), 0.0f, MaxStamina);
    }
    
    // Health effects from hunger
    if (Hunger > 90.0f)
    {
        Health = FMath::Clamp(Health - (DeltaTime * 5.0f), 0.0f, MaxHealth);
    }
    else if (Hunger < 30.0f && Health < MaxHealth)
    {
        Health = FMath::Clamp(Health + (DeltaTime * 2.0f), 0.0f, MaxHealth);
    }
}

void AEng_DinosaurBase::BehaviorTick()
{
    // Basic AI behavior - can be overridden by AI controller
    if (!GetController())
    {
        return;
    }
    
    // Simple territory patrol behavior
    FVector CurrentLocation = GetActorLocation();
    float DistanceFromTerritory = FVector::Dist2D(CurrentLocation, TerritoryCenter);
    
    if (DistanceFromTerritory > TerritoryRadius)
    {
        // Return to territory
        FVector DirectionToTerritory = (TerritoryCenter - CurrentLocation).GetSafeNormal();
        AddMovementInput(DirectionToTerritory, 1.0f);
    }
    else
    {
        // Random movement within territory
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        AddMovementInput(RandomDirection, 0.3f);
    }
}

void AEng_DinosaurBase::CheckTerritoryBounds()
{
    FVector CurrentLocation = GetActorLocation();
    float DistanceFromTerritory = FVector::Dist2D(CurrentLocation, TerritoryCenter);
    
    if (DistanceFromTerritory > TerritoryRadius * 1.2f)
    {
        // Force return to territory
        SetActorLocation(TerritoryCenter + FVector(FMath::RandRange(-500.0f, 500.0f), 
                                                  FMath::RandRange(-500.0f, 500.0f), 
                                                  CurrentLocation.Z));
        
        UE_LOG(LogDinosaurBase, Warning, TEXT("Dinosaur %s forced back to territory"), *GetName());
    }
}

FString AEng_DinosaurBase::GetSpeciesName() const
{
    return UEnum::GetValueAsString(DinosaurSpecies);
}

FString AEng_DinosaurBase::GetSizeName() const
{
    return UEnum::GetValueAsString(DinosaurSize);
}

FString AEng_DinosaurBase::GetDietName() const
{
    return UEnum::GetValueAsString(DinosaurDiet);
}

FString AEng_DinosaurBase::GetBehaviorName() const
{
    return UEnum::GetValueAsString(DinosaurBehavior);
}

bool AEng_DinosaurBase::IsAlive() const
{
    return Health > 0.0f;
}

bool AEng_DinosaurBase::IsHungry() const
{
    return Hunger > 70.0f;
}

bool AEng_DinosaurBase::IsTired() const
{
    return Stamina < 30.0f;
}

void AEng_DinosaurBase::TakeDamage(float DamageAmount)
{
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    
    if (Health <= 0.0f)
    {
        OnDeath();
    }
    
    UE_LOG(LogDinosaurBase, Log, TEXT("Dinosaur %s took %.0f damage, Health: %.0f"), 
           *GetName(), DamageAmount, Health);
}

void AEng_DinosaurBase::OnDeath()
{
    UE_LOG(LogDinosaurBase, Log, TEXT("Dinosaur %s died"), *GetName());
    
    // Disable movement
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->DisableMovement();
    }
    
    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Stop behavior timer
    GetWorldTimerManager().ClearTimer(BehaviorTickTimer);
    
    // Mark for destruction after delay
    GetWorldTimerManager().SetTimer(DeathTimer, this, &AEng_DinosaurBase::DestroyDinosaur, 10.0f, false);
}

void AEng_DinosaurBase::DestroyDinosaur()
{
    Destroy();
}