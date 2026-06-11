#include "Narr_StoryProgressionManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize player progress
    CurrentProgress = FNarr_PlayerProgress();
    
    // Initialize story milestones
    InitializeStoryMilestones();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_StoryProgressionManager initialized with %d milestones"), StoryMilestones.Num());
}

void UNarr_StoryProgressionManager::UpdatePlayerProgress(const FString& ProgressType, int32 Value)
{
    if (ProgressType == TEXT("DaysAlive"))
    {
        CurrentProgress.DaysAlive = FMath::Max(CurrentProgress.DaysAlive, Value);
    }
    else if (ProgressType == TEXT("DinosaursSeen"))
    {
        CurrentProgress.DinosaursSeen += Value;
    }
    else if (ProgressType == TEXT("DinosaursSurvived"))
    {
        CurrentProgress.DinosaursSurvived += Value;
    }
    else if (ProgressType == TEXT("ToolsCrafted"))
    {
        CurrentProgress.ToolsCrafted += Value;
    }
    else if (ProgressType == TEXT("SheltersBuilt"))
    {
        CurrentProgress.SheltersBuilt += Value;
    }
    
    // Check if any milestones are now completed
    CheckAllMilestones();
    
    UE_LOG(LogTemp, Log, TEXT("Updated progress: %s = %d"), *ProgressType, Value);
}

bool UNarr_StoryProgressionManager::CheckMilestoneConditions(const FString& MilestoneID)
{
    if (FNarr_StoryMilestone* Milestone = StoryMilestones.Find(MilestoneID))
    {
        if (Milestone->bCompleted)
        {
            return true;
        }
        
        // Check all required conditions
        for (const FString& Condition : Milestone->RequiredConditions)
        {
            if (Condition == TEXT("FirstDay") && CurrentProgress.DaysAlive >= 1)
            {
                continue;
            }
            else if (Condition == TEXT("FirstDinosaur") && CurrentProgress.DinosaursSeen >= 1)
            {
                continue;
            }
            else if (Condition == TEXT("FirstTool") && CurrentProgress.ToolsCrafted >= 1)
            {
                continue;
            }
            else if (Condition == TEXT("FirstShelter") && CurrentProgress.SheltersBuilt >= 1)
            {
                continue;
            }
            else if (Condition == TEXT("SurviveWeek") && CurrentProgress.DaysAlive >= 7)
            {
                continue;
            }
            else
            {
                return false; // Condition not met
            }
        }
        
        return true; // All conditions met
    }
    
    return false;
}

void UNarr_StoryProgressionManager::CompleteMilestone(const FString& MilestoneID)
{
    if (FNarr_StoryMilestone* Milestone = StoryMilestones.Find(MilestoneID))
    {
        if (!Milestone->bCompleted)
        {
            Milestone->bCompleted = true;
            CurrentProgress.CompletedMilestones.Add(MilestoneID);
            
            // Unlock associated content
            UnlockContent(Milestone->UnlockedContent);
            
            // Display milestone completion
            if (GEngine)
            {
                FString CompletionText = FString::Printf(TEXT("MILESTONE COMPLETED: %s"), 
                                                       *Milestone->MilestoneName.ToString());
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, CompletionText);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Completed milestone: %s"), *MilestoneID);
        }
    }
}

FNarr_PlayerProgress UNarr_StoryProgressionManager::GetPlayerProgress() const
{
    return CurrentProgress;
}

TArray<FNarr_StoryMilestone> UNarr_StoryProgressionManager::GetAvailableMilestones() const
{
    TArray<FNarr_StoryMilestone> AvailableMilestones;
    
    for (const auto& MilestonePair : StoryMilestones)
    {
        if (!MilestonePair.Value.bCompleted)
        {
            AvailableMilestones.Add(MilestonePair.Value);
        }
    }
    
    return AvailableMilestones;
}

