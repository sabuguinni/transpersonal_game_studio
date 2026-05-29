#include "Combat_CombatStateManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UCombat_CombatStateManager::UCombat_CombatStateManager()
{
    CombatTimeoutDuration = 30.0f;
    ThreatDecayRate = 1.0f;
    MaxThreatDistance = 3000.0f;
    MaxCombatEventsHistory = 100;
}

void UCombat_CombatStateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat State Manager initialized"));
    
    // Start combat update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CombatUpdateTimer, this, 
            &UCombat_CombatStateManager::UpdateCombatStates, 1.0f, true);
    }
}

void UCombat_CombatStateManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CombatUpdateTimer);
    }
    
    CombatActors.Empty();
    ThreatMap.Empty();
    RecentCombatEvents.Empty();
    CombatStartTimes.Empty();
    
    Super::Deinitialize();
}

void UCombat_CombatStateManager::RegisterCombatActor(AActor* Actor, float CombatRadius)
{
    if (!Actor)
    {
        return;
    }
    
    CombatActors.Add(Actor, CombatRadius);
    UE_LOG(LogTemp, Log, TEXT("Registered combat actor: %s"), *Actor->GetName());
}

void UCombat_CombatStateManager::UnregisterCombatActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    CombatActors.Remove(Actor);
    ThreatMap.Remove(Actor);
    CombatStartTimes.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered combat actor: %s"), *Actor->GetName());
}

bool UCombat_CombatStateManager::IsActorInCombat(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    return CombatStartTimes.Contains(Actor);
}

void UCombat_CombatStateManager::StartCombat(AActor* Instigator, AActor* Target)
{
    if (!Instigator || !Target)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (!CombatStartTimes.Contains(Instigator))
    {
        CombatStartTimes.Add(Instigator, CurrentTime);
        UE_LOG(LogTemp, Warning, TEXT("Combat started: %s vs %s"), 
            *Instigator->GetName(), *Target->GetName());
    }
    
    if (!CombatStartTimes.Contains(Target))
    {
        CombatStartTimes.Add(Target, CurrentTime);
    }
    
    // Add mutual threat
    AddThreat(Instigator, Target, 50.0f);
    AddThreat(Target, Instigator, 50.0f);
}

void UCombat_CombatStateManager::EndCombat(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (CombatStartTimes.Contains(Actor))
    {
        float CombatDuration = 0.0f;
        if (UWorld* World = GetWorld())
        {
            CombatDuration = World->GetTimeSeconds() - CombatStartTimes[Actor];
        }
        
        CombatStartTimes.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("Combat ended for %s (Duration: %.2fs)"), 
            *Actor->GetName(), CombatDuration);
    }
}

void UCombat_CombatStateManager::ProcessCombatEvent(const FCombat_CombatEvent& CombatEvent)
{
    RecentCombatEvents.Add(CombatEvent);
    
    // Keep history manageable
    if (RecentCombatEvents.Num() > MaxCombatEventsHistory)
    {
        RecentCombatEvents.RemoveAt(0);
    }
    
    // Start combat if not already active
    if (CombatEvent.Instigator && CombatEvent.Target)
    {
        StartCombat(CombatEvent.Instigator, CombatEvent.Target);
        
        // Increase threat based on damage
        float ThreatIncrease = FMath::Clamp(CombatEvent.Damage * 0.5f, 5.0f, 100.0f);
        AddThreat(CombatEvent.Instigator, CombatEvent.Target, ThreatIncrease);
    }
}

void UCombat_CombatStateManager::AddThreat(AActor* ThreatActor, AActor* TargetActor, float ThreatAmount)
{
    if (!ThreatActor || !TargetActor)
    {
        return;
    }
    
    TArray<FCombat_ThreatData>& Threats = ThreatMap.FindOrAdd(TargetActor);
    
    // Find existing threat or create new one
    FCombat_ThreatData* ExistingThreat = nullptr;
    for (FCombat_ThreatData& Threat : Threats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            ExistingThreat = &Threat;
            break;
        }
    }
    
    if (ExistingThreat)
    {
        ExistingThreat->ThreatLevel += ThreatAmount;
        ExistingThreat->LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        ExistingThreat->LastKnownPosition = ThreatActor->GetActorLocation();
        ExistingThreat->bIsActive = true;
    }
    else
    {
        FCombat_ThreatData NewThreat;
        NewThreat.ThreatActor = ThreatActor;
        NewThreat.ThreatLevel = ThreatAmount;
        NewThreat.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        NewThreat.LastKnownPosition = ThreatActor->GetActorLocation();
        NewThreat.bIsActive = true;
        
        Threats.Add(NewThreat);
    }
}

void UCombat_CombatStateManager::RemoveThreat(AActor* ThreatActor, AActor* TargetActor)
{
    if (!ThreatActor || !TargetActor)
    {
        return;
    }
    
    if (TArray<FCombat_ThreatData>* Threats = ThreatMap.Find(TargetActor))
    {
        Threats->RemoveAll([ThreatActor](const FCombat_ThreatData& Threat)
        {
            return Threat.ThreatActor == ThreatActor;
        });
    }
}

