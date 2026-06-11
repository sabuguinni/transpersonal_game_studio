#include "CombatAIManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TriggerBox.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCombatAIManager::UCombatAIManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CombatUpdateInterval = 0.5f;
    MaxEngagementDistance = 3000.0f;
    LastCombatUpdate = 0.0f;
    PlayerCharacter = nullptr;
}

void UCombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesTactics();
    
    // Find player character
    PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager initialized"));
}

void UCombatAIManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastCombatUpdate += DeltaTime;
    if (LastCombatUpdate >= CombatUpdateInterval)
    {
        UpdateCombatStates(DeltaTime);
        ProcessTacticalDecisions();
        LastCombatUpdate = 0.0f;
    }
}

void UCombatAIManager::RegisterCombatZone(ATriggerBox* Zone, ECombat_ThreatLevel ThreatLevel)
{
    if (!Zone)
    {
        return;
    }
    
    FCombat_EncounterData NewEncounter;
    NewEncounter.Location = Zone->GetActorLocation();
    NewEncounter.ThreatLevel = ThreatLevel;
    NewEncounter.EngagementRadius = 1500.0f;
    NewEncounter.bIsActive = true;
    
    ActiveEncounters.Add(NewEncounter);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat zone registered at %s with threat level %d"), 
           *NewEncounter.Location.ToString(), (int32)ThreatLevel);
}

void UCombatAIManager::CreateTacticalEncounter(FVector Location, const TArray<FString>& DinosaurTypes, ECombat_ThreatLevel ThreatLevel)
{
    FCombat_EncounterData NewEncounter;
    NewEncounter.Location = Location;
    NewEncounter.ThreatLevel = ThreatLevel;
    NewEncounter.DinosaurTypes = DinosaurTypes;
    NewEncounter.EngagementRadius = 2000.0f;
    NewEncounter.bIsActive = true;
    
    ActiveEncounters.Add(NewEncounter);
    
    UE_LOG(LogTemp, Warning, TEXT("Tactical encounter created at %s with %d dinosaur types"), 
           *Location.ToString(), DinosaurTypes.Num());
}

