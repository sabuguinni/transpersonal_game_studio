#include "CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for AI updates
    
    // Set default combat parameters
    GlobalAggressionMultiplier = 1.0f;
    ThreatUpdateInterval = 0.5f;
    MaxSimultaneousCombats = 5;
    bEnablePackBehavior = true;
    bEnableTerritorialBehavior = true;
    
    CurrentActiveCombats = 0;
    LastThreatUpdate = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: System initialized"));
    
    // Initialize threat tracking
    ActiveThreats.Empty();
    RegisteredActors.Empty();
    ActorStates.Empty();
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update threat assessment at intervals
    LastThreatUpdate += DeltaTime;
    if (LastThreatUpdate >= ThreatUpdateInterval)
    {
        UpdateThreatAssessment();
        ProcessCombatStates();
        
        if (bEnablePackBehavior)
        {
            HandlePackCoordination();
        }
        
        LastThreatUpdate = 0.0f;
    }
}

void ACombatAIManager::RegisterCombatActor(AActor* Actor, const FCombat_AIProfile& Profile)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Attempted to register null actor"));
        return;
    }
    
    RegisteredActors.Add(Actor, Profile);
    ActorStates.Add(Actor, ECombat_AIState::Idle);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Registered actor %s for combat AI"), 
           *Actor->GetName());
}

void ACombatAIManager::UnregisterCombatActor(AActor* Actor)
{
    if (!Actor) return;
    
    RegisteredActors.Remove(Actor);
    ActorStates.Remove(Actor);
    
    // Remove from active threats
    ActiveThreats.RemoveAll([Actor](const FCombat_ThreatData& Threat)
    {
        return Threat.ThreatActor == Actor;
    });
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Unregistered actor %s from combat AI"), 
           *Actor->GetName());
}

ECombat_ThreatLevel ACombatAIManager::EvaluateThreat(AActor* Evaluator, AActor* Target)
{
    if (!Evaluator || !Target) return ECombat_ThreatLevel::None;
    
    float ThreatScore = CalculateThreatScore(Evaluator, Target);
    
    if (ThreatScore < 0.2f) return ECombat_ThreatLevel::None;
    if (ThreatScore < 0.4f) return ECombat_ThreatLevel::Low;
    if (ThreatScore < 0.6f) return ECombat_ThreatLevel::Medium;
    if (ThreatScore < 0.8f) return ECombat_ThreatLevel::High;
    
    return ECombat_ThreatLevel::Critical;
}

TArray<AActor*> ACombatAIManager::GetNearbyThreats(AActor* Actor, float Radius)
{
    TArray<AActor*> NearbyThreats;
    
    if (!Actor) return NearbyThreats;
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (const auto& RegisteredPair : RegisteredActors)
    {
        AActor* OtherActor = RegisteredPair.Key;
        if (OtherActor && OtherActor != Actor)
        {
            float Distance = FVector::Dist(ActorLocation, OtherActor->GetActorLocation());
            if (Distance <= Radius)
            {
                ECombat_ThreatLevel ThreatLevel = EvaluateThreat(Actor, OtherActor);
                if (ThreatLevel > ECombat_ThreatLevel::None)
                {
                    NearbyThreats.Add(OtherActor);
                }
            }
        }
    }
    
    return NearbyThreats;
}

void ACombatAIManager::UpdateCombatState(AActor* Actor, ECombat_AIState NewState)
{
    if (!Actor) return;
    
    ECombat_AIState* CurrentState = ActorStates.Find(Actor);
    if (CurrentState && *CurrentState != NewState)
    {
        *CurrentState = NewState;
        
        // Handle state transitions
        switch (NewState)
        {
            case ECombat_AIState::Combat:
                CurrentActiveCombats++;
                break;
                
            case ECombat_AIState::Idle:
            case ECombat_AIState::Patrol:
                if (*CurrentState == ECombat_AIState::Combat)
                {
                    CurrentActiveCombats = FMath::Max(0, CurrentActiveCombats - 1);
                }
                break;
                
            default:
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Actor %s state changed to %d"), 
               *Actor->GetName(), (int32)NewState);
    }
}

