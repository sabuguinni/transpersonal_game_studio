#include "Combat_CombatManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Combat_TacticalAI.h"
#include "Combat_DinosaurAI.h"

ACombat_CombatManager::ACombat_CombatManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create detection sphere component
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize combat settings
    CombatUpdateInterval = 0.5f;
    MaxSimultaneousCombats = 5;
    CurrentCombatCount = 0;
    LastCombatUpdate = 0.0f;

    // Initialize default combat zones
    FCombat_CombatZone TRexZone;
    TRexZone.Center = FVector(3000, 0, 100);
    TRexZone.Radius = 800.0f;
    TRexZone.ThreatLevel = ECombat_ThreatLevel::Extreme;
    CombatZones.Add(TRexZone);

    FCombat_CombatZone RaptorZone;
    RaptorZone.Center = FVector(1000, 1500, 100);
    RaptorZone.Radius = 600.0f;
    RaptorZone.ThreatLevel = ECombat_ThreatLevel::High;
    CombatZones.Add(RaptorZone);

    FCombat_CombatZone HerbivoreZone;
    HerbivoreZone.Center = FVector(-1000, 0, 100);
    HerbivoreZone.Radius = 1000.0f;
    HerbivoreZone.ThreatLevel = ECombat_ThreatLevel::Medium;
    CombatZones.Add(HerbivoreZone);
}

void ACombat_CombatManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Manager initialized with %d combat zones"), CombatZones.Num());
    UpdateCombatZones();
}

void ACombat_CombatManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCombatUpdate >= CombatUpdateInterval)
    {
        UpdateCombatZones();
        ProcessCombatQueue();
        LastCombatUpdate = CurrentTime;
    }
}

void ACombat_CombatManager::RegisterCombatant(AActor* Combatant)
{
    if (!Combatant || ActiveCombatants.Contains(Combatant))
    {
        return;
    }

    ActiveCombatants.Add(Combatant);
    UE_LOG(LogTemp, Log, TEXT("Registered combatant: %s"), *Combatant->GetName());
}

void ACombat_CombatManager::UnregisterCombatant(AActor* Combatant)
{
    if (!Combatant)
    {
        return;
    }

    ActiveCombatants.Remove(Combatant);
    PendingCombatants.Remove(Combatant);
    
    // Remove from combat zones
    for (FCombat_CombatZone& Zone : CombatZones)
    {
        Zone.ActiveCombatants.Remove(Combatant);
    }

    UE_LOG(LogTemp, Log, TEXT("Unregistered combatant: %s"), *Combatant->GetName());
}

void ACombat_CombatManager::StartCombat(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target || !CanStartNewCombat())
    {
        return;
    }

    // Add to pending queue if at capacity
    if (CurrentCombatCount >= MaxSimultaneousCombats)
    {
        if (!PendingCombatants.Contains(Attacker))
        {
            PendingCombatants.Add(Attacker);
        }
        return;
    }

    CurrentCombatCount++;
    UE_LOG(LogTemp, Warning, TEXT("Combat started: %s vs %s"), *Attacker->GetName(), *Target->GetName());

    // Notify tactical AI components
    UCombat_TacticalAI* AttackerAI = Attacker->FindComponentByClass<UCombat_TacticalAI>();
    if (AttackerAI)
    {
        AttackerAI->SetCombatTarget(Target);
    }

    UCombat_DinosaurAI* DinoAI = Attacker->FindComponentByClass<UCombat_DinosaurAI>();
    if (DinoAI)
    {
        DinoAI->EnterCombatMode(Target);
    }
}

void ACombat_CombatManager::EndCombat(AActor* Combatant)
{
    if (!Combatant)
    {
        return;
    }

    CurrentCombatCount = FMath::Max(0, CurrentCombatCount - 1);
    UE_LOG(LogTemp, Log, TEXT("Combat ended for: %s"), *Combatant->GetName());

    // Process pending combats
    ProcessCombatQueue();
}

bool ACombat_CombatManager::IsInCombatZone(const FVector& Location) const
{
    for (const FCombat_CombatZone& Zone : CombatZones)
    {
        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius)
        {
            return true;
        }
    }
    return false;
}

ECombat_ThreatLevel ACombat_CombatManager::GetThreatLevelAtLocation(const FVector& Location) const
{
    ECombat_ThreatLevel HighestThreat = ECombat_ThreatLevel::Low;

    for (const FCombat_CombatZone& Zone : CombatZones)
    {
        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius)
        {
            if (Zone.ThreatLevel > HighestThreat)
            {
                HighestThreat = Zone.ThreatLevel;
            }
        }
    }

    return HighestThreat;
}

TArray<AActor*> ACombat_CombatManager::GetNearbyCombatants(const FVector& Location, float Radius) const
{
    TArray<AActor*> NearbyCombatants;

    for (AActor* Combatant : ActiveCombatants)
    {
        if (Combatant && IsValid(Combatant))
        {
            float Distance = FVector::Dist(Location, Combatant->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyCombatants.Add(Combatant);
            }
        }
    }

    return NearbyCombatants;
}

void ACombat_CombatManager::UpdateCombatZones()
{
    // Clear previous zone assignments
    for (FCombat_CombatZone& Zone : CombatZones)
    {
        Zone.ActiveCombatants.Empty();
    }

    // Reassign combatants to zones
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!Combatant || !IsValid(Combatant))
        {
            continue;
        }

        FVector CombatantLocation = Combatant->GetActorLocation();
        
        for (FCombat_CombatZone& Zone : CombatZones)
        {
            float Distance = FVector::Dist(CombatantLocation, Zone.Center);
            if (Distance <= Zone.Radius)
            {
                Zone.ActiveCombatants.AddUnique(Combatant);
            }
        }
    }
}

void ACombat_CombatManager::ProcessCombatQueue()
{
    if (PendingCombatants.Num() == 0 || !CanStartNewCombat())
    {
        return;
    }

    // Process one pending combat
    AActor* PendingCombatant = PendingCombatants[0];
    PendingCombatants.RemoveAt(0);

    if (PendingCombatant && IsValid(PendingCombatant))
    {
        // Find nearest target
        TArray<AActor*> NearbyTargets = GetNearbyCombatants(
            PendingCombatant->GetActorLocation(), 
            1000.0f
        );

        if (NearbyTargets.Num() > 0)
        {
            AActor* Target = NearbyTargets[0];
            StartCombat(PendingCombatant, Target);
        }
    }
}

bool ACombat_CombatManager::CanStartNewCombat() const
{
    return CurrentCombatCount < MaxSimultaneousCombats;
}

bool ACombat_CombatManager::IsActorInCombat(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    UCombat_TacticalAI* TacticalAI = Actor->FindComponentByClass<UCombat_TacticalAI>();
    if (TacticalAI)
    {
        return TacticalAI->IsInCombat();
    }

    UCombat_DinosaurAI* DinoAI = Actor->FindComponentByClass<UCombat_DinosaurAI>();
    if (DinoAI)
    {
        return DinoAI->IsInCombatMode();
    }

    return false;
}