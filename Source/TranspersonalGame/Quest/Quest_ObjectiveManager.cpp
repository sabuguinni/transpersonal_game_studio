#include "Quest_ObjectiveManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UQuest_ObjectiveManager::UQuest_ObjectiveManager()
{
    // Initialize default values
}

void UQuest_ObjectiveManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default objectives for the prehistoric survival game
    InitializeDefaultObjectives();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Objective Manager initialized"));
}

void UQuest_ObjectiveManager::CreateObjective(const FString& ObjectiveID, const FString& Title, const FString& Description, EQuest_ObjectiveType Type, int32 TargetCount, FVector TargetLocation, float TargetRadius)
{
    FQuest_ObjectiveData NewObjective;
    NewObjective.ObjectiveID = ObjectiveID;
    NewObjective.Title = Title;
    NewObjective.Description = Description;
    NewObjective.Type = Type;
    NewObjective.TargetCount = TargetCount;
    NewObjective.TargetLocation = TargetLocation;
    NewObjective.TargetRadius = TargetRadius;
    NewObjective.Status = EQuest_ObjectiveStatus::Inactive;
    NewObjective.CurrentCount = 0;
    
    Objectives.Add(ObjectiveID, NewObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("Created objective: %s"), *Title);
}

void UQuest_ObjectiveManager::ActivateObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        Objectives[ObjectiveID].Status = EQuest_ObjectiveStatus::Active;
        ActiveObjectiveIDs.AddUnique(ObjectiveID);
        
        BroadcastObjectiveUpdate(ObjectiveID);
        
        UE_LOG(LogTemp, Warning, TEXT("Activated objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount)
{
    if (Objectives.Contains(ObjectiveID))
    {
        FQuest_ObjectiveData& Objective = Objectives[ObjectiveID];
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active)
        {
            Objective.CurrentCount += ProgressAmount;
            
            // Check if objective is completed
            if (Objective.CurrentCount >= Objective.TargetCount)
            {
                CompleteObjective(ObjectiveID);
            }
            else
            {
                BroadcastObjectiveUpdate(ObjectiveID);
            }
        }
    }
}

void UQuest_ObjectiveManager::CompleteObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        Objectives[ObjectiveID].Status = EQuest_ObjectiveStatus::Completed;
        Objectives[ObjectiveID].CurrentCount = Objectives[ObjectiveID].TargetCount;
        
        ActiveObjectiveIDs.Remove(ObjectiveID);
        
        BroadcastObjectiveUpdate(ObjectiveID);
        
        UE_LOG(LogTemp, Warning, TEXT("Completed objective: %s"), *ObjectiveID);
    }
}

bool UQuest_ObjectiveManager::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    if (Objectives.Contains(ObjectiveID))
    {
        return Objectives[ObjectiveID].Status == EQuest_ObjectiveStatus::Completed;
    }
    return false;
}

TArray<FQuest_ObjectiveData> UQuest_ObjectiveManager::GetActiveObjectives() const
{
    TArray<FQuest_ObjectiveData> ActiveObjectives;
    
    for (const FString& ObjectiveID : ActiveObjectiveIDs)
    {
        if (Objectives.Contains(ObjectiveID))
        {
            ActiveObjectives.Add(Objectives[ObjectiveID]);
        }
    }
    
    return ActiveObjectives;
}

FQuest_ObjectiveData UQuest_ObjectiveManager::GetObjectiveData(const FString& ObjectiveID) const
{
    if (Objectives.Contains(ObjectiveID))
    {
        return Objectives[ObjectiveID];
    }
    
    return FQuest_ObjectiveData();
}

void UQuest_ObjectiveManager::CheckLocationObjectives(FVector PlayerLocation)
{
    for (const FString& ObjectiveID : ActiveObjectiveIDs)
    {
        if (Objectives.Contains(ObjectiveID))
        {
            const FQuest_ObjectiveData& Objective = Objectives[ObjectiveID];
            
            if (Objective.Type == EQuest_ObjectiveType::Explore_Area)
            {
                float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                
                if (Distance <= Objective.TargetRadius)
                {
                    UpdateObjectiveProgress(ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_ObjectiveManager::InitializeDefaultObjectives()
{
    // Hunt the Great Mammoth
    CreateObjective(
        "hunt_mammoth_001",
        "Hunt the Great Mammoth",
        "A massive mammoth has been spotted near the frozen lake. Hunt it down for meat and tusks.",
        EQuest_ObjectiveType::Hunt_Target,
        1,
        FVector(3000, 2500, 150),
        800.0f
    );
    
    // Gather Berries for Winter
    CreateObjective(
        "gather_berries_001",
        "Gather Winter Berries",
        "Collect 20 berries from the northern forest to prepare for the harsh winter.",
        EQuest_ObjectiveType::Gather_Resource,
        20,
        FVector(-1000, 2000, 100),
        500.0f
    );
    
    // Build Emergency Shelter
    CreateObjective(
        "build_shelter_001",
        "Build Emergency Shelter",
        "Construct a shelter using wood and stone to protect against predators.",
        EQuest_ObjectiveType::Build_Structure,
        1,
        FVector(500, -1500, 100),
        400.0f
    );
    
    // Tame a Raptor
    CreateObjective(
        "tame_raptor_001",
        "Tame a Velociraptor",
        "Approach and tame a raptor using meat and patience. They make excellent hunting companions.",
        EQuest_ObjectiveType::Tame_Creature,
        1,
        FVector(-2000, -1000, 100),
        600.0f
    );
    
    // Explore the Crystal Caves
    CreateObjective(
        "explore_caves_001",
        "Explore the Crystal Caves",
        "Venture deep into the mysterious crystal caves to discover ancient secrets.",
        EQuest_ObjectiveType::Explore_Area,
        1,
        FVector(-2500, 1000, 200),
        300.0f
    );
    
    // Survive the Night
    CreateObjective(
        "survive_night_001",
        "Survive the Night",
        "Stay alive through the dangerous night when predators are most active.",
        EQuest_ObjectiveType::Survive_Duration,
        1,
        FVector::ZeroVector,
        0.0f
    );
}

void UQuest_ObjectiveManager::BroadcastObjectiveUpdate(const FString& ObjectiveID)
{
    // In a full implementation, this would broadcast to UI elements
    // For now, just log the update
    if (Objectives.Contains(ObjectiveID))
    {
        const FQuest_ObjectiveData& Objective = Objectives[ObjectiveID];
        UE_LOG(LogTemp, Warning, TEXT("Objective Update - %s: %d/%d"), 
               *Objective.Title, 
               Objective.CurrentCount, 
               Objective.TargetCount);
    }
}