void UNarr_StoryProgressionManager::TriggerStoryEvent(const FString& EventID, const FVector& Location)
{
    if (EventID == TEXT("DinosaurEncounter"))
    {
        UpdatePlayerProgress(TEXT("DinosaursSeen"), 1);
    }
    else if (EventID == TEXT("DinosaurSurvival"))
    {
        UpdatePlayerProgress(TEXT("DinosaursSurvived"), 1);
    }
    else if (EventID == TEXT("ToolCrafted"))
    {
        UpdatePlayerProgress(TEXT("ToolsCrafted"), 1);
    }
    else if (EventID == TEXT("ShelterBuilt"))
    {
        UpdatePlayerProgress(TEXT("SheltersBuilt"), 1);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Triggered story event: %s at location %s"), *EventID, *Location.ToString());
}

void UNarr_StoryProgressionManager::InitializeStoryMilestones()
{
    // First Day milestone
    FNarr_StoryMilestone FirstDay;
    FirstDay.MilestoneID = TEXT("FirstDay");
    FirstDay.MilestoneName = FText::FromString(TEXT("First Dawn"));
    FirstDay.Description = FText::FromString(TEXT("Survive your first day in the prehistoric world"));
    FirstDay.RequiredConditions.Add(TEXT("FirstDay"));
    FirstDay.UnlockedContent.Add(TEXT("BasicCrafting"));
    StoryMilestones.Add(FirstDay.MilestoneID, FirstDay);
    
    // First Dinosaur milestone
    FNarr_StoryMilestone FirstDinosaur;
    FirstDinosaur.MilestoneID = TEXT("FirstEncounter");
    FirstDinosaur.MilestoneName = FText::FromString(TEXT("Ancient Giants"));
    FirstDinosaur.Description = FText::FromString(TEXT("Witness your first dinosaur in the wild"));
    FirstDinosaur.RequiredConditions.Add(TEXT("FirstDinosaur"));
    FirstDinosaur.UnlockedContent.Add(TEXT("DinosaurLore"));
    StoryMilestones.Add(FirstDinosaur.MilestoneID, FirstDinosaur);
    
    // First Tool milestone
    FNarr_StoryMilestone FirstTool;
    FirstTool.MilestoneID = TEXT("FirstTool");
    FirstTool.MilestoneName = FText::FromString(TEXT("Stone Age Begins"));
    FirstTool.Description = FText::FromString(TEXT("Craft your first primitive tool"));
    FirstTool.RequiredConditions.Add(TEXT("FirstTool"));
    FirstTool.UnlockedContent.Add(TEXT("AdvancedCrafting"));
    StoryMilestones.Add(FirstTool.MilestoneID, FirstTool);
    
    // First Shelter milestone
    FNarr_StoryMilestone FirstShelter;
    FirstShelter.MilestoneID = TEXT("FirstShelter");
    FirstShelter.MilestoneName = FText::FromString(TEXT("Safe Haven"));
    FirstShelter.Description = FText::FromString(TEXT("Build your first shelter from the elements"));
    FirstShelter.RequiredConditions.Add(TEXT("FirstShelter"));
    FirstShelter.UnlockedContent.Add(TEXT("Construction"));
    StoryMilestones.Add(FirstShelter.MilestoneID, FirstShelter);
    
    // Survive Week milestone
    FNarr_StoryMilestone SurviveWeek;
    SurviveWeek.MilestoneID = TEXT("SurviveWeek");
    SurviveWeek.MilestoneName = FText::FromString(TEXT("Seasoned Survivor"));
    SurviveWeek.Description = FText::FromString(TEXT("Survive for seven days in the prehistoric world"));
    SurviveWeek.RequiredConditions.Add(TEXT("SurviveWeek"));
    SurviveWeek.UnlockedContent.Add(TEXT("ExpertSurvival"));
    StoryMilestones.Add(SurviveWeek.MilestoneID, SurviveWeek);
}

void UNarr_StoryProgressionManager::CheckAllMilestones()
{
    for (auto& MilestonePair : StoryMilestones)
    {
        if (!MilestonePair.Value.bCompleted && CheckMilestoneConditions(MilestonePair.Key))
        {
            CompleteMilestone(MilestonePair.Key);
        }
    }
}

void UNarr_StoryProgressionManager::UnlockContent(const TArray<FString>& ContentIDs)
{
    for (const FString& ContentID : ContentIDs)
    {
        UE_LOG(LogTemp, Log, TEXT("Unlocked content: %s"), *ContentID);
        
        // Here you would implement actual content unlocking
        // e.g., enable new crafting recipes, unlock new areas, etc.
    }
}