float UCombat_CombatStateManager::GetThreatLevel(AActor* ThreatActor, AActor* TargetActor) const
{
    if (!ThreatActor || !TargetActor)
    {
        return 0.0f;
    }
    
    if (const TArray<FCombat_ThreatData>* Threats = ThreatMap.Find(TargetActor))
    {
        for (const FCombat_ThreatData& Threat : *Threats)
        {
            if (Threat.ThreatActor == ThreatActor)
            {
                return Threat.ThreatLevel;
            }
        }
    }
    
    return 0.0f;
}

TArray<FCombat_ThreatData> UCombat_CombatStateManager::GetThreatsForActor(AActor* Actor) const
{
    if (const TArray<FCombat_ThreatData>* Threats = ThreatMap.Find(Actor))
    {
        return *Threats;
    }
    
    return TArray<FCombat_ThreatData>();
}

TArray<AActor*> UCombat_CombatStateManager::GetNearbyEnemies(AActor* Actor, float Radius) const
{
    TArray<AActor*> NearbyEnemies;
    
    if (!Actor)
    {
        return NearbyEnemies;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (const auto& CombatPair : CombatActors)
    {
        AActor* OtherActor = CombatPair.Key;
        if (OtherActor && OtherActor != Actor)
        {
            float Distance = FVector::Dist(ActorLocation, OtherActor->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyEnemies.Add(OtherActor);
            }
        }
    }
    
    return NearbyEnemies;
}

AActor* UCombat_CombatStateManager::GetNearestEnemy(AActor* Actor, float MaxRange) const
{
    if (!Actor)
    {
        return nullptr;
    }
    
    AActor* NearestEnemy = nullptr;
    float NearestDistance = MaxRange;
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (const auto& CombatPair : CombatActors)
    {
        AActor* OtherActor = CombatPair.Key;
        if (OtherActor && OtherActor != Actor)
        {
            float Distance = FVector::Dist(ActorLocation, OtherActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestEnemy = OtherActor;
            }
        }
    }
    
    return NearestEnemy;
}

bool UCombat_CombatStateManager::IsLocationSafe(const FVector& Location, float SafetyRadius) const
{
    for (const auto& CombatPair : CombatActors)
    {
        AActor* CombatActor = CombatPair.Key;
        if (CombatActor)
        {
            float Distance = FVector::Dist(Location, CombatActor->GetActorLocation());
            if (Distance < SafetyRadius)
            {
                return false;
            }
        }
    }
    
    return true;
}

int32 UCombat_CombatStateManager::GetActiveCombatCount() const
{
    return CombatStartTimes.Num();
}

float UCombat_CombatStateManager::GetAverageCombatDuration() const
{
    if (RecentCombatEvents.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalDuration = 0.0f;
    int32 CompletedCombats = 0;
    
    // This is a simplified calculation - in a real system you'd track combat end times
    for (const FCombat_CombatEvent& Event : RecentCombatEvents)
    {
        TotalDuration += 15.0f; // Assume average combat duration
        CompletedCombats++;
    }
    
    return CompletedCombats > 0 ? TotalDuration / CompletedCombats : 0.0f;
}

void UCombat_CombatStateManager::UpdateCombatStates()
{
    DecayThreats();
    CleanupOldEvents();
    
    // Check for combat timeouts
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TArray<AActor*> ActorsToEndCombat;
    
    for (const auto& CombatPair : CombatStartTimes)
    {
        float CombatDuration = CurrentTime - CombatPair.Value;
        if (CombatDuration > CombatTimeoutDuration)
        {
            ActorsToEndCombat.Add(CombatPair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToEndCombat)
    {
        EndCombat(Actor);
    }
}

void UCombat_CombatStateManager::DecayThreats()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (auto& ThreatPair : ThreatMap)
    {
        TArray<FCombat_ThreatData>& Threats = ThreatPair.Value;
        
        for (int32 i = Threats.Num() - 1; i >= 0; i--)
        {
            FCombat_ThreatData& Threat = Threats[i];
            
            // Decay threat over time
            float TimeSinceLastSeen = CurrentTime - Threat.LastSeenTime;
            float DecayAmount = ThreatDecayRate * TimeSinceLastSeen;
            Threat.ThreatLevel = FMath::Max(0.0f, Threat.ThreatLevel - DecayAmount);
            
            // Remove if threat is too low or actor is too far
            if (Threat.ThreatLevel <= 0.1f || !Threat.ThreatActor)
            {
                Threats.RemoveAt(i);
            }
            else if (Threat.ThreatActor)
            {
                float Distance = FVector::Dist(ThreatPair.Key->GetActorLocation(), 
                    Threat.ThreatActor->GetActorLocation());
                if (Distance > MaxThreatDistance)
                {
                    Threats.RemoveAt(i);
                }
            }
        }
    }
}

void UCombat_CombatStateManager::CleanupOldEvents()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float MaxEventAge = 300.0f; // 5 minutes
    
    RecentCombatEvents.RemoveAll([CurrentTime, MaxEventAge](const FCombat_CombatEvent& Event)
    {
        return (CurrentTime - Event.Timestamp) > MaxEventAge;
    });
}