ECombat_ThreatLevel UCombatAIManager::CalculateThreatLevel(APawn* Target, const TArray<APawn*>& Enemies)
{
    if (!Target || Enemies.Num() == 0)
    {
        return ECombat_ThreatLevel::None;
    }
    
    int32 EnemyCount = Enemies.Num();
    float ClosestDistance = MAX_FLT;
    
    for (APawn* Enemy : Enemies)
    {
        if (Enemy)
        {
            float Distance = FVector::Dist(Target->GetActorLocation(), Enemy->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
            }
        }
    }
    
    // Calculate threat based on enemy count and proximity
    if (EnemyCount >= 5 || ClosestDistance < 500.0f)
    {
        return ECombat_ThreatLevel::Extreme;
    }
    else if (EnemyCount >= 3 || ClosestDistance < 1000.0f)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (EnemyCount >= 2 || ClosestDistance < 1500.0f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (EnemyCount >= 1 || ClosestDistance < 2500.0f)
    {
        return ECombat_ThreatLevel::Low;
    }
    
    return ECombat_ThreatLevel::None;
}

void UCombatAIManager::SetDinosaurAIState(APawn* Dinosaur, ECombat_AIState NewState)
{
    if (!Dinosaur)
    {
        return;
    }
    
    DinosaurStates.Add(Dinosaur, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s state changed to %d"), 
           *Dinosaur->GetName(), (int32)NewState);
}

ECombat_AIState UCombatAIManager::GetDinosaurAIState(APawn* Dinosaur) const
{
    if (!Dinosaur)
    {
        return ECombat_AIState::Passive;
    }
    
    const ECombat_AIState* StatePtr = DinosaurStates.Find(Dinosaur);
    return StatePtr ? *StatePtr : ECombat_AIState::Passive;
}

void UCombatAIManager::CoordinatePackAttack(const TArray<APawn*>& PackMembers, APawn* Target)
{
    if (!Target || PackMembers.Num() < 2)
    {
        return;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    
    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        APawn* Member = PackMembers[i];
        if (!Member)
        {
            continue;
        }
        
        if (i == 0)
        {
            // Alpha leads direct attack
            SetDinosaurAIState(Member, ECombat_AIState::Aggressive);
        }
        else
        {
            // Others flank
            FVector FlankPosition = CalculateFlankingPosition(Member, Target);
            SetDinosaurAIState(Member, ECombat_AIState::Hunting);
            
            // Move to flank position (this would be handled by behavior tree in full implementation)
            UE_LOG(LogTemp, Log, TEXT("Pack member %s flanking to %s"), 
                   *Member->GetName(), *FlankPosition.ToString());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack attack coordinated with %d members"), PackMembers.Num());
}

FVector UCombatAIManager::CalculateFlankingPosition(APawn* Attacker, APawn* Target)
{
    if (!Attacker || !Target)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector AttackerLocation = Attacker->GetActorLocation();
    
    // Calculate perpendicular flanking position
    FVector DirectionToTarget = (TargetLocation - AttackerLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    
    // Choose random side for flanking
    float FlankSide = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankPosition = TargetLocation + (RightVector * FlankSide * 800.0f);
    
    return FlankPosition;
}

bool UCombatAIManager::ShouldEngageTarget(APawn* Dinosaur, APawn* Target)
{
    if (!Dinosaur || !Target)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Dinosaur->GetActorLocation(), Target->GetActorLocation());
    
    // Get species tactics
    FString DinosaurName = Dinosaur->GetName();
    FCombat_TacticalData* TacticsPtr = nullptr;
    
    for (auto& SpeciesPair : SpeciesTactics)
    {
        if (DinosaurName.Contains(SpeciesPair.Key))
        {
            TacticsPtr = &SpeciesPair.Value;
            break;
        }
    }
    
    if (!TacticsPtr)
    {
        return Distance < 1000.0f; // Default engagement range
    }
    
    return Distance < TacticsPtr->AttackRange;
}

void UCombatAIManager::OnPlayerEnterCombatZone(ACharacter* Player, ATriggerBox* Zone)
{
    if (!Player || !Zone)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player entered combat zone: %s"), *Zone->GetName());
    
    // Find encounter data for this zone
    FVector ZoneLocation = Zone->GetActorLocation();
    for (FCombat_EncounterData& Encounter : ActiveEncounters)
    {
        if (FVector::Dist(Encounter.Location, ZoneLocation) < 500.0f)
        {
            // Activate encounter
            Encounter.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("Combat encounter activated - Threat Level: %d"), 
                   (int32)Encounter.ThreatLevel);
            break;
        }
    }
}

void UCombatAIManager::OnDinosaurSpotPlayer(APawn* Dinosaur, ACharacter* Player)
{
    if (!Dinosaur || !Player)
    {
        return;
    }
    
    SetDinosaurAIState(Dinosaur, ECombat_AIState::Alert);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s spotted player"), *Dinosaur->GetName());
}

void UCombatAIManager::OnCombatInitiated(APawn* Aggressor, APawn* Target)
{
    if (!Aggressor || !Target)
    {
        return;
    }
    
    SetDinosaurAIState(Aggressor, ECombat_AIState::Aggressive);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat initiated: %s vs %s"), 
           *Aggressor->GetName(), *Target->GetName());
}

void UCombatAIManager::UpdateCombatStates(float DeltaTime)
{
    if (!PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Update all dinosaur states based on player proximity
    for (auto& StatePair : DinosaurStates)
    {
        APawn* Dinosaur = StatePair.Key;
        ECombat_AIState CurrentState = StatePair.Value;
        
        if (!Dinosaur)
        {
            continue;
        }
        
        float DistanceToPlayer = FVector::Dist(Dinosaur->GetActorLocation(), PlayerLocation);
        
        // State transitions based on distance
        if (DistanceToPlayer < 500.0f && CurrentState == ECombat_AIState::Passive)
        {
            SetDinosaurAIState(Dinosaur, ECombat_AIState::Alert);
        }
        else if (DistanceToPlayer < 300.0f && CurrentState == ECombat_AIState::Alert)
        {
            SetDinosaurAIState(Dinosaur, ECombat_AIState::Aggressive);
        }
        else if (DistanceToPlayer > 2000.0f && CurrentState != ECombat_AIState::Passive)
        {
            SetDinosaurAIState(Dinosaur, ECombat_AIState::Passive);
        }
    }
}

void UCombatAIManager::ProcessTacticalDecisions()
{
    // Find nearby dinosaurs for pack coordination
    TArray<APawn*> NearbyDinosaurs;
    
    for (auto& StatePair : DinosaurStates)
    {
        APawn* Dinosaur = StatePair.Key;
        if (Dinosaur && StatePair.Value != ECombat_AIState::Passive)
        {
            NearbyDinosaurs.Add(Dinosaur);
        }
    }
    
    // Group dinosaurs by proximity for pack tactics
    if (NearbyDinosaurs.Num() >= 2 && PlayerCharacter)
    {
        TArray<APawn*> PackMembers;
        
        for (APawn* Dino : NearbyDinosaurs)
        {
            float DistanceToPlayer = FVector::Dist(Dino->GetActorLocation(), PlayerCharacter->GetActorLocation());
            if (DistanceToPlayer < 1500.0f)
            {
                PackMembers.Add(Dino);
            }
        }
        
        if (PackMembers.Num() >= 2)
        {
            CoordinatePackAttack(PackMembers, PlayerCharacter);
        }
    }
}

void UCombatAIManager::InitializeSpeciesTactics()
{
    // T-Rex tactics
    FCombat_TacticalData TRexTactics;
    TRexTactics.AttackRange = 800.0f;
    TRexTactics.FleeDistance = 0.0f; // Never flees
    TRexTactics.GroupCoordination = 0.2f; // Solitary
    TRexTactics.bCanAmbush = false;
    TRexTactics.bPackHunter = false;
    SpeciesTactics.Add("TRex", TRexTactics);
    
    // Velociraptor tactics
    FCombat_TacticalData VelociTactics;
    VelociTactics.AttackRange = 400.0f;
    VelociTactics.FleeDistance = 1200.0f;
    VelociTactics.GroupCoordination = 0.9f; // Highly coordinated
    VelociTactics.bCanAmbush = true;
    VelociTactics.bPackHunter = true;
    SpeciesTactics.Add("Veloci", VelociTactics);
    
    // Triceratops tactics
    FCombat_TacticalData TriceratoTactics;
    TriceratoTactics.AttackRange = 600.0f;
    TriceratoTactics.FleeDistance = 800.0f;
    TriceratoTactics.GroupCoordination = 0.6f; // Herd behavior
    TriceratoTactics.bCanAmbush = false;
    TriceratoTactics.bPackHunter = false;
    SpeciesTactics.Add("Tricera", TriceratoTactics);
    
    UE_LOG(LogTemp, Warning, TEXT("Species tactics initialized for %d species"), SpeciesTactics.Num());
}