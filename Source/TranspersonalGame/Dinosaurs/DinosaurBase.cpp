#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule size — will be overridden per species
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f; // Dinosaurs don't jump by default
    GetCharacterMovement()->AirControl = 0.0f;

    // No controller rotation — movement component handles it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Initialize territory at spawn location
    bHasTerritoryOverride = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location if not overridden
    if (!bHasTerritoryOverride)
    {
        TerritoryCenter = GetActorLocation();
    }

    // Apply species-specific data
    InitializeSpeciesData();

    // Apply movement speed from stats
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned. Species: %d, Health: %.0f, Territory: %s"),
        *GetName(),
        (int32)Species,
        Stats.CurrentHealth,
        *TerritoryCenter.ToString());
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Periodic threat scan
    TimeSinceLastScan += DeltaTime;
    if (TimeSinceLastScan >= ScanInterval)
    {
        TimeSinceLastScan = 0.0f;
        ScanForThreats();
        UpdateAggressionLevel(DeltaTime);
    }
}

// ── Species Initialization ────────────────────────────────────────────────────

void ADinosaurBase::InitializeSpeciesData()
{
    switch (Species)
    {
    case EEng_DinoSpecies::TyrannosaurusRex:
        SpeciesData.CommonName = TEXT("Tyrannosaurus Rex");
        SpeciesData.bIsCarnivore = true;
        SpeciesData.bIsPackHunter = false;
        SpeciesData.bIsFlying = false;
        Stats.MaxHealth = 500.0f;
        Stats.CurrentHealth = 500.0f;
        Stats.MoveSpeed = 350.0f;
        Stats.SprintSpeed = 700.0f;
        Stats.AttackDamage = 80.0f;
        Stats.AttackRange = 250.0f;
        Stats.DetectionRadius = 2500.0f;
        Stats.TerritoryRadius = 5000.0f;
        Stats.Mass = 8000.0f;
        MaxPackSize = 1;
        break;

    case EEng_DinoSpecies::Velociraptor:
        SpeciesData.CommonName = TEXT("Velociraptor");
        SpeciesData.bIsCarnivore = true;
        SpeciesData.bIsPackHunter = true;
        SpeciesData.bIsFlying = false;
        SpeciesData.PackDetectionBonus = 1.5f;
        Stats.MaxHealth = 80.0f;
        Stats.CurrentHealth = 80.0f;
        Stats.MoveSpeed = 600.0f;
        Stats.SprintSpeed = 900.0f;
        Stats.AttackDamage = 30.0f;
        Stats.AttackRange = 120.0f;
        Stats.DetectionRadius = 1800.0f;
        Stats.TerritoryRadius = 2500.0f;
        Stats.Mass = 80.0f;
        MaxPackSize = 6;
        break;

    case EEng_DinoSpecies::Brachiosaurus:
        SpeciesData.CommonName = TEXT("Brachiosaurus");
        SpeciesData.bIsCarnivore = false;
        SpeciesData.bIsPackHunter = false;
        SpeciesData.bIsFlying = false;
        Stats.MaxHealth = 1200.0f;
        Stats.CurrentHealth = 1200.0f;
        Stats.MoveSpeed = 200.0f;
        Stats.SprintSpeed = 350.0f;
        Stats.AttackDamage = 60.0f; // Stomp damage
        Stats.AttackRange = 300.0f;
        Stats.DetectionRadius = 1000.0f;
        Stats.TerritoryRadius = 8000.0f;
        Stats.AggressionThreshold = 0.9f; // Very passive
        Stats.Mass = 50000.0f;
        MaxPackSize = 4;
        break;

    case EEng_DinoSpecies::Triceratops:
        SpeciesData.CommonName = TEXT("Triceratops");
        SpeciesData.bIsCarnivore = false;
        SpeciesData.bIsPackHunter = false;
        SpeciesData.bIsFlying = false;
        Stats.MaxHealth = 400.0f;
        Stats.CurrentHealth = 400.0f;
        Stats.MoveSpeed = 300.0f;
        Stats.SprintSpeed = 550.0f;
        Stats.AttackDamage = 50.0f;
        Stats.AttackRange = 200.0f;
        Stats.DetectionRadius = 1200.0f;
        Stats.TerritoryRadius = 3000.0f;
        Stats.AggressionThreshold = 0.7f;
        Stats.Mass = 6000.0f;
        MaxPackSize = 3;
        break;

    case EEng_DinoSpecies::Pteranodon:
        SpeciesData.CommonName = TEXT("Pteranodon");
        SpeciesData.bIsCarnivore = true;
        SpeciesData.bIsPackHunter = false;
        SpeciesData.bIsFlying = true;
        Stats.MaxHealth = 60.0f;
        Stats.CurrentHealth = 60.0f;
        Stats.MoveSpeed = 800.0f;
        Stats.SprintSpeed = 1200.0f;
        Stats.AttackDamage = 20.0f;
        Stats.AttackRange = 100.0f;
        Stats.DetectionRadius = 3000.0f;
        Stats.TerritoryRadius = 10000.0f;
        Stats.Mass = 25.0f;
        MaxPackSize = 2;
        break;

    case EEng_DinoSpecies::Spinosaurus:
        SpeciesData.CommonName = TEXT("Spinosaurus");
        SpeciesData.bIsCarnivore = true;
        SpeciesData.bIsPackHunter = false;
        SpeciesData.bIsFlying = false;
        Stats.MaxHealth = 700.0f;
        Stats.CurrentHealth = 700.0f;
        Stats.MoveSpeed = 400.0f;
        Stats.SprintSpeed = 750.0f;
        Stats.AttackDamage = 70.0f;
        Stats.AttackRange = 280.0f;
        Stats.DetectionRadius = 2000.0f;
        Stats.TerritoryRadius = 6000.0f;
        Stats.Mass = 9000.0f;
        MaxPackSize = 1;
        break;

    default:
        // Generic herbivore defaults
        SpeciesData.CommonName = TEXT("Unknown Dinosaur");
        SpeciesData.bIsCarnivore = false;
        Stats.MaxHealth = 100.0f;
        Stats.CurrentHealth = 100.0f;
        Stats.MoveSpeed = 300.0f;
        Stats.SprintSpeed = 500.0f;
        Stats.AttackDamage = 15.0f;
        Stats.AttackRange = 100.0f;
        Stats.DetectionRadius = 1000.0f;
        Stats.TerritoryRadius = 2000.0f;
        Stats.Mass = 200.0f;
        MaxPackSize = 1;
        break;
    }

    // Apply movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
    }
}

