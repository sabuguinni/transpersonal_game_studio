#include "CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
    
    // Initialize default values
    GlobalThreatLevel = 0.0f;
    MaxActiveCombatants = 10;
    CombatUpdateInterval = 0.5f;
    DinosaurHardCap = 150;
    CurrentDinosaurCount = 0;
    CombatUpdateTimer = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: System initialized"));
    
    // Enforce dinosaur limit on startup
    EnforceDinosaurLimit();
    
    // Initialize combat zones
    CheckCombatZoneActivation();
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CombatUpdateTimer += DeltaTime;
    
    if (CombatUpdateTimer >= CombatUpdateInterval)
    {
        UpdateCombatBehaviors();
        CheckCombatZoneActivation();
        CombatUpdateTimer = 0.0f;
    }
    
    // Update global threat level
    GlobalThreatLevel = CalculateGlobalThreat();
}

void ACombatAIManager::RegisterCombatant(AActor* Combatant, const FCombat_ThreatLevel& ThreatLevel)
{
    if (!Combatant)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Attempted to register null combatant"));
        return;
    }
    
    if (!RegisteredCombatants.Contains(Combatant))
    {
        RegisteredCombatants.Add(Combatant);
        ThreatLevels.Add(Combatant, ThreatLevel);
        
        // Initialize behavior state
        FCombat_BehaviorState InitialState;
        InitialState.CurrentState = ECombat_AIState::Patrol;
        CombatantStates.Add(Combatant, InitialState);
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Registered combatant %s"), *Combatant->GetName());
    }
}

void ACombatAIManager::UnregisterCombatant(AActor* Combatant)
{
    if (Combatant && RegisteredCombatants.Contains(Combatant))
    {
        RegisteredCombatants.Remove(Combatant);
        ThreatLevels.Remove(Combatant);
        CombatantStates.Remove(Combatant);
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Unregistered combatant %s"), *Combatant->GetName());
    }
}

void ACombatAIManager::UpdateCombatBehaviors()
{
    for (AActor* Combatant : RegisteredCombatants)
    {
        if (!Combatant || !IsValid(Combatant))
        {
            continue;
        }
        
        FCombat_BehaviorState* State = CombatantStates.Find(Combatant);
        FCombat_ThreatLevel* Threat = ThreatLevels.Find(Combatant);
        
        if (!State || !Threat)
        {
            continue;
        }
        
        // Update state timer
        State->StateTimer += CombatUpdateInterval;
        
        // Basic AI state machine
        switch (State->CurrentState)
        {
            case ECombat_AIState::Patrol:
            {
                // Look for threats
                AActor* NearestThreat = FindNearestThreat(Combatant, Threat->DetectionRadius);
                if (NearestThreat)
                {
                    State->CurrentState = ECombat_AIState::Alert;
                    State->TargetActor = NearestThreat;
                    State->LastKnownLocation = NearestThreat->GetActorLocation();
                    State->StateTimer = 0.0f;
                }
                break;
            }
            
            case ECombat_AIState::Alert:
            {
                if (State->TargetActor && IsValid(State->TargetActor))
                {
                    float DistanceToTarget = FVector::Dist(Combatant->GetActorLocation(), State->TargetActor->GetActorLocation());
                    
                    if (DistanceToTarget <= Threat->DetectionRadius * 0.5f && Threat->AggressionLevel > 0.7f)
                    {
                        State->CurrentState = ECombat_AIState::Combat;
                        State->StateTimer = 0.0f;
                    }
                    else if (DistanceToTarget > Threat->DetectionRadius * 1.5f)
                    {
                        State->CurrentState = ECombat_AIState::Patrol;
                        State->TargetActor = nullptr;
                        State->StateTimer = 0.0f;
                    }
                }
                else
                {
                    State->CurrentState = ECombat_AIState::Patrol;
                    State->TargetActor = nullptr;
                    State->StateTimer = 0.0f;
                }
                break;
            }
            
            case ECombat_AIState::Combat:
            {
                if (!State->TargetActor || !IsValid(State->TargetActor))
                {
                    State->CurrentState = ECombat_AIState::Alert;
                    State->StateTimer = 0.0f;
                }
                else
                {
                    float DistanceToTarget = FVector::Dist(Combatant->GetActorLocation(), State->TargetActor->GetActorLocation());
                    
                    if (DistanceToTarget > Threat->DetectionRadius * 2.0f)
                    {
                        State->CurrentState = ECombat_AIState::Patrol;
                        State->TargetActor = nullptr;
                        State->StateTimer = 0.0f;
                    }
                }
                break;
            }
            
            case ECombat_AIState::Flee:
            {
                if (State->StateTimer > 10.0f) // Flee for 10 seconds
                {
                    State->CurrentState = ECombat_AIState::Patrol;
                    State->StateTimer = 0.0f;
                }
                break;
            }
        }
    }
}

