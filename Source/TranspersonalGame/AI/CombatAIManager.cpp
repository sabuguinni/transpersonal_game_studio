#include "CombatAIManager.h"
#include "EnemyAIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombatAIManager::UCombatAIManager()
{
    SightRange = 1000.0f;
    HearingRange = 800.0f;
    CombatRange = 300.0f;
    FlankingDistance = 400.0f;
    PackCoordinationRadius = 1000.0f;
}

void UCombatAIManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Initialized"));
    
    // Start pack behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PackUpdateTimerHandle,
            [this]() { UpdatePackBehaviors(0.1f); },
            0.1f,
            true
        );
    }
}

void UCombatAIManager::Deinitialize()
{
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PackUpdateTimerHandle);
    }
    
    // Clear all data
    RegisteredControllers.Empty();
    TacticalDataMap.Empty();
    ActivePacks.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Deinitialized"));
    
    Super::Deinitialize();
}

void UCombatAIManager::RegisterEnemyController(AEnemyAIController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Attempted to register null controller"));
        return;
    }
    
    if (!RegisteredControllers.Contains(Controller))
    {
        RegisteredControllers.Add(Controller);
        
        // Initialize tactical data
        FCombat_TacticalData TacticalData;
        TacticalDataMap.Add(Controller, TacticalData);
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Registered controller %s"), 
               Controller->GetPawn() ? *Controller->GetPawn()->GetName() : TEXT("Unknown"));
    }
}

void UCombatAIManager::UnregisterEnemyController(AEnemyAIController* Controller)
{
    if (!Controller)
    {
        return;
    }
    
    // Remove from any packs
    DisbandPack(Controller);
    
    // Remove from registered controllers
    RegisteredControllers.Remove(Controller);
    TacticalDataMap.Remove(Controller);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Unregistered controller"));
}

void UCombatAIManager::CreatePack(const TArray<AEnemyAIController*>& Members, AEnemyAIController* Leader)
{
    if (Members.Num() < 2 || !Leader || !Members.Contains(Leader))
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Invalid pack creation parameters"));
        return;
    }
    
    // Check if any members are already in packs
    for (AEnemyAIController* Member : Members)
    {
        if (GetPackData(Member))
        {
            DisbandPack(Member);
        }
    }
    
    // Create new pack
    FCombat_PackData NewPack;
    NewPack.PackMembers = Members;
    NewPack.PackLeader = Leader;
    NewPack.PackState = ECombat_TacticalState::Patrol;
    
    if (Leader->GetPawn())
    {
        NewPack.PackTargetLocation = Leader->GetPawn()->GetActorLocation();
    }
    
    ActivePacks.Add(NewPack);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Created pack with %d members"), Members.Num());
}

void UCombatAIManager::DisbandPack(AEnemyAIController* PackMember)
{
    for (int32 i = ActivePacks.Num() - 1; i >= 0; i--)
    {
        if (ActivePacks[i].PackMembers.Contains(PackMember))
        {
            UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Disbanded pack"));
            ActivePacks.RemoveAt(i);
            break;
        }
    }
}

FCombat_PackData* UCombatAIManager::GetPackData(AEnemyAIController* Controller)
{
    for (FCombat_PackData& Pack : ActivePacks)
    {
        if (Pack.PackMembers.Contains(Controller))
        {
            return &Pack;
        }
    }
    return nullptr;
}

void UCombatAIManager::UpdateTacticalState(AEnemyAIController* Controller, ECombat_TacticalState NewState)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        Data->CurrentState = NewState;
        
        // Update pack state if this is a pack leader
        if (FCombat_PackData* PackData = GetPackData(Controller))
        {
            if (PackData->PackLeader == Controller)
            {
                PackData->PackState = NewState;
            }
        }
    }
}

void UCombatAIManager::SetThreatLevel(AEnemyAIController* Controller, ECombat_ThreatLevel ThreatLevel)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        Data->ThreatLevel = ThreatLevel;
    }
}

void UCombatAIManager::ReportPlayerSighting(AEnemyAIController* Controller, const FVector& PlayerLocation)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        Data->LastKnownPlayerLocation = PlayerLocation;
        Data->LastPlayerSightTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        // Alert nearby enemies
        TArray<AEnemyAIController*> NearbyEnemies = GetNearbyEnemies(PlayerLocation, PackCoordinationRadius);
        for (AEnemyAIController* NearbyEnemy : NearbyEnemies)
        {
            if (NearbyEnemy != Controller)
            {
                if (FCombat_TacticalData* NearbyData = TacticalDataMap.Find(NearbyEnemy))
                {
                    NearbyData->LastKnownPlayerLocation = PlayerLocation;
                    NearbyData->LastPlayerSightTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
                }
            }
        }
    }
}

void UCombatAIManager::InitiateCombat(AEnemyAIController* Controller, APawn* Target)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        Data->bIsInCombat = true;
        Data->CombatStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        Data->CurrentState = ECombat_TacticalState::Engage;
        
        if (Target)
        {
            Data->LastKnownPlayerLocation = Target->GetActorLocation();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Combat initiated"));
    }
}

