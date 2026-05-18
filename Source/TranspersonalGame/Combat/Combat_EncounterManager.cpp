#include "Combat_EncounterManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UCombat_EncounterManager::UCombat_EncounterManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = EncounterUpdateInterval;
    
    // Initialize encounter data
    CurrentEncounter = FCombat_EncounterData();
    LastUpdateTime = 0.0f;
}

void UCombat_EncounterManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Encounter Manager initialized"));
    
    // Set up initial encounter monitoring
    if (GetWorld())
    {
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UCombat_EncounterManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentEncounter.bIsActive)
    {
        UpdateEncounter(DeltaTime);
    }
}

void UCombat_EncounterManager::InitiateEncounter(ECombat_EncounterType Type, const TArray<AActor*>& Participants, FVector Center, float Radius)
{
    // End any existing encounter
    if (CurrentEncounter.bIsActive)
    {
        EndEncounter(false);
    }
    
    // Initialize new encounter
    CurrentEncounter.EncounterType = Type;
    CurrentEncounter.ParticipatingActors = Participants;
    CurrentEncounter.EncounterCenter = Center;
    CurrentEncounter.EncounterRadius = Radius;
    CurrentEncounter.Duration = 0.0f;
    CurrentEncounter.bIsActive = true;
    
    // Calculate threat level based on participants
    if (Participants.Num() > 0)
    {
        CurrentEncounter.ThreatLevel = CalculateThreatLevel(Participants, nullptr);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Encounter Initiated: Type=%d, Participants=%d, ThreatLevel=%d"), 
           (int32)Type, Participants.Num(), (int32)CurrentEncounter.ThreatLevel);
    
    // Broadcast encounter start
    BroadcastEncounterEvent(TEXT("EncounterStarted"), CurrentEncounter);
}

void UCombat_EncounterManager::EndEncounter(bool bPlayerVictory)
{
    if (!CurrentEncounter.bIsActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Encounter Ended: Duration=%.1f, PlayerVictory=%s"), 
           CurrentEncounter.Duration, bPlayerVictory ? TEXT("True") : TEXT("False"));
    
    // Broadcast encounter end
    BroadcastEncounterEvent(bPlayerVictory ? TEXT("EncounterVictory") : TEXT("EncounterDefeat"), CurrentEncounter);
    
    // Reset encounter state
    CurrentEncounter.bIsActive = false;
    CurrentEncounter.ParticipatingActors.Empty();
    CurrentEncounter.Duration = 0.0f;
}

void UCombat_EncounterManager::UpdateEncounter(float DeltaTime)
{
    if (!CurrentEncounter.bIsActive)
    {
        return;
    }
    
    CurrentEncounter.Duration += DeltaTime;
    
    // Check for encounter timeout
    if (CurrentEncounter.Duration > MaxEncounterDuration)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat Encounter timed out after %.1f seconds"), CurrentEncounter.Duration);
        EndEncounter(false);
        return;
    }
    
    // Process encounter based on type
    switch (CurrentEncounter.EncounterType)
    {
        case ECombat_EncounterType::Ambush:
            ProcessAmbushEncounter(DeltaTime);
            break;
        case ECombat_EncounterType::DirectAssault:
            ProcessDirectAssaultEncounter(DeltaTime);
            break;
        case ECombat_EncounterType::PackHunt:
            ProcessPackHuntEncounter(DeltaTime);
            break;
        case ECombat_EncounterType::TerritorialDefense:
            ProcessTerritorialDefenseEncounter(DeltaTime);
            break;
        case ECombat_EncounterType::Stalking:
            ProcessStalkingEncounter(DeltaTime);
            break;
        case ECombat_EncounterType::Retreat:
            ProcessRetreatEncounter(DeltaTime);
            break;
    }
}

ECombat_EncounterType UCombat_EncounterManager::DetermineOptimalEncounterType(const TArray<AActor*>& Predators, AActor* Target)
{
    if (Predators.Num() == 0 || !Target)
    {
        return ECombat_EncounterType::DirectAssault;
    }
    
    // Single predator logic
    if (Predators.Num() == 1)
    {
        AActor* Predator = Predators[0];
        float Distance = CalculateDistanceToTarget(Predator, Target);
        
        // Check if predator has "TRex" tag for alpha behavior
        if (Predator->Tags.Contains(TEXT("TRex")) || Predator->Tags.Contains(TEXT("Alpha")))
        {
            return Distance > 3000.0f ? ECombat_EncounterType::Stalking : ECombat_EncounterType::DirectAssault;
        }
        
        return ECombat_EncounterType::Ambush;
    }
    
    // Pack behavior logic
    if (Predators.Num() >= 2)
    {
        // Check if pack has coordination
        bool bHasPackTags = false;
        for (AActor* Predator : Predators)
        {
            if (Predator->Tags.Contains(TEXT("Pack")) || Predator->Tags.Contains(TEXT("Raptor")))
            {
                bHasPackTags = true;
                break;
            }
        }
        
        return bHasPackTags ? ECombat_EncounterType::PackHunt : ECombat_EncounterType::DirectAssault;
    }
    
    return ECombat_EncounterType::DirectAssault;
}

