#include "Combat_CombatManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCombat_CombatManager::UCombat_CombatManager()
{
    CombatTimeout = 30.0f;
    ProximityCheckInterval = 2.0f;
    CombatRange = 800.0f;
    ThreatDecayRate = 1.0f;
}

void UCombat_CombatManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Manager initialized"));
    
    // Start proximity checking timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ProximityCheckTimer, this, &UCombat_CombatManager::CheckProximityCombat, ProximityCheckInterval, true);
        
        // Start combat update timer
        FTimerDelegate CombatUpdateDelegate;
        CombatUpdateDelegate.BindUFunction(this, FName("UpdateCombatTimers"), 0.1f);
        World->GetTimerManager().SetTimer(CombatUpdateTimer, CombatUpdateDelegate, 0.1f, true);
    }
}

void UCombat_CombatManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProximityCheckTimer);
        World->GetTimerManager().ClearTimer(CombatUpdateTimer);
    }
    
    ActiveCombatants.Empty();
    ThreatTable.Empty();
    CombatPairs.Empty();
    CombatTimers.Empty();
    
    Super::Deinitialize();
}

bool UCombat_CombatManager::InitiateCombat(AActor* Attacker, AActor* Target, float Damage)
{
    if (!Attacker || !Target || Attacker == Target)
    {
        return false;
    }
    
    // Register both as combatants
    RegisterCombatant(Attacker);
    RegisterCombatant(Target);
    
    // Set up combat pair
    CombatPairs.Add(Attacker, Target);
    CombatPairs.Add(Target, Attacker);
    
    // Reset combat timers
    CombatTimers.Add(Attacker, CombatTimeout);
    CombatTimers.Add(Target, CombatTimeout);
    
    // Add initial threat
    AddThreat(Target, Attacker, Damage * 2.0f);
    
    // Apply damage
    ApplyDamage(Target, Damage, Attacker);
    
    // Broadcast combat started
    OnCombatStarted.Broadcast(Attacker, Target, Damage);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat initiated between %s and %s"), 
           *Attacker->GetName(), *Target->GetName());
    
    return true;
}

void UCombat_CombatManager::EndCombat(AActor* Combatant)
{
    if (!Combatant || !IsInCombat(Combatant))
    {
        return;
    }
    
    // Get combat partner
    AActor* Partner = CombatPairs.FindRef(Combatant);
    
    // Remove from active combatants
    ActiveCombatants.Remove(Combatant);
    if (Partner)
    {
        ActiveCombatants.Remove(Partner);
    }
    
    // Clean up combat pairs
    CombatPairs.Remove(Combatant);
    if (Partner)
    {
        CombatPairs.Remove(Partner);
    }
    
    // Clean up timers
    CombatTimers.Remove(Combatant);
    if (Partner)
    {
        CombatTimers.Remove(Partner);
    }
    
    // Clean up threat
    ThreatTable.Remove(Combatant);
    if (Partner)
    {
        ThreatTable.Remove(Partner);
    }
    
    // Broadcast combat ended
    OnCombatEnded.Broadcast(Combatant, Partner, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat ended for %s"), *Combatant->GetName());
}

bool UCombat_CombatManager::IsInCombat(AActor* Actor)
{
    return ActiveCombatants.Contains(Actor);
}

TArray<AActor*> UCombat_CombatManager::GetCombatants()
{
    return ActiveCombatants;
}

float UCombat_CombatManager::CalculateDamage(AActor* Attacker, AActor* Target, float BaseDamage)
{
    if (!Attacker || !Target)
    {
        return 0.0f;
    }
    
    float FinalDamage = BaseDamage;
    
    // Distance modifier - closer = more damage
    float Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());
    float DistanceModifier = FMath::Clamp(1.0f - (Distance / CombatRange), 0.1f, 1.0f);
    FinalDamage *= DistanceModifier;
    
    // Random variance
    float RandomModifier = FMath::RandRange(0.8f, 1.2f);
    FinalDamage *= RandomModifier;
    
    return FMath::Max(1.0f, FinalDamage);
}

void UCombat_CombatManager::ApplyDamage(AActor* Target, float Damage, AActor* DamageSource)
{
    if (!Target)
    {
        return;
    }
    
    // Try to find a health component or similar
    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        // For now, just log the damage
        UE_LOG(LogTemp, Warning, TEXT("%s takes %.1f damage from %s"), 
               *Target->GetName(), Damage, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"));
    }
    
    // Broadcast damage event
    OnDamageDealt.Broadcast(DamageSource, Target, Damage);
}

void UCombat_CombatManager::AddThreat(AActor* Target, AActor* ThreatSource, float ThreatAmount)
{
    if (!Target || !ThreatSource)
    {
        return;
    }
    
    if (!ThreatTable.Contains(Target))
    {
        ThreatTable.Add(Target, TMap<AActor*, float>());
    }
    
    TMap<AActor*, float>& TargetThreats = ThreatTable[Target];
    float CurrentThreat = TargetThreats.FindRef(ThreatSource);
    TargetThreats.Add(ThreatSource, CurrentThreat + ThreatAmount);
    
    UE_LOG(LogTemp, Log, TEXT("Threat added: %s -> %s (%.1f total)"), 
           *ThreatSource->GetName(), *Target->GetName(), CurrentThreat + ThreatAmount);
}