void UCombatAIManager::EndCombat(AEnemyAIController* Controller)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        Data->bIsInCombat = false;
        Data->CurrentState = ECombat_TacticalState::Patrol;
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Combat ended"));
    }
}

bool UCombatAIManager::IsInCombat(AEnemyAIController* Controller)
{
    if (FCombat_TacticalData* Data = TacticalDataMap.Find(Controller))
    {
        return Data->bIsInCombat;
    }
    return false;
}

TArray<AEnemyAIController*> UCombatAIManager::GetNearbyEnemies(const FVector& Location, float Radius)
{
    TArray<AEnemyAIController*> NearbyEnemies;
    
    for (AEnemyAIController* Controller : RegisteredControllers)
    {
        if (Controller && Controller->GetPawn())
        {
            float Distance = FVector::Dist(Controller->GetPawn()->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyEnemies.Add(Controller);
            }
        }
    }
    
    return NearbyEnemies;
}

FVector UCombatAIManager::GetOptimalFlankingPosition(const FVector& TargetLocation, const FVector& EnemyLocation)
{
    FVector Direction = (TargetLocation - EnemyLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    
    // Calculate flanking position to the right
    FVector FlankingPosition = TargetLocation + (RightVector * FlankingDistance);
    
    return FlankingPosition;
}

bool UCombatAIManager::CanSeePlayer(AEnemyAIController* Controller)
{
    if (!Controller || !Controller->GetPawn())
    {
        return false;
    }
    
    // Get player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(Controller->GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Distance > SightRange)
    {
        return false;
    }
    
    // Line trace for line of sight
    FHitResult HitResult;
    FVector Start = Controller->GetPawn()->GetActorLocation();
    FVector End = PlayerPawn->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility
    );
    
    // If we hit the player or nothing, we can see them
    return !bHit || HitResult.GetActor() == PlayerPawn;
}

void UCombatAIManager::UpdatePackBehaviors(float DeltaTime)
{
    // Clean up invalid controllers first
    CleanupInvalidControllers();
    
    // Update each active pack
    for (FCombat_PackData& Pack : ActivePacks)
    {
        if (ValidatePackIntegrity(Pack))
        {
            CoordinatePackHunting(Pack);
        }
    }
}

void UCombatAIManager::CoordinatePackHunting(FCombat_PackData& PackData)
{
    if (!PackData.PackLeader || !PackData.PackLeader->GetPawn())
    {
        return;
    }
    
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check if pack should engage
    float DistanceToPlayer = FVector::Dist(PackData.PackLeader->GetPawn()->GetActorLocation(), PlayerLocation);
    
    if (DistanceToPlayer <= CombatRange && PackData.PackState != ECombat_TacticalState::PackHunt)
    {
        PackData.PackState = ECombat_TacticalState::PackHunt;
        PackData.PackTargetLocation = PlayerLocation;
        
        // Assign flanking positions
        AssignFlankingPositions(PackData, PlayerLocation);
    }
}

void UCombatAIManager::AssignFlankingPositions(FCombat_PackData& PackData, const FVector& TargetLocation)
{
    if (PackData.PackMembers.Num() < 2)
    {
        return;
    }
    
    FVector LeaderLocation = PackData.PackLeader->GetPawn() ? 
        PackData.PackLeader->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    
    // Assign positions around the target
    float AngleStep = 360.0f / PackData.PackMembers.Num();
    
    for (int32 i = 0; i < PackData.PackMembers.Num(); i++)
    {
        if (PackData.PackMembers[i] && PackData.PackMembers[i] != PackData.PackLeader)
        {
            float Angle = FMath::DegreesToRadians(AngleStep * i);
            FVector Offset = FVector(
                FMath::Cos(Angle) * FlankingDistance,
                FMath::Sin(Angle) * FlankingDistance,
                0.0f
            );
            
            FVector FlankingPos = TargetLocation + Offset;
            
            // Update tactical data for this member
            if (FCombat_TacticalData* Data = TacticalDataMap.Find(PackData.PackMembers[i]))
            {
                Data->CurrentState = ECombat_TacticalState::Flank;
                Data->LastKnownPlayerLocation = FlankingPos;
            }
        }
    }
}

bool UCombatAIManager::ValidatePackIntegrity(FCombat_PackData& PackData)
{
    // Remove invalid members
    PackData.PackMembers.RemoveAll([](AEnemyAIController* Controller) {
        return !IsValid(Controller) || !IsValid(Controller->GetPawn());
    });
    
    // Check if pack still has enough members
    if (PackData.PackMembers.Num() < 2)
    {
        return false;
    }
    
    // Validate leader
    if (!IsValid(PackData.PackLeader) || !PackData.PackMembers.Contains(PackData.PackLeader))
    {
        // Assign new leader
        PackData.PackLeader = PackData.PackMembers[0];
    }
    
    return true;
}

void UCombatAIManager::CleanupInvalidControllers()
{
    RegisteredControllers.RemoveAll([this](AEnemyAIController* Controller) {
        if (!IsValid(Controller))
        {
            TacticalDataMap.Remove(Controller);
            return true;
        }
        return false;
    });
}