// ── Combat ────────────────────────────────────────────────────────────────────

void ADinosaurBase::TakeDinoHit(float Damage, AActor* DamageSource)
{
    if (bIsDead) return;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - Damage);

    // Become threatened when hit
    if (AggressionState == EEng_DinoAggressionState::Passive ||
        AggressionState == EEng_DinoAggressionState::Curious)
    {
        SetAggressionState(EEng_DinoAggressionState::Threatened);
        CurrentTarget = DamageSource;
    }

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s hit for %.0f damage. Health: %.0f/%.0f"),
        *GetName(), Damage, Stats.CurrentHealth, Stats.MaxHealth);

    if (Stats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && Stats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::OnDeath()
{
    if (bIsDead) return;
    bIsDead = true;

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died."), *GetName());

    // Notify pack members
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && Member->IsAlive())
        {
            Member->RemovePackMember(this);
        }
    }
    PackMembers.Empty();

    // Disable movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Ragdoll (if mesh exists)
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

// ── Behavior ──────────────────────────────────────────────────────────────────

void ADinosaurBase::SetAggressionState(EEng_DinoAggressionState NewState)
{
    if (AggressionState == NewState) return;
    AggressionState = NewState;

    // Adjust movement speed based on state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
        case EEng_DinoAggressionState::Hunting:
        case EEng_DinoAggressionState::Territorial:
            GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed;
            break;
        case EEng_DinoAggressionState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed * 1.1f;
            break;
        default:
            GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
            break;
        }
    }
}

void ADinosaurBase::SetBehaviorMode(EEng_DinoBehaviorMode NewMode)
{
    BehaviorMode = NewMode;
}

void ADinosaurBase::UpdateAggressionLevel(float DeltaTime)
{
    // Decay aggression over time when no threat is present
    if (CurrentTarget == nullptr)
    {
        AggressionLevel = FMath::Max(0.0f, AggressionLevel - DeltaTime * 0.1f);
        if (AggressionLevel < 0.1f && AggressionState != EEng_DinoAggressionState::Passive)
        {
            SetAggressionState(EEng_DinoAggressionState::Passive);
        }
    }
}

void ADinosaurBase::ScanForThreats()
{
    // Basic proximity scan — finds actors within detection radius
    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        Stats.DetectionRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    // Check if any overlapping actor is a potential threat
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == this) continue;

        // If carnivore, look for non-dino pawns (players)
        if (SpeciesData.bIsCarnivore)
        {
            ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
            if (!OtherDino) // Non-dino pawn = potential prey
            {
                float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                float ThreatFactor = 1.0f - (Distance / Stats.DetectionRadius);
                AggressionLevel = FMath::Min(1.0f, AggressionLevel + ThreatFactor * 0.3f);

                if (AggressionLevel >= Stats.AggressionThreshold)
                {
                    CurrentTarget = Actor;
                    SetAggressionState(EEng_DinoAggressionState::Hunting);
                }
            }
        }
    }
}

// ── Territory ─────────────────────────────────────────────────────────────────

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= Stats.TerritoryRadius;
}

// ── Pack ──────────────────────────────────────────────────────────────────────

void ADinosaurBase::AddPackMember(ADinosaurBase* Member)
{
    if (!Member || Member == this) return;
    if (PackMembers.Num() >= MaxPackSize) return;
    if (!PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s joined pack of %s. Pack size: %d"),
            *Member->GetName(), *GetName(), PackMembers.Num());
    }
}

void ADinosaurBase::RemovePackMember(ADinosaurBase* Member)
{
    PackMembers.Remove(Member);
}

// ── Debug ─────────────────────────────────────────────────────────────────────

void ADinosaurBase::DebugPrintStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DinosaurBase Debug: %s ==="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  Species: %s"), *SpeciesData.CommonName);
    UE_LOG(LogTemp, Warning, TEXT("  Health: %.0f / %.0f"), Stats.CurrentHealth, Stats.MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("  Aggression: %.2f (State: %d)"), AggressionLevel, (int32)AggressionState);
    UE_LOG(LogTemp, Warning, TEXT("  Behavior: %d"), (int32)BehaviorMode);
    UE_LOG(LogTemp, Warning, TEXT("  Territory Center: %s"), *TerritoryCenter.ToString());
    UE_LOG(LogTemp, Warning, TEXT("  Pack Size: %d / %d"), PackMembers.Num(), MaxPackSize);
    UE_LOG(LogTemp, Warning, TEXT("  Is Carnivore: %s"), SpeciesData.bIsCarnivore ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Is Dead: %s"), bIsDead ? TEXT("Yes") : TEXT("No"));

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), Stats.DetectionRadius, 16,
        FColor::Yellow, false, 5.0f);
    DrawDebugSphere(GetWorld(), TerritoryCenter, Stats.TerritoryRadius, 24,
        FColor::Red, false, 5.0f);
#endif
}
