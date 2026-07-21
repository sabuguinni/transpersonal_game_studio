#include "Quest_HuntingMissionSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"

UQuest_HuntingMissionSystem::UQuest_HuntingMissionSystem()
{
    NextMissionID = 1;
}

void UQuest_HuntingMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeHuntMissionTemplates();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HuntingMissionSystem initialized"));
}

void UQuest_HuntingMissionSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionTimerHandle);
    }
    
    ActiveHuntMissions.Empty();
    CompletedMissionIDs.Empty();
    
    Super::Deinitialize();
}

FString UQuest_HuntingMissionSystem::CreateHuntMission(const FString& MissionName, EQuest_HuntTargetType TargetType, int32 TargetCount, float TimeLimit)
{
    if (!ValidateMissionParameters(MissionName, TargetType, TargetCount))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid hunt mission parameters"));
        return FString();
    }

    FString MissionID = FString::Printf(TEXT("HUNT_%d"), NextMissionID++);
    
    FQuest_HuntMission NewMission = CreateBasicHuntTemplate(TargetType);
    NewMission.MissionID = MissionID;
    NewMission.MissionName = MissionName;
    NewMission.TimeLimit = TimeLimit;
    NewMission.TimeRemaining = TimeLimit;
    
    // Set up hunt target
    if (NewMission.Targets.Num() > 0)
    {
        NewMission.Targets[0].TargetType = TargetType;
        NewMission.Targets[0].RequiredCount = TargetCount;
        NewMission.Targets[0].DifficultyRating = CalculateHuntDifficulty(TargetType, TargetCount);
    }
    
    ActiveHuntMissions.Add(MissionID, NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created hunt mission: %s (ID: %s)"), *MissionName, *MissionID);
    
    return MissionID;
}

bool UQuest_HuntingMissionSystem::StartHuntMission(const FString& MissionID)
{
    if (!ActiveHuntMissions.Contains(MissionID))
    {
        return false;
    }
    
    FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
    
    // Activate all targets
    for (FQuest_HuntTarget& Target : Mission.Targets)
    {
        Target.bIsActive = true;
    }
    
    // Start mission timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MissionTimerHandle, this, &UQuest_HuntingMissionSystem::CheckMissionTimeouts, 1.0f, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started hunt mission: %s"), *MissionID);
    
    return true;
}

bool UQuest_HuntingMissionSystem::CompleteHuntMission(const FString& MissionID)
{
    if (!ActiveHuntMissions.Contains(MissionID))
    {
        return false;
    }
    
    FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
    Mission.bIsCompleted = true;
    
    // Grant rewards
    GrantHuntRewards(MissionID);
    
    // Move to completed list
    CompletedMissionIDs.Add(MissionID);
    ActiveHuntMissions.Remove(MissionID);
    
    UE_LOG(LogTemp, Warning, TEXT("Completed hunt mission: %s"), *MissionID);
    
    return true;
}

void UQuest_HuntingMissionSystem::UpdateHuntProgress(const FString& MissionID, const FString& SpeciesName, const FVector& KillLocation)
{
    if (!ActiveHuntMissions.Contains(MissionID))
    {
        return;
    }
    
    FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
    
    // Find matching target
    for (FQuest_HuntTarget& Target : Mission.Targets)
    {
        if (Target.SpeciesName == SpeciesName && Target.bIsActive)
        {
            Target.RequiredCount--;
            Target.LastKnownLocation = KillLocation;
            
            UE_LOG(LogTemp, Warning, TEXT("Hunt progress updated - %s: %d remaining"), *SpeciesName, Target.RequiredCount);
            
            // Check if target is complete
            if (Target.RequiredCount <= 0)
            {
                Target.bIsActive = false;
                
                // Check if all targets are complete
                bool bAllTargetsComplete = true;
                for (const FQuest_HuntTarget& CheckTarget : Mission.Targets)
                {
                    if (CheckTarget.RequiredCount > 0)
                    {
                        bAllTargetsComplete = false;
                        break;
                    }
                }
                
                if (bAllTargetsComplete)
                {
                    CompleteHuntMission(MissionID);
                }
            }
            break;
        }
    }
}

FQuest_HuntMission UQuest_HuntingMissionSystem::GetHuntMission(const FString& MissionID)
{
    if (ActiveHuntMissions.Contains(MissionID))
    {
        return ActiveHuntMissions[MissionID];
    }
    
    return FQuest_HuntMission();
}

TArray<FQuest_HuntMission> UQuest_HuntingMissionSystem::GetActiveHuntMissions()
{
    TArray<FQuest_HuntMission> Missions;
    for (const auto& MissionPair : ActiveHuntMissions)
    {
        Missions.Add(MissionPair.Value);
    }
    return Missions;
}

void UQuest_HuntingMissionSystem::UpdateMissionTimers(float DeltaTime)
{
    TArray<FString> MissionsToRemove;
    
    for (auto& MissionPair : ActiveHuntMissions)
    {
        FQuest_HuntMission& Mission = MissionPair.Value;
        Mission.TimeRemaining -= DeltaTime;
        
        if (Mission.TimeRemaining <= 0.0f)
        {
            Mission.bIsFailed = true;
            MissionsToRemove.Add(MissionPair.Key);
            UE_LOG(LogTemp, Warning, TEXT("Hunt mission failed due to timeout: %s"), *MissionPair.Key);
        }
    }
    
    // Remove failed missions
    for (const FString& MissionID : MissionsToRemove)
    {
        ActiveHuntMissions.Remove(MissionID);
    }
}

