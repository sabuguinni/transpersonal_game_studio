#include "Quest_ObjectiveTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_ObjectiveTracker::UQuest_ObjectiveTracker()
{
}

void UQuest_ObjectiveTracker::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultObjectives();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Objective Tracker initialized"));
}

void UQuest_ObjectiveTracker::AddObjective(const FQuest_Objective& Objective)
{
    if (!Objective.ObjectiveID.IsEmpty())
    {
        Objectives.Add(Objective.ObjectiveID, Objective);
        UE_LOG(LogTemp, Warning, TEXT("Added objective: %s - %s"), *Objective.ObjectiveID, *Objective.Title);
    }
}

void UQuest_ObjectiveTracker::RemoveObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        Objectives.Remove(ObjectiveID);
        UE_LOG(LogTemp, Warning, TEXT("Removed objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveTracker::ActivateObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        FQuest_Objective& Objective = Objectives[ObjectiveID];
        Objective.Status = EQuest_ObjectiveStatus::Active;
        
        OnObjectiveUpdated.Broadcast(Objective);
        UE_LOG(LogTemp, Warning, TEXT("Activated objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveTracker::CompleteObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        FQuest_Objective& Objective = Objectives[ObjectiveID];
        Objective.Status = EQuest_ObjectiveStatus::Completed;
        Objective.CurrentCount = Objective.RequiredCount;
        
        OnObjectiveCompleted.Broadcast(Objective);
        UE_LOG(LogTemp, Warning, TEXT("Completed objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveTracker::FailObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        FQuest_Objective& Objective = Objectives[ObjectiveID];
        Objective.Status = EQuest_ObjectiveStatus::Failed;
        
        OnObjectiveUpdated.Broadcast(Objective);
        UE_LOG(LogTemp, Warning, TEXT("Failed objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveTracker::UpdateObjectiveProgress(const FQuest_ProgressEvent& ProgressEvent)
{
    if (Objectives.Contains(ProgressEvent.ObjectiveID))
    {
        FQuest_Objective& Objective = Objectives[ProgressEvent.ObjectiveID];
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && MatchesProgressEvent(Objective, ProgressEvent))
        {
            Objective.CurrentCount = FMath::Min(Objective.CurrentCount + ProgressEvent.ProgressAmount, Objective.RequiredCount);
            
            OnObjectiveUpdated.Broadcast(Objective);
            
            // Check if objective is now complete
            if (Objective.CurrentCount >= Objective.RequiredCount)
            {
                CompleteObjective(ProgressEvent.ObjectiveID);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Updated objective progress: %s (%d/%d)"), 
                   *ProgressEvent.ObjectiveID, Objective.CurrentCount, Objective.RequiredCount);
        }
    }
}

FQuest_Objective UQuest_ObjectiveTracker::GetObjective(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        return Objectives[ObjectiveID];
    }
    
    return FQuest_Objective();
}

TArray<FQuest_Objective> UQuest_ObjectiveTracker::GetActiveObjectives()
{
    TArray<FQuest_Objective> ActiveObjectives;
    
    for (const auto& ObjectivePair : Objectives)
    {
        if (ObjectivePair.Value.Status == EQuest_ObjectiveStatus::Active && !ObjectivePair.Value.bIsHidden)
        {
            ActiveObjectives.Add(ObjectivePair.Value);
        }
    }
    
    return ActiveObjectives;
}

TArray<FQuest_Objective> UQuest_ObjectiveTracker::GetCompletedObjectives()
{
    TArray<FQuest_Objective> CompletedObjectives;
    
    for (const auto& ObjectivePair : Objectives)
    {
        if (ObjectivePair.Value.Status == EQuest_ObjectiveStatus::Completed)
        {
            CompletedObjectives.Add(ObjectivePair.Value);
        }
    }
    
    return CompletedObjectives;
}

bool UQuest_ObjectiveTracker::IsObjectiveComplete(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        return Objectives[ObjectiveID].Status == EQuest_ObjectiveStatus::Completed;
    }
    
    return false;
}

int32 UQuest_ObjectiveTracker::GetObjectiveProgress(const FString& ObjectiveID)
{
    if (Objectives.Contains(ObjectiveID))
    {
        return Objectives[ObjectiveID].CurrentCount;
    }
    
    return 0;
}

void UQuest_ObjectiveTracker::InitializeDefaultObjectives()
{
    // Hunt Objectives
    FQuest_Objective HuntTRex;
    HuntTRex.ObjectiveID = TEXT("hunt_trex_01");
    HuntTRex.Title = TEXT("Hunt the Apex Predator");
    HuntTRex.Description = TEXT("Defeat a Tyrannosaurus Rex and bring back its tooth as proof");
    HuntTRex.ObjectiveType = EQuest_ObjectiveType::Kill_Dinosaur;
    HuntTRex.Status = EQuest_ObjectiveStatus::Inactive;
    HuntTRex.RequiredCount = 1;
    HuntTRex.TargetName = TEXT("TRex");
    AddObjective(HuntTRex);
    
    FQuest_Objective HuntRaptors;
    HuntRaptors.ObjectiveID = TEXT("hunt_raptors_01");
    HuntRaptors.Title = TEXT("Pack Hunter Challenge");
    HuntRaptors.Description = TEXT("Defeat 3 Velociraptors and collect their claws");
    HuntRaptors.ObjectiveType = EQuest_ObjectiveType::Kill_Dinosaur;
    HuntRaptors.Status = EQuest_ObjectiveStatus::Inactive;
    HuntRaptors.RequiredCount = 3;
    HuntRaptors.TargetName = TEXT("Raptor");
    AddObjective(HuntRaptors);
    
    FQuest_Objective HuntTriceratops;
    HuntTriceratops.ObjectiveID = TEXT("hunt_triceratops_01");
    HuntTriceratops.Title = TEXT("Armored Giant Hunt");
    HuntTriceratops.Description = TEXT("Take down a Triceratops and harvest its hide");
    HuntTriceratops.ObjectiveType = EQuest_ObjectiveType::Kill_Dinosaur;
    HuntTriceratops.Status = EQuest_ObjectiveStatus::Inactive;
    HuntTriceratops.RequiredCount = 1;
    HuntTriceratops.TargetName = TEXT("Triceratops");
    AddObjective(HuntTriceratops);
    
    // Gathering Objectives
    FQuest_Objective GatherWood;
    GatherWood.ObjectiveID = TEXT("gather_wood_01");
    GatherWood.Title = TEXT("Collect Ancient Wood");
    GatherWood.Description = TEXT("Gather 10 pieces of strong wood from ancient trees");
    GatherWood.ObjectiveType = EQuest_ObjectiveType::Gather_Resource;
    GatherWood.Status = EQuest_ObjectiveStatus::Inactive;
    GatherWood.RequiredCount = 10;
    GatherWood.TargetName = TEXT("Wood");
    AddObjective(GatherWood);
    
    FQuest_Objective GatherStone;
    GatherStone.ObjectiveID = TEXT("gather_stone_01");
    GatherStone.Title = TEXT("Sacred Stone Collection");
    GatherStone.Description = TEXT("Collect 15 sacred stones for tool crafting");
    GatherStone.ObjectiveType = EQuest_ObjectiveType::Gather_Resource;
    GatherStone.Status = EQuest_ObjectiveStatus::Inactive;
    GatherStone.RequiredCount = 15;
    GatherStone.TargetName = TEXT("Stone");
    AddObjective(GatherStone);
    
    FQuest_Objective GatherFood;
    GatherFood.ObjectiveID = TEXT("gather_food_01");
    GatherFood.Title = TEXT("Sustenance Gathering");
    GatherFood.Description = TEXT("Gather 20 berries and edible plants for the tribe");
    GatherFood.ObjectiveType = EQuest_ObjectiveType::Gather_Resource;
    GatherFood.Status = EQuest_ObjectiveStatus::Inactive;
    GatherFood.RequiredCount = 20;
    GatherFood.TargetName = TEXT("Food");
    AddObjective(GatherFood);
    
    // Exploration Objectives
    FQuest_Objective ExploreCanyon;
    ExploreCanyon.ObjectiveID = TEXT("explore_canyon_01");
    ExploreCanyon.Title = TEXT("Canyon Reconnaissance");
    ExploreCanyon.Description = TEXT("Explore the mysterious canyon and report back");
    ExploreCanyon.ObjectiveType = EQuest_ObjectiveType::Explore_Area;
    ExploreCanyon.Status = EQuest_ObjectiveStatus::Inactive;
    ExploreCanyon.RequiredCount = 1;
    ExploreCanyon.TargetName = TEXT("Canyon");
    ExploreCanyon.TargetLocation = FVector(1000, 1000, 0);
    AddObjective(ExploreCanyon);
    
    FQuest_Objective ExploreForest;
    ExploreForest.ObjectiveID = TEXT("explore_forest_01");
    ExploreForest.Title = TEXT("Deep Forest Survey");
    ExploreForest.Description = TEXT("Map the dangerous deep forest territories");
    ExploreForest.ObjectiveType = EQuest_ObjectiveType::Explore_Area;
    ExploreForest.Status = EQuest_ObjectiveStatus::Inactive;
    ExploreForest.RequiredCount = 1;
    ExploreForest.TargetName = TEXT("Forest");
    ExploreForest.TargetLocation = FVector(-1000, 1000, 0);
    AddObjective(ExploreForest);
    
    // Survival Objectives
    FQuest_Objective SurviveNight;
    SurviveNight.ObjectiveID = TEXT("survive_night_01");
    SurviveNight.Title = TEXT("Night Survival Test");
    SurviveNight.Description = TEXT("Survive through the dangerous night without shelter");
    SurviveNight.ObjectiveType = EQuest_ObjectiveType::Survive_Duration;
    SurviveNight.Status = EQuest_ObjectiveStatus::Inactive;
    SurviveNight.RequiredCount = 480; // 8 minutes (representing night duration)
    AddObjective(SurviveNight);
}

bool UQuest_ObjectiveTracker::MatchesProgressEvent(const FQuest_Objective& Objective, const FQuest_ProgressEvent& Event)
{
    // Check if the event type matches the objective type
    if (Objective.ObjectiveType != Event.EventType)
    {
        return false;
    }
    
    // For specific target objectives, check if the target name matches
    if (!Objective.TargetName.IsEmpty() && !Event.TargetName.IsEmpty())
    {
        return Objective.TargetName.Equals(Event.TargetName, ESearchCase::IgnoreCase);
    }
    
    // For location-based objectives, check distance
    if (Objective.ObjectiveType == EQuest_ObjectiveType::Explore_Area || 
        Objective.ObjectiveType == EQuest_ObjectiveType::Reach_Location)
    {
        float Distance = FVector::Dist(Objective.TargetLocation, Event.EventLocation);
        return Distance <= 500.0f; // Within 500 units
    }
    
    return true;
}