void ACombatAIManager::SetCombatState(AActor* Combatant, ECombat_AIState NewState)
{
    if (FCombat_BehaviorState* State = CombatantStates.Find(Combatant))
    {
        State->CurrentState = NewState;
        State->StateTimer = 0.0f;
    }
}

ECombat_AIState ACombatAIManager::GetCombatState(AActor* Combatant) const
{
    if (const FCombat_BehaviorState* State = CombatantStates.Find(Combatant))
    {
        return State->CurrentState;
    }
    return ECombat_AIState::Patrol;
}

void ACombatAIManager::EnforceDinosaurLimit()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Find dinosaur actors by label
    TArray<AActor*> DinosaurActors;
    TArray<FString> DinosaurLabels = {
        TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
        TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
    };
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        for (const FString& DinoLabel : DinosaurLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                DinosaurActors.Add(Actor);
                break;
            }
        }
    }
    
    CurrentDinosaurCount = DinosaurActors.Num();
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Found %d dinosaurs (limit: %d)"), CurrentDinosaurCount, DinosaurHardCap);
    
    // Remove excess dinosaurs if over limit
    if (CurrentDinosaurCount > DinosaurHardCap)
    {
        int32 ExcessCount = CurrentDinosaurCount - DinosaurHardCap;
        
        // Shuffle array for random removal
        for (int32 i = DinosaurActors.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            DinosaurActors.Swap(i, j);
        }
        
        // Remove excess dinosaurs
        for (int32 i = 0; i < ExcessCount && i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i] && IsValid(DinosaurActors[i]))
            {
                DinosaurActors[i]->Destroy();
                UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Removed excess dinosaur %s"), *DinosaurActors[i]->GetName());
            }
        }
        
        CurrentDinosaurCount = DinosaurHardCap;
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Enforced dinosaur limit - reduced to %d"), DinosaurHardCap);
    }
}

int32 ACombatAIManager::CountDinosaurs() const
{
    return CurrentDinosaurCount;
}

bool ACombatAIManager::CanSpawnDinosaur() const
{
    return CurrentDinosaurCount < DinosaurHardCap;
}

void ACombatAIManager::RegisterCombatZone(ATriggerBox* Zone)
{
    if (Zone && !CombatZones.Contains(Zone))
    {
        CombatZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Registered combat zone %s"), *Zone->GetName());
    }
}

void ACombatAIManager::CheckCombatZoneActivation()
{
    // Find trigger boxes in the world that could be combat zones
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> TriggerBoxes;
    UGameplayStatics::GetAllActorsOfClass(World, ATriggerBox::StaticClass(), TriggerBoxes);
    
    for (AActor* Actor : TriggerBoxes)
    {
        ATriggerBox* TriggerBox = Cast<ATriggerBox>(Actor);
        if (TriggerBox)
        {
            FString BoxLabel = TriggerBox->GetActorLabel().ToLower();
            if (BoxLabel.Contains(TEXT("combat")) || BoxLabel.Contains(TEXT("predator")) || BoxLabel.Contains(TEXT("pack")))
            {
                RegisterCombatZone(TriggerBox);
            }
        }
    }
}

float ACombatAIManager::CalculateGlobalThreat() const
{
    float TotalThreat = 0.0f;
    int32 ActiveCombatants = 0;
    
    for (const auto& ThreatPair : ThreatLevels)
    {
        if (ThreatPair.Key && IsValid(ThreatPair.Key))
        {
            TotalThreat += ThreatPair.Value.ThreatValue;
            ActiveCombatants++;
        }
    }
    
    return ActiveCombatants > 0 ? TotalThreat / ActiveCombatants : 0.0f;
}

AActor* ACombatAIManager::FindNearestThreat(AActor* FromActor, float SearchRadius) const
{
    if (!FromActor)
    {
        return nullptr;
    }
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = SearchRadius;
    FVector FromLocation = FromActor->GetActorLocation();
    
    for (AActor* Combatant : RegisteredCombatants)
    {
        if (Combatant && IsValid(Combatant) && Combatant != FromActor)
        {
            float Distance = FVector::Dist(FromLocation, Combatant->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestThreat = Combatant;
            }
        }
    }
    
    return NearestThreat;
}

TArray<AActor*> ACombatAIManager::GetThreatsInRadius(const FVector& Location, float Radius) const
{
    TArray<AActor*> ThreatsInRadius;
    
    for (AActor* Combatant : RegisteredCombatants)
    {
        if (Combatant && IsValid(Combatant))
        {
            float Distance = FVector::Dist(Location, Combatant->GetActorLocation());
            if (Distance <= Radius)
            {
                ThreatsInRadius.Add(Combatant);
            }
        }
    }
    
    return ThreatsInRadius;
}