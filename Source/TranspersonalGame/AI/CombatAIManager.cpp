#include "CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
    
    CombatUpdateInterval = 0.5f;
    MaxCombatDistance = 5000.0f;
    LastCombatUpdate = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: System initialized"));
    
    // Initialize combat zones
    ProcessCombatZones();
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastCombatUpdate += DeltaTime;
    if (LastCombatUpdate >= CombatUpdateInterval)
    {
        UpdateCombatStates();
        HandlePackBehavior();
        LastCombatUpdate = 0.0f;
    }
}

void ACombatAIManager::RegisterCombatZone(ATriggerVolume* Zone, ECombat_ThreatLevel ThreatLevel)
{
    if (!Zone) return;
    
    CombatZones.Add(Zone, ThreatLevel);
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Registered combat zone %s with threat level %d"), 
           *Zone->GetName(), (int32)ThreatLevel);
}

void ACombatAIManager::UnregisterCombatZone(ATriggerVolume* Zone)
{
    if (!Zone) return;
    
    CombatZones.Remove(Zone);
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Unregistered combat zone %s"), *Zone->GetName());
}

void ACombatAIManager::FormPack(const TArray<AActor*>& PackMembers, ECombat_Formation Formation)
{
    if (PackMembers.Num() < 2) return;
    
    for (AActor* Member : PackMembers)
    {
        if (!Member) continue;
        
        FCombat_TacticalData& Data = CombatData.FindOrAdd(Member);
        Data.Formation = Formation;
        Data.PackSize = PackMembers.Num();
        Data.ThreatLevel = ECombat_ThreatLevel::Hunting;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Formed pack of %d members in %d formation"), 
           PackMembers.Num(), (int32)Formation);
}

void ACombatAIManager::DisbandPack(const TArray<AActor*>& PackMembers)
{
    for (AActor* Member : PackMembers)
    {
        if (!Member) continue;
        
        FCombat_TacticalData& Data = CombatData.FindOrAdd(Member);
        Data.Formation = ECombat_Formation::None;
        Data.PackSize = 1;
        Data.ThreatLevel = ECombat_ThreatLevel::Passive;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Disbanded pack of %d members"), PackMembers.Num());
}

ECombat_ThreatLevel ACombatAIManager::AssessThreat(AActor* Target, AActor* Observer)
{
    if (!Target || !Observer) return ECombat_ThreatLevel::Passive;
    
    float Distance = FVector::Dist(Target->GetActorLocation(), Observer->GetActorLocation());
    
    // Distance-based threat assessment
    if (Distance < 500.0f)
        return ECombat_ThreatLevel::Enraged;
    else if (Distance < 1000.0f)
        return ECombat_ThreatLevel::Aggressive;
    else if (Distance < 2000.0f)
        return ECombat_ThreatLevel::Cautious;
    
    return ECombat_ThreatLevel::Passive;
}

bool ACombatAIManager::ShouldEngageTarget(AActor* Predator, AActor* Target)
{
    if (!Predator || !Target) return false;
    
    FCombat_TacticalData* Data = CombatData.Find(Predator);
    if (!Data) return false;
    
    float Distance = FVector::Dist(Predator->GetActorLocation(), Target->GetActorLocation());
    
    // Engage if within aggression radius and threat level is high enough
    return (Distance <= Data->AggressionRadius && 
            Data->ThreatLevel >= ECombat_ThreatLevel::Aggressive);
}

FVector ACombatAIManager::GetOptimalAttackPosition(AActor* Attacker, AActor* Target, ECombat_Formation Formation)
{
    if (!Attacker || !Target) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector AttackerLocation = Attacker->GetActorLocation();
    
    switch (Formation)
    {
        case ECombat_Formation::Circle:
            return CalculateFlankPosition(Target, Attacker, 800.0f);
            
        case ECombat_Formation::Triangle:
            return CalculateFlankPosition(Target, Attacker, 600.0f);
            
        case ECombat_Formation::Ambush:
            return TargetLocation + FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0);
            
        default:
            return TargetLocation + (AttackerLocation - TargetLocation).GetSafeNormal() * 500.0f;
    }
}

void ACombatAIManager::CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target)
{
    if (!Target || PackMembers.Num() == 0) return;
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        AActor* Member = PackMembers[i];
        if (!Member) continue;
        
        FCombat_TacticalData* Data = CombatData.Find(Member);
        if (!Data) continue;
        
        FVector AttackPosition = GetOptimalAttackPosition(Member, Target, Data->Formation);
        
        // Move to attack position (this would be handled by AI controller in full implementation)
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Pack member %s moving to attack position"), 
               *Member->GetName());
    }
}

void ACombatAIManager::UpdateCombatStates()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Update threat levels for all tracked actors
    for (auto& CombatPair : CombatData)
    {
        AActor* Actor = CombatPair.Key;
        FCombat_TacticalData& Data = CombatPair.Value;
        
        if (!Actor) continue;
        
        // Find nearby threats
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
        
        ECombat_ThreatLevel HighestThreat = ECombat_ThreatLevel::Passive;
        
        for (AActor* OtherActor : FoundActors)
        {
            if (OtherActor == Actor) continue;
            
            float Distance = FVector::Dist(Actor->GetActorLocation(), OtherActor->GetActorLocation());
            if (Distance > MaxCombatDistance) continue;
            
            ECombat_ThreatLevel CurrentThreat = AssessThreat(OtherActor, Actor);
            if (CurrentThreat > HighestThreat)
            {
                HighestThreat = CurrentThreat;
            }
        }
        
        Data.ThreatLevel = HighestThreat;
    }
}

void ACombatAIManager::ProcessCombatZones()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> TriggerVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, ATriggerVolume::StaticClass(), TriggerVolumes);
    
    for (AActor* Actor : TriggerVolumes)
    {
        ATriggerVolume* Volume = Cast<ATriggerVolume>(Actor);
        if (!Volume) continue;
        
        FString VolumeName = Volume->GetName().ToLower();
        if (VolumeName.Contains("combat"))
        {
            RegisterCombatZone(Volume, ECombat_ThreatLevel::Aggressive);
        }
    }
}

void ACombatAIManager::HandlePackBehavior()
{
    // Group nearby actors of the same type into packs
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);
    
    // Simple pack formation for actors with "raptor" in their name
    TArray<AActor*> Raptors;
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetName().ToLower().Contains("raptor"))
        {
            Raptors.Add(Actor);
        }
    }
    
    if (Raptors.Num() >= 3)
    {
        FormPack(Raptors, ECombat_Formation::Triangle);
    }
}

FVector ACombatAIManager::CalculateFlankPosition(AActor* Target, AActor* Flanker, float Distance)
{
    if (!Target || !Flanker) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector FlankerLocation = Flanker->GetActorLocation();
    
    // Calculate perpendicular position for flanking
    FVector Direction = (FlankerLocation - TargetLocation).GetSafeNormal();
    FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    
    // Choose left or right flank randomly
    float FlankSide = FMath::RandBool() ? 1.0f : -1.0f;
    
    return TargetLocation + (Perpendicular * FlankSide * Distance);
}