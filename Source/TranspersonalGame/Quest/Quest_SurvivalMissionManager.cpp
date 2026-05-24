#include "Quest_SurvivalMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_SurvivalMissionManager::UQuest_SurvivalMissionManager()
{
    MissionUpdateInterval = 5.0f;
    MaxActiveMissions = 8;
    bAutoCreateMissions = true;
}

void UQuest_SurvivalMissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Initializing survival mission system"));
    
    // Clear any existing missions
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    // Start mission update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionUpdateTimer,
            this,
            &UQuest_SurvivalMissionManager::UpdateMissionProgress,
            MissionUpdateInterval,
            true
        );
    }
    
    // Create initial test missions
    CreateTestSurvivalMissions();
}

void UQuest_SurvivalMissionManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionUpdateTimer);
    }
    
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Survival mission system deinitialized"));
    
    Super::Deinitialize();
}

void UQuest_SurvivalMissionManager::CreateSurvivalMission(const FString& MissionName, EQuest_SurvivalMissionType MissionType, const FVector& Location, EQuest_SurvivalPriority Priority)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Cannot create mission '%s' - max active missions reached"), *MissionName);
        return;
    }
    
    // Check if mission already exists
    if (FindMissionIndex(MissionName) != INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Mission '%s' already exists"), *MissionName);
        return;
    }
    
    FQuest_SurvivalMissionData NewMission;
    NewMission.MissionName = MissionName;
    NewMission.MissionType = MissionType;
    NewMission.Priority = Priority;
    NewMission.TargetLocation = Location;
    NewMission.bIsActive = true;
    NewMission.bIsCompleted = false;
    
    // Set mission-specific parameters
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::HunterGatherer:
            NewMission.RequiredRadius = 300.0f;
            NewMission.RequiredResourceCount = 3;
            NewMission.TimeLimit = 600.0f;
            break;
            
        case EQuest_SurvivalMissionType::ShelterBuilder:
            NewMission.RequiredRadius = 200.0f;
            NewMission.RequiredResourceCount = 5;
            NewMission.TimeLimit = 900.0f;
            break;
            
        case EQuest_SurvivalMissionType::WaterFinder:
            NewMission.RequiredRadius = 100.0f;
            NewMission.RequiredResourceCount = 1;
            NewMission.TimeLimit = 300.0f;
            break;
            
        case EQuest_SurvivalMissionType::FireKeeper:
            NewMission.RequiredRadius = 150.0f;
            NewMission.RequiredResourceCount = 2;
            NewMission.TimeLimit = 450.0f;
            break;
            
        case EQuest_SurvivalMissionType::ToolCrafter:
            NewMission.RequiredRadius = 250.0f;
            NewMission.RequiredResourceCount = 4;
            NewMission.TimeLimit = 750.0f;
            break;
            
        case EQuest_SurvivalMissionType::TerritoryDefender:
            NewMission.RequiredRadius = 500.0f;
            NewMission.RequiredResourceCount = 1;
            NewMission.TimeLimit = 1200.0f;
            break;
    }
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Created survival mission '%s' of type %d at location %s"), 
           *MissionName, (int32)MissionType, *Location.ToString());
}

void UQuest_SurvivalMissionManager::CompleteMission(const FString& MissionName)
{
    int32 MissionIndex = FindMissionIndex(MissionName);
    if (MissionIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Mission '%s' not found for completion"), *MissionName);
        return;
    }
    
    FQuest_SurvivalMissionData CompletedMission = ActiveMissions[MissionIndex];
    CompletedMission.bIsCompleted = true;
    CompletedMission.bIsActive = false;
    
    CompletedMissions.Add(CompletedMission);
    ActiveMissions.RemoveAt(MissionIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Completed survival mission '%s'"), *MissionName);
    
    // Auto-generate new mission if enabled
    if (bAutoCreateMissions && ActiveMissions.Num() < MaxActiveMissions)
    {
        AutoGenerateMissions();
    }
}

void UQuest_SurvivalMissionManager::CancelMission(const FString& MissionName)
{
    int32 MissionIndex = FindMissionIndex(MissionName);
    if (MissionIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Mission '%s' not found for cancellation"), *MissionName);
        return;
    }
    
    ActiveMissions.RemoveAt(MissionIndex);
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Cancelled survival mission '%s'"), *MissionName);
}

TArray<FQuest_SurvivalMissionData> UQuest_SurvivalMissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

FQuest_SurvivalMissionData UQuest_SurvivalMissionManager::GetMissionByName(const FString& MissionName) const
{
    int32 MissionIndex = FindMissionIndex(MissionName);
    if (MissionIndex != INDEX_NONE)
    {
        return ActiveMissions[MissionIndex];
    }
    
    return FQuest_SurvivalMissionData();
}

TArray<FQuest_SurvivalMissionData> UQuest_SurvivalMissionManager::GetMissionsByType(EQuest_SurvivalMissionType MissionType) const
{
    TArray<FQuest_SurvivalMissionData> FilteredMissions;
    
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            FilteredMissions.Add(Mission);
        }
    }
    
    return FilteredMissions;
}

TArray<FQuest_SurvivalMissionData> UQuest_SurvivalMissionManager::GetMissionsByPriority(EQuest_SurvivalPriority Priority) const
{
    TArray<FQuest_SurvivalMissionData> FilteredMissions;
    
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.Priority == Priority)
        {
            FilteredMissions.Add(Mission);
        }
    }
    
    return FilteredMissions;
}

bool UQuest_SurvivalMissionManager::ValidateMissionCompletion(const FString& MissionName, const FVector& PlayerLocation)
{
    int32 MissionIndex = FindMissionIndex(MissionName);
    if (MissionIndex == INDEX_NONE)
    {
        return false;
    }
    
    const FQuest_SurvivalMissionData& Mission = ActiveMissions[MissionIndex];
    float Distance = FVector::Dist(PlayerLocation, Mission.TargetLocation);
    
    return Distance <= Mission.RequiredRadius;
}

void UQuest_SurvivalMissionManager::UpdateMissionProgress(float DeltaTime)
{
    ProcessMissionTimeouts();
    
    if (bAutoCreateMissions && ActiveMissions.Num() < MaxActiveMissions)
    {
        AutoGenerateMissions();
    }
}

void UQuest_SurvivalMissionManager::RegisterResourceCollection(const FString& ResourceType, const FVector& Location)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Resource '%s' collected at %s"), *ResourceType, *Location.ToString());
    
    // Check for relevant hunter-gatherer missions
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionType == EQuest_SurvivalMissionType::HunterGatherer)
        {
            float Distance = FVector::Dist(Location, Mission.TargetLocation);
            if (Distance <= Mission.RequiredRadius)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Resource collection contributes to mission '%s'"), *Mission.MissionName);
            }
        }
    }
}

void UQuest_SurvivalMissionManager::RegisterShelterConstruction(const FVector& Location)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Shelter constructed at %s"), *Location.ToString());
    
    // Auto-complete shelter building missions in range
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionType == EQuest_SurvivalMissionType::ShelterBuilder)
        {
            float Distance = FVector::Dist(Location, Mission.TargetLocation);
            if (Distance <= Mission.RequiredRadius)
            {
                CompleteMission(Mission.MissionName);
                break;
            }
        }
    }
}

void UQuest_SurvivalMissionManager::RegisterFireCreation(const FVector& Location)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Fire created at %s"), *Location.ToString());
    
    // Auto-complete fire keeper missions in range
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionType == EQuest_SurvivalMissionType::FireKeeper)
        {
            float Distance = FVector::Dist(Location, Mission.TargetLocation);
            if (Distance <= Mission.RequiredRadius)
            {
                CompleteMission(Mission.MissionName);
                break;
            }
        }
    }
}

void UQuest_SurvivalMissionManager::RegisterToolCrafting(const FString& ToolType)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Tool '%s' crafted"), *ToolType);
    
    // Complete tool crafting missions
    for (const FQuest_SurvivalMissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionType == EQuest_SurvivalMissionType::ToolCrafter)
        {
            CompleteMission(Mission.MissionName);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::CreateTestSurvivalMissions()
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Creating test survival missions"));
    
    // Create one mission of each type for testing
    CreateSurvivalMission(TEXT("Gather Food"), EQuest_SurvivalMissionType::HunterGatherer, FVector(1000, 0, 100), EQuest_SurvivalPriority::High);
    CreateSurvivalMission(TEXT("Build Shelter"), EQuest_SurvivalMissionType::ShelterBuilder, FVector(0, 1000, 100), EQuest_SurvivalPriority::Critical);
    CreateSurvivalMission(TEXT("Find Water Source"), EQuest_SurvivalMissionType::WaterFinder, FVector(-1000, 0, 50), EQuest_SurvivalPriority::Critical);
    CreateSurvivalMission(TEXT("Maintain Fire"), EQuest_SurvivalMissionType::FireKeeper, FVector(500, 500, 100), EQuest_SurvivalPriority::Medium);
}

void UQuest_SurvivalMissionManager::ClearAllMissions()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: All missions cleared"));
}

void UQuest_SurvivalMissionManager::LogMissionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager: Active missions: %d, Completed missions: %d"), 
           ActiveMissions.Num(), CompletedMissions.Num());
    
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        const FQuest_SurvivalMissionData& Mission = ActiveMissions[i];
        UE_LOG(LogTemp, Warning, TEXT("  [%d] %s (Type: %d, Priority: %d, Location: %s)"), 
               i, *Mission.MissionName, (int32)Mission.MissionType, (int32)Mission.Priority, *Mission.TargetLocation.ToString());
    }
}

void UQuest_SurvivalMissionManager::ProcessMissionTimeouts()
{
    // Implementation for mission timeout processing
    // This would check mission time limits and handle expired missions
}

void UQuest_SurvivalMissionManager::AutoGenerateMissions()
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    // Randomly generate a new mission
    EQuest_SurvivalMissionType RandomType = static_cast<EQuest_SurvivalMissionType>(FMath::RandRange(0, 5));
    FVector RandomLocation = FindSuitableLocationForMission(RandomType);
    EQuest_SurvivalPriority RandomPriority = static_cast<EQuest_SurvivalPriority>(FMath::RandRange(0, 3));
    
    FString MissionName = FString::Printf(TEXT("Auto_%s_%d"), 
                                         *UEnum::GetValueAsString(RandomType), 
                                         FMath::RandRange(1000, 9999));
    
    CreateSurvivalMission(MissionName, RandomType, RandomLocation, RandomPriority);
}

FVector UQuest_SurvivalMissionManager::FindSuitableLocationForMission(EQuest_SurvivalMissionType MissionType)
{
    // Generate random location within reasonable bounds
    float X = FMath::RandRange(-2000.0f, 2000.0f);
    float Y = FMath::RandRange(-2000.0f, 2000.0f);
    float Z = FMath::RandRange(50.0f, 200.0f);
    
    return FVector(X, Y, Z);
}

int32 UQuest_SurvivalMissionManager::FindMissionIndex(const FString& MissionName) const
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}