TArray<AActor*> ACombatAIManager::GetPackMembers(AActor* PackLeader, float Radius)
{
    TArray<AActor*> PackMembers;
    
    if (!PackLeader) return PackMembers;
    
    const FCombat_AIProfile* LeaderProfile = RegisteredActors.Find(PackLeader);
    if (!LeaderProfile || !LeaderProfile->bIsPackHunter) return PackMembers;
    
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    for (const auto& RegisteredPair : RegisteredActors)
    {
        AActor* OtherActor = RegisteredPair.Key;
        const FCombat_AIProfile& OtherProfile = RegisteredPair.Value;
        
        if (OtherActor && OtherActor != PackLeader && OtherProfile.bIsPackHunter)
        {
            float Distance = FVector::Dist(LeaderLocation, OtherActor->GetActorLocation());
            if (Distance <= Radius)
            {
                PackMembers.Add(OtherActor);
            }
        }
    }
    
    return PackMembers;
}

void ACombatAIManager::CoordinatePackAttack(AActor* PackLeader, AActor* Target)
{
    if (!PackLeader || !Target) return;
    
    const FCombat_AIProfile* LeaderProfile = RegisteredActors.Find(PackLeader);
    if (!LeaderProfile || !LeaderProfile->bIsPackHunter) return;
    
    TArray<AActor*> PackMembers = GetPackMembers(PackLeader, LeaderProfile->PackCoordinationRadius);
    
    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        AActor* Member = PackMembers[i];
        if (i == 0)
        {
            // First member flanks left
            FVector FlankPos = CalculateFlankingPosition(Member, Target, PackLeader);
            // Move to flanking position (would need movement component integration)
        }
        else if (i == 1)
        {
            // Second member flanks right
            FVector FlankPos = CalculateFlankingPosition(Member, Target, PackLeader);
            // Move to flanking position
        }
        else
        {
            // Additional members provide support
            UpdateCombatState(Member, ECombat_AIState::Combat);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Coordinated pack attack with %d members"), 
           PackMembers.Num());
}

FVector ACombatAIManager::CalculateFlankingPosition(AActor* Attacker, AActor* Target, AActor* PackLeader)
{
    if (!Attacker || !Target || !PackLeader) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    // Calculate perpendicular flanking positions
    FVector ToTarget = (TargetLocation - LeaderLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    // Alternate left/right flanking
    static bool bFlankLeft = true;
    bFlankLeft = !bFlankLeft;
    
    float FlankDistance = 800.0f; // Distance to flank from target
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankDistance * (bFlankLeft ? 1.0f : -1.0f));
    
    return FlankPosition;
}

bool ACombatAIManager::IsInTerritory(AActor* Actor, FVector Location)
{
    if (!Actor) return false;
    
    const FCombat_AIProfile* Profile = RegisteredActors.Find(Actor);
    if (!Profile || !Profile->bIsTerritorial) return false;
    
    FVector ActorLocation = Actor->GetActorLocation();
    float Distance = FVector::Dist(ActorLocation, Location);
    
    // Territory radius is based on detection radius
    return Distance <= Profile->DetectionRadius * 0.7f;
}

void ACombatAIManager::DefendTerritory(AActor* Defender, AActor* Intruder)
{
    if (!Defender || !Intruder) return;
    
    const FCombat_AIProfile* DefenderProfile = RegisteredActors.Find(Defender);
    if (!DefenderProfile || !DefenderProfile->bIsTerritorial) return;
    
    // Escalate to alert or combat state
    ECombat_ThreatLevel ThreatLevel = EvaluateThreat(Defender, Intruder);
    
    if (ThreatLevel >= ECombat_ThreatLevel::Medium)
    {
        UpdateCombatState(Defender, ECombat_AIState::Combat);
    }
    else
    {
        UpdateCombatState(Defender, ECombat_AIState::Alert);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s defending territory against %s"), 
           *Defender->GetName(), *Intruder->GetName());
}

void ACombatAIManager::CreateCombatZone(FVector Center, float Radius, ECombat_ThreatLevel MinThreatLevel)
{
    // This would create a combat zone trigger volume
    // Implementation would depend on level design integration
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Created combat zone at %s with radius %f"), 
           *Center.ToString(), Radius);
}

