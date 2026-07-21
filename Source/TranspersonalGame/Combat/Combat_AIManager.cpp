#include "Combat_AIManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombat_AIManager::UCombat_AIManager()
{
    ThreatUpdateInterval = 0.5f;
    MaxThreatDistance = 5000.0f;
    PackCoordinationRadius = 1500.0f;
    LastThreatUpdate = 0.0f;
}

void UCombat_AIManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI Manager initialized"));
    
    // Clear existing data
    CombatActors.Empty();
    PackFormations.Empty();
    ThreatDatabase.Empty();
}

void UCombat_AIManager::Deinitialize()
{
    CombatActors.Empty();
    PackFormations.Empty();
    ThreatDatabase.Empty();
    
    Super::Deinitialize();
}

void UCombat_AIManager::RegisterCombatActor(AActor* Actor, ECombat_AIState InitialState)
{
    if (!Actor)
    {
        return;
    }

    CombatActors.Add(Actor, InitialState);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered combat actor: %s with state: %d"), 
           *Actor->GetName(), (int32)InitialState);
}

void UCombat_AIManager::UnregisterCombatActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    CombatActors.Remove(Actor);
    ThreatDatabase.Remove(Actor);
    
    // Remove from any pack formations
    for (auto& Pack : PackFormations)
    {
        Pack.Value.Remove(Actor);
    }
    
    // Remove if pack leader
    PackFormations.Remove(Actor);
}

void UCombat_AIManager::UpdateCombatAI(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastThreatUpdate >= ThreatUpdateInterval)
    {
        UpdateThreatDatabase();
        UpdatePackBehavior();
        LastThreatUpdate = CurrentTime;
    }
}

ECombat_ThreatLevel UCombat_AIManager::EvaluateThreat(AActor* Evaluator, AActor* Target)
{
    if (!Evaluator || !Target)
    {
        return ECombat_ThreatLevel::None;
    }

    float ThreatScore = CalculateThreatScore(Evaluator, Target);
    
    if (ThreatScore >= 0.8f)
    {
        return ECombat_ThreatLevel::Extreme;
    }
    else if (ThreatScore >= 0.6f)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (ThreatScore >= 0.4f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (ThreatScore >= 0.2f)
    {
        return ECombat_ThreatLevel::Low;
    }
    
    return ECombat_ThreatLevel::None;
}

TArray<FCombat_ThreatData> UCombat_AIManager::GetNearbyThreats(AActor* Actor, float SearchRadius)
{
    TArray<FCombat_ThreatData> NearbyThreats;
    
    if (!Actor || !GetWorld())
    {
        return NearbyThreats;
    }

    FVector ActorLocation = Actor->GetActorLocation();
    
    for (auto& ThreatPair : ThreatDatabase)
    {
        if (ThreatPair.Key == Actor)
        {
            continue;
        }

        FCombat_ThreatData& ThreatData = ThreatPair.Value;
        
        if (ThreatData.ThreatActor && IsValid(ThreatData.ThreatActor))
        {
            float Distance = FVector::Dist(ActorLocation, ThreatData.ThreatActor->GetActorLocation());
            
            if (Distance <= SearchRadius)
            {
                ThreatData.Distance = Distance;
                ThreatData.LastKnownPosition = ThreatData.ThreatActor->GetActorLocation();
                NearbyThreats.Add(ThreatData);
            }
        }
    }
    
    // Sort by distance (closest first)
    NearbyThreats.Sort([](const FCombat_ThreatData& A, const FCombat_ThreatData& B) {
        return A.Distance < B.Distance;
    });
    
    return NearbyThreats;
}

void UCombat_AIManager::SetCombatState(AActor* Actor, ECombat_AIState NewState)
{
    if (!Actor)
    {
        return;
    }

    if (CombatActors.Contains(Actor))
    {
        CombatActors[Actor] = NewState;
        
        UE_LOG(LogTemp, Warning, TEXT("Combat state changed for %s: %d"), 
               *Actor->GetName(), (int32)NewState);
    }
}

ECombat_AIState UCombat_AIManager::GetCombatState(AActor* Actor)
{
    if (!Actor || !CombatActors.Contains(Actor))
    {
        return ECombat_AIState::Passive;
    }

    return CombatActors[Actor];
}

void UCombat_AIManager::FormPack(const TArray<AActor*>& PackMembers, AActor* PackLeader)
{
    if (!PackLeader || PackMembers.Num() == 0)
    {
        return;
    }

    PackFormations.Add(PackLeader, PackMembers);
    
    UE_LOG(LogTemp, Warning, TEXT("Pack formed with leader %s and %d members"), 
           *PackLeader->GetName(), PackMembers.Num());
}

void UCombat_AIManager::DisbandPack(AActor* PackLeader)
{
    if (!PackLeader)
    {
        return;
    }

    PackFormations.Remove(PackLeader);
    
    UE_LOG(LogTemp, Warning, TEXT("Pack disbanded for leader %s"), *PackLeader->GetName());
}

TArray<AActor*> UCombat_AIManager::GetPackMembers(AActor* PackLeader)
{
    if (!PackLeader || !PackFormations.Contains(PackLeader))
    {
        return TArray<AActor*>();
    }

    return PackFormations[PackLeader];
}

void UCombat_AIManager::UpdateThreatDatabase()
{
    if (!GetWorld())
    {
        return;
    }

    // Get player character as primary threat
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    
    if (PlayerCharacter)
    {
        FCombat_ThreatData PlayerThreat;
        PlayerThreat.ThreatActor = PlayerCharacter;
        PlayerThreat.ThreatLevel = ECombat_ThreatLevel::High;
        PlayerThreat.LastKnownPosition = PlayerCharacter->GetActorLocation();
        PlayerThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
        
        ThreatDatabase.Add(PlayerCharacter, PlayerThreat);
    }

    // Update threat levels for all registered combat actors
    for (auto& ActorPair : CombatActors)
    {
        if (PlayerCharacter && ActorPair.Key)
        {
            float Distance = FVector::Dist(ActorPair.Key->GetActorLocation(), 
                                         PlayerCharacter->GetActorLocation());
            
            if (Distance <= MaxThreatDistance)
            {
                ECombat_ThreatLevel ThreatLevel = EvaluateThreat(ActorPair.Key, PlayerCharacter);
                
                if (ThreatDatabase.Contains(PlayerCharacter))
                {
                    ThreatDatabase[PlayerCharacter].Distance = Distance;
                    ThreatDatabase[PlayerCharacter].ThreatLevel = ThreatLevel;
                }
            }
        }
    }
}

void UCombat_AIManager::UpdatePackBehavior()
{
    for (auto& PackPair : PackFormations)
    {
        AActor* PackLeader = PackPair.Key;
        TArray<AActor*>& PackMembers = PackPair.Value;
        
        if (!PackLeader || !IsValid(PackLeader))
        {
            continue;
        }

        // Update pack coordination based on leader's state
        ECombat_AIState LeaderState = GetCombatState(PackLeader);
        
        for (AActor* Member : PackMembers)
        {
            if (Member && IsValid(Member))
            {
                // Synchronize pack member states with leader
                ECombat_AIState MemberState = GetCombatState(Member);
                
                if (MemberState != LeaderState && LeaderState == ECombat_AIState::Attacking)
                {
                    SetCombatState(Member, ECombat_AIState::Attacking);
                }
            }
        }
    }
}

float UCombat_AIManager::CalculateThreatScore(AActor* Evaluator, AActor* Target)
{
    if (!Evaluator || !Target)
    {
        return 0.0f;
    }

    float ThreatScore = 0.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(Evaluator->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / MaxThreatDistance), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.4f;
    
    // Player character is always high threat
    if (Cast<ACharacter>(Target))
    {
        ThreatScore += 0.6f;
    }
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}