AActor* UCombat_CombatManager::GetHighestThreatTarget(AActor* Combatant)
{
    if (!Combatant || !ThreatTable.Contains(Combatant))
    {
        return nullptr;
    }
    
    TMap<AActor*, float>& Threats = ThreatTable[Combatant];
    AActor* HighestThreatActor = nullptr;
    float HighestThreat = 0.0f;
    
    for (auto& ThreatPair : Threats)
    {
        if (ThreatPair.Value > HighestThreat && IsValid(ThreatPair.Key))
        {
            HighestThreat = ThreatPair.Value;
            HighestThreatActor = ThreatPair.Key;
        }
    }
    
    return HighestThreatActor;
}

float UCombat_CombatManager::GetThreatLevel(AActor* Target, AActor* ThreatSource)
{
    if (!Target || !ThreatSource || !ThreatTable.Contains(Target))
    {
        return 0.0f;
    }
    
    return ThreatTable[Target].FindRef(ThreatSource);
}

void UCombat_CombatManager::RegisterCombatant(AActor* Combatant)
{
    if (Combatant && !ActiveCombatants.Contains(Combatant))
    {
        ActiveCombatants.Add(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Registered combatant: %s"), *Combatant->GetName());
    }
}

void UCombat_CombatManager::UnregisterCombatant(AActor* Combatant)
{
    if (Combatant)
    {
        EndCombat(Combatant);
    }
}

void UCombat_CombatManager::CheckProximityCombat()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    // Check for proximity between potential combatants
    for (int32 i = 0; i < AllActors.Num(); i++)
    {
        for (int32 j = i + 1; j < AllActors.Num(); j++)
        {
            AActor* Actor1 = AllActors[i];
            AActor* Actor2 = AllActors[j];
            
            if (!Actor1 || !Actor2 || Actor1 == Actor2)
            {
                continue;
            }
            
            // Check if they're already in combat
            if (IsInCombat(Actor1) || IsInCombat(Actor2))
            {
                continue;
            }
            
            // Check distance
            float Distance = FVector::Dist(Actor1->GetActorLocation(), Actor2->GetActorLocation());
            if (Distance <= CombatRange && AreEnemies(Actor1, Actor2))
            {
                // Initiate combat
                InitiateCombat(Actor1, Actor2, 5.0f);
                break; // Only one combat initiation per check
            }
        }
    }
}

TArray<AActor*> UCombat_CombatManager::GetNearbyEnemies(AActor* Actor, float Range)
{
    TArray<AActor*> NearbyEnemies;
    
    if (!Actor || !GetWorld())
    {
        return NearbyEnemies;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (AActor* OtherActor : AllActors)
    {
        if (!OtherActor || OtherActor == Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(ActorLocation, OtherActor->GetActorLocation());
        if (Distance <= Range && AreEnemies(Actor, OtherActor))
        {
            NearbyEnemies.Add(OtherActor);
        }
    }
    
    return NearbyEnemies;
}

void UCombat_CombatManager::UpdateCombatTimers(float DeltaTime)
{
    TArray<AActor*> ToRemove;
    
    for (auto& TimerPair : CombatTimers)
    {
        TimerPair.Value -= DeltaTime;
        if (TimerPair.Value <= 0.0f)
        {
            ToRemove.Add(TimerPair.Key);
        }
    }
    
    // End combat for timed out combatants
    for (AActor* Actor : ToRemove)
    {
        EndCombat(Actor);
    }
    
    // Decay threat
    DecayThreat(DeltaTime);
}

void UCombat_CombatManager::DecayThreat(float DeltaTime)
{
    for (auto& TargetPair : ThreatTable)
    {
        TMap<AActor*, float>& Threats = TargetPair.Value;
        TArray<AActor*> ToRemove;
        
        for (auto& ThreatPair : Threats)
        {
            ThreatPair.Value -= ThreatDecayRate * DeltaTime;
            if (ThreatPair.Value <= 0.0f)
            {
                ToRemove.Add(ThreatPair.Key);
            }
        }
        
        for (AActor* Actor : ToRemove)
        {
            Threats.Remove(Actor);
        }
    }
}

bool UCombat_CombatManager::AreEnemies(AActor* Actor1, AActor* Actor2)
{
    if (!Actor1 || !Actor2)
    {
        return false;
    }
    
    // Simple enemy detection - different classes are enemies
    // Player vs Dinosaur, Dinosaur vs Player, etc.
    bool Actor1IsPlayer = Actor1->GetName().Contains(TEXT("Character")) || Actor1->GetName().Contains(TEXT("Player"));
    bool Actor2IsPlayer = Actor2->GetName().Contains(TEXT("Character")) || Actor2->GetName().Contains(TEXT("Player"));
    
    bool Actor1IsDinosaur = Actor1->GetName().Contains(TEXT("Dinosaur")) || Actor1->GetName().Contains(TEXT("Rex")) || Actor1->GetName().Contains(TEXT("Raptor"));
    bool Actor2IsDinosaur = Actor2->GetName().Contains(TEXT("Dinosaur")) || Actor2->GetName().Contains(TEXT("Rex")) || Actor2->GetName().Contains(TEXT("Raptor"));
    
    // Player vs Dinosaur = enemies
    if ((Actor1IsPlayer && Actor2IsDinosaur) || (Actor1IsDinosaur && Actor2IsPlayer))
    {
        return true;
    }
    
    // For now, different dinosaur species can be enemies too
    return false;
}