void ACombatAIManager::UpdateThreatAssessment()
{
    ActiveThreats.Empty();
    
    for (const auto& ActorPair : RegisteredActors)
    {
        AActor* Actor = ActorPair.Key;
        const FCombat_AIProfile& Profile = ActorPair.Value;
        
        if (!Actor) continue;
        
        TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, Profile.DetectionRadius);
        
        for (AActor* Threat : NearbyThreats)
        {
            FCombat_ThreatData ThreatData;
            ThreatData.ThreatActor = Threat;
            ThreatData.ThreatLevel = EvaluateThreat(Actor, Threat);
            ThreatData.Distance = FVector::Dist(Actor->GetActorLocation(), Threat->GetActorLocation());
            ThreatData.LastSeenTime = GetWorld()->GetTimeSeconds();
            ThreatData.LastKnownLocation = Threat->GetActorLocation();
            
            ActiveThreats.Add(ThreatData);
        }
    }
}

void ACombatAIManager::ProcessCombatStates()
{
    for (const auto& StatePair : ActorStates)
    {
        AActor* Actor = StatePair.Key;
        ECombat_AIState CurrentState = StatePair.Value;
        
        if (!Actor) continue;
        
        const FCombat_AIProfile* Profile = RegisteredActors.Find(Actor);
        if (!Profile) continue;
        
        // Process state logic
        switch (CurrentState)
        {
            case ECombat_AIState::Idle:
                {
                    TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, Profile->DetectionRadius);
                    if (NearbyThreats.Num() > 0)
                    {
                        UpdateCombatState(Actor, ECombat_AIState::Alert);
                    }
                }
                break;
                
            case ECombat_AIState::Alert:
                {
                    TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, Profile->AttackRange);
                    if (NearbyThreats.Num() > 0 && CanEngageInCombat())
                    {
                        UpdateCombatState(Actor, ECombat_AIState::Combat);
                    }
                }
                break;
                
            case ECombat_AIState::Combat:
                {
                    TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, Profile->AttackRange * 1.5f);
                    if (NearbyThreats.Num() == 0)
                    {
                        UpdateCombatState(Actor, ECombat_AIState::Patrol);
                    }
                }
                break;
                
            default:
                break;
        }
    }
}

void ACombatAIManager::HandlePackCoordination()
{
    // Find pack leaders and coordinate their members
    for (const auto& ActorPair : RegisteredActors)
    {
        AActor* Actor = ActorPair.Key;
        const FCombat_AIProfile& Profile = ActorPair.Value;
        
        if (!Actor || !Profile.bIsPackHunter) continue;
        
        ECombat_AIState* CurrentState = ActorStates.Find(Actor);
        if (!CurrentState || *CurrentState != ECombat_AIState::Combat) continue;
        
        // Find nearby enemies to coordinate against
        TArray<AActor*> NearbyThreats = GetNearbyThreats(Actor, Profile.AttackRange);
        if (NearbyThreats.Num() > 0)
        {
            CoordinatePackAttack(Actor, NearbyThreats[0]);
        }
    }
}

float ACombatAIManager::CalculateThreatScore(AActor* Evaluator, AActor* Target)
{
    if (!Evaluator || !Target) return 0.0f;
    
    const FCombat_AIProfile* EvaluatorProfile = RegisteredActors.Find(Evaluator);
    if (!EvaluatorProfile) return 0.0f;
    
    float Distance = FVector::Dist(Evaluator->GetActorLocation(), Target->GetActorLocation());
    float NormalizedDistance = FMath::Clamp(Distance / EvaluatorProfile->DetectionRadius, 0.0f, 1.0f);
    
    // Base threat score (closer = more threatening)
    float ThreatScore = 1.0f - NormalizedDistance;
    
    // Modify by aggression level
    ThreatScore *= EvaluatorProfile->AggressionLevel * GlobalAggressionMultiplier;
    
    // Check if target is player character (higher threat)
    if (Target->IsA<ACharacter>())
    {
        ThreatScore *= 1.5f;
    }
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

bool ACombatAIManager::CanEngageInCombat() const
{
    return CurrentActiveCombats < MaxSimultaneousCombats;
}