ECombat_ThreatLevel UCombat_EncounterManager::CalculateThreatLevel(const TArray<AActor*>& Threats, AActor* Target)
{
    if (Threats.Num() == 0)
    {
        return ECombat_ThreatLevel::Low;
    }
    
    int32 ThreatScore = 0;
    
    for (AActor* Threat : Threats)
    {
        if (!Threat)
        {
            continue;
        }
        
        // Base threat by type
        if (Threat->Tags.Contains(TEXT("TRex")) || Threat->Tags.Contains(TEXT("Alpha")))
        {
            ThreatScore += 50;
        }
        else if (Threat->Tags.Contains(TEXT("Raptor")) || Threat->Tags.Contains(TEXT("Pack")))
        {
            ThreatScore += 25;
        }
        else if (Threat->Tags.Contains(TEXT("Predator")))
        {
            ThreatScore += 15;
        }
        
        // Distance modifier
        if (Target)
        {
            float Distance = CalculateDistanceToTarget(Threat, Target);
            if (Distance < 1000.0f)
            {
                ThreatScore += 20; // Immediate danger
            }
            else if (Distance < 3000.0f)
            {
                ThreatScore += 10; // Close proximity
            }
        }
    }
    
    // Pack coordination bonus
    if (Threats.Num() >= 3)
    {
        ThreatScore += 30;
    }
    else if (Threats.Num() >= 2)
    {
        ThreatScore += 15;
    }
    
    // Convert score to threat level
    if (ThreatScore >= 100)
    {
        return ECombat_ThreatLevel::Lethal;
    }
    else if (ThreatScore >= 75)
    {
        return ECombat_ThreatLevel::Extreme;
    }
    else if (ThreatScore >= 50)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (ThreatScore >= 25)
    {
        return ECombat_ThreatLevel::Medium;
    }
    
    return ECombat_ThreatLevel::Low;
}

TArray<FVector> UCombat_EncounterManager::GenerateTacticalPositions(FVector Center, int32 NumPositions, float Radius)
{
    TArray<FVector> Positions;
    
    if (NumPositions <= 0)
    {
        return Positions;
    }
    
    float AngleStep = 360.0f / NumPositions;
    
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = AngleStep * i;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector Position = Center + FVector(
            FMath::Cos(RadianAngle) * Radius,
            FMath::Sin(RadianAngle) * Radius,
            0.0f
        );
        
        Positions.Add(Position);
    }
    
    return Positions;
}

void UCombat_EncounterManager::CoordinatePackBehavior(const TArray<AActor*>& PackMembers, AActor* Target)
{
    if (PackMembers.Num() < 2 || !Target)
    {
        return;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    
    // Generate flanking positions
    TArray<FVector> FlankingPositions = GenerateTacticalPositions(TargetLocation, PackMembers.Num(), 2000.0f);
    
    // Assign positions to pack members
    for (int32 i = 0; i < PackMembers.Num() && i < FlankingPositions.Num(); i++)
    {
        AActor* PackMember = PackMembers[i];
        if (PackMember)
        {
            // This would normally set AI behavior tree variables
            UE_LOG(LogTemp, Log, TEXT("Pack Coordination: %s assigned to position (%s)"), 
                   *PackMember->GetName(), *FlankingPositions[i].ToString());
        }
    }
}

FVector UCombat_EncounterManager::GetOptimalFlankingPosition(AActor* Predator, AActor* Target, const TArray<AActor*>& PackMembers)
{
    if (!Predator || !Target)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector PredatorLocation = Predator->GetActorLocation();
    
    // Calculate flanking angle based on pack size
    float BaseAngle = 45.0f; // Base flanking angle
    float AngleOffset = PackMembers.Num() > 2 ? 30.0f : 60.0f;
    
    // Determine left or right flank based on predator position
    FVector ToTarget = (TargetLocation - PredatorLocation).GetSafeNormal();
    FVector RightFlank = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    float FlankDistance = 1500.0f;
    FVector FlankPosition = TargetLocation + (RightFlank * FlankDistance);
    
    return FlankPosition;
}

void UCombat_EncounterManager::ProcessAmbushEncounter(float DeltaTime)
{
    // Ambush logic: predators wait for optimal moment
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Ambush Encounter"));
}

void UCombat_EncounterManager::ProcessDirectAssaultEncounter(float DeltaTime)
{
    // Direct assault: immediate aggressive approach
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Direct Assault Encounter"));
}

void UCombat_EncounterManager::ProcessPackHuntEncounter(float DeltaTime)
{
    // Pack hunt: coordinated flanking and positioning
    if (CurrentEncounter.ParticipatingActors.Num() >= 2)
    {
        CoordinatePackBehavior(CurrentEncounter.ParticipatingActors, nullptr);
    }
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Pack Hunt Encounter"));
}

void UCombat_EncounterManager::ProcessTerritorialDefenseEncounter(float DeltaTime)
{
    // Territorial defense: protect area from intruders
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Territorial Defense Encounter"));
}

void UCombat_EncounterManager::ProcessStalkingEncounter(float DeltaTime)
{
    // Stalking: maintain distance while following
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Stalking Encounter"));
}

void UCombat_EncounterManager::ProcessRetreatEncounter(float DeltaTime)
{
    // Retreat: disengage from combat
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing Retreat Encounter"));
}

float UCombat_EncounterManager::CalculateDistanceToTarget(AActor* Predator, AActor* Target)
{
    if (!Predator || !Target)
    {
        return 99999.0f;
    }
    
    return FVector::Dist(Predator->GetActorLocation(), Target->GetActorLocation());
}

bool UCombat_EncounterManager::IsPlayerInDanger(AActor* Player, const TArray<AActor*>& Threats)
{
    if (!Player || Threats.Num() == 0)
    {
        return false;
    }
    
    for (AActor* Threat : Threats)
    {
        if (Threat && CalculateDistanceToTarget(Threat, Player) < 2000.0f)
        {
            return true;
        }
    }
    
    return false;
}

void UCombat_EncounterManager::BroadcastEncounterEvent(const FString& EventName, const FCombat_EncounterData& Data)
{
    UE_LOG(LogTemp, Warning, TEXT("Combat Event: %s - Type=%d, Duration=%.1f, Participants=%d"), 
           *EventName, (int32)Data.EncounterType, Data.Duration, Data.ParticipatingActors.Num());
}