bool UQuest_HuntingMissionSystem::IsHuntMissionActive(const FString& MissionID)
{
    return ActiveHuntMissions.Contains(MissionID);
}

void UQuest_HuntingMissionSystem::CancelHuntMission(const FString& MissionID)
{
    if (ActiveHuntMissions.Contains(MissionID))
    {
        ActiveHuntMissions.Remove(MissionID);
        UE_LOG(LogTemp, Warning, TEXT("Cancelled hunt mission: %s"), *MissionID);
    }
}

void UQuest_HuntingMissionSystem::SetHuntingGrounds(const FString& MissionID, const FVector& Location, float Radius)
{
    if (ActiveHuntMissions.Contains(MissionID))
    {
        FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
        Mission.HuntingGrounds = Location;
        Mission.HuntingRadius = Radius;
    }
}

bool UQuest_HuntingMissionSystem::IsInHuntingGrounds(const FString& MissionID, const FVector& Location)
{
    if (!ActiveHuntMissions.Contains(MissionID))
    {
        return false;
    }
    
    const FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
    float Distance = FVector::Dist(Location, Mission.HuntingGrounds);
    return Distance <= Mission.HuntingRadius;
}

float UQuest_HuntingMissionSystem::CalculateHuntDifficulty(EQuest_HuntTargetType TargetType, int32 Count)
{
    float BaseDifficulty = 1.0f;
    
    switch (TargetType)
    {
        case EQuest_HuntTargetType::SmallPrey:
            BaseDifficulty = 1.0f;
            break;
        case EQuest_HuntTargetType::MediumPrey:
            BaseDifficulty = 2.0f;
            break;
        case EQuest_HuntTargetType::LargePrey:
            BaseDifficulty = 3.5f;
            break;
        case EQuest_HuntTargetType::Predator:
            BaseDifficulty = 5.0f;
            break;
        case EQuest_HuntTargetType::ApexPredator:
            BaseDifficulty = 8.0f;
            break;
    }
    
    return BaseDifficulty * FMath::Sqrt(static_cast<float>(Count));
}

void UQuest_HuntingMissionSystem::GrantHuntRewards(const FString& MissionID)
{
    if (!ActiveHuntMissions.Contains(MissionID))
    {
        return;
    }
    
    const FQuest_HuntMission& Mission = ActiveHuntMissions[MissionID];
    
    // Calculate total difficulty for reward scaling
    float TotalDifficulty = 0.0f;
    for (const FQuest_HuntTarget& Target : Mission.Targets)
    {
        TotalDifficulty += Target.DifficultyRating;
    }
    
    // Grant experience and resources based on difficulty
    int32 ExperienceGained = FMath::RoundToInt(TotalDifficulty * 100.0f);
    int32 MeatGained = FMath::RoundToInt(TotalDifficulty * 50.0f);
    int32 HideGained = FMath::RoundToInt(TotalDifficulty * 25.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt rewards granted - XP: %d, Meat: %d, Hide: %d"), ExperienceGained, MeatGained, HideGained);
}

void UQuest_HuntingMissionSystem::InitializeHuntMissionTemplates()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing hunt mission templates"));
}

FQuest_HuntMission UQuest_HuntingMissionSystem::CreateBasicHuntTemplate(EQuest_HuntTargetType TargetType)
{
    FQuest_HuntMission Template;
    
    FQuest_HuntTarget Target;
    Target.TargetType = TargetType;
    Target.bIsActive = false;
    
    switch (TargetType)
    {
        case EQuest_HuntTargetType::SmallPrey:
            Target.SpeciesName = TEXT("Compsognathus");
            Template.Description = TEXT("Hunt small prey for meat and experience");
            break;
        case EQuest_HuntTargetType::MediumPrey:
            Target.SpeciesName = TEXT("Parasaurolophus");
            Template.Description = TEXT("Hunt medium herbivores for substantial resources");
            break;
        case EQuest_HuntTargetType::LargePrey:
            Target.SpeciesName = TEXT("Triceratops");
            Template.Description = TEXT("Hunt large herbivores - dangerous but rewarding");
            break;
        case EQuest_HuntTargetType::Predator:
            Target.SpeciesName = TEXT("Velociraptor");
            Template.Description = TEXT("Hunt pack predators - extreme danger");
            break;
        case EQuest_HuntTargetType::ApexPredator:
            Target.SpeciesName = TEXT("Tyrannosaurus");
            Template.Description = TEXT("Hunt the apex predator - ultimate challenge");
            break;
    }
    
    Template.Targets.Add(Target);
    Template.HuntingGrounds = FVector(0, 0, 100); // Savanna biome
    Template.HuntingRadius = 15000.0f;
    
    return Template;
}

void UQuest_HuntingMissionSystem::CheckMissionTimeouts()
{
    UpdateMissionTimers(1.0f);
}

bool UQuest_HuntingMissionSystem::ValidateHuntTarget(const FQuest_HuntTarget& Target)
{
    return !Target.SpeciesName.IsEmpty() && Target.RequiredCount > 0;
}

bool UQuest_HuntingMissionSystem::ValidateMissionParameters(const FString& MissionName, EQuest_HuntTargetType TargetType, int32 TargetCount)
{
    return !MissionName.IsEmpty() && TargetCount > 0 && TargetCount <= 10;
}