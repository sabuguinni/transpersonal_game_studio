#include "NPCDailyRoutineSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

UNPC_DailyRoutineSystem::UNPC_DailyRoutineSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentActivity = ENPC_ActivityType::Resting;
    ActivityTransitionTime = 2.0f;
    bUseGameTimeOfDay = true;
    SimulatedTimeScale = 1.0f;
    CurrentGameHour = 8.0f;
    bIsTransitioning = false;
    CurrentScheduledActivity = nullptr;
    LastActivityCheckTime = 0.0f;

    // Default daily schedule for primitive human
    FNPC_ScheduledActivity SleepActivity;
    SleepActivity.ActivityType = ENPC_ActivityType::Sleeping;
    SleepActivity.StartHour = 22.0f;
    SleepActivity.EndHour = 6.0f;
    SleepActivity.Priority = 10.0f;
    SleepActivity.bIsFlexible = false;
    DailySchedule.Add(SleepActivity);

    FNPC_ScheduledActivity ForagingActivity;
    ForagingActivity.ActivityType = ENPC_ActivityType::Foraging;
    ForagingActivity.StartHour = 7.0f;
    ForagingActivity.EndHour = 11.0f;
    ForagingActivity.Priority = 8.0f;
    ForagingActivity.bIsFlexible = true;
    DailySchedule.Add(ForagingActivity);

    FNPC_ScheduledActivity CraftingActivity;
    CraftingActivity.ActivityType = ENPC_ActivityType::Crafting;
    CraftingActivity.StartHour = 12.0f;
    CraftingActivity.EndHour = 16.0f;
    CraftingActivity.Priority = 6.0f;
    CraftingActivity.bIsFlexible = true;
    DailySchedule.Add(CraftingActivity);

    FNPC_ScheduledActivity SocializingActivity;
    SocializingActivity.ActivityType = ENPC_ActivityType::Socializing;
    SocializingActivity.StartHour = 17.0f;
    SocializingActivity.EndHour = 21.0f;
    SocializingActivity.Priority = 5.0f;
    SocializingActivity.bIsFlexible = true;
    DailySchedule.Add(SocializingActivity);
}

void UNPC_DailyRoutineSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateCurrentGameTime();
    CurrentActivity = GetScheduledActivityForTime(CurrentGameHour);
    
    UE_LOG(LogTemp, Log, TEXT("NPC Daily Routine System initialized for %s"), *GetOwner()->GetName());
}

void UNPC_DailyRoutineSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsTransitioning)
    {
        UpdateCurrentGameTime();
        CheckForActivityTransition();
    }
}

void UNPC_DailyRoutineSystem::UpdateCurrentGameTime()
{
    if (bUseGameTimeOfDay)
    {
        // In a real implementation, this would get time from a TimeOfDay system
        // For now, simulate time progression
        CurrentGameHour += GetWorld()->GetDeltaSeconds() * SimulatedTimeScale * 0.1f;
        
        if (CurrentGameHour >= 24.0f)
        {
            CurrentGameHour -= 24.0f;
        }
    }
}

void UNPC_DailyRoutineSystem::CheckForActivityTransition()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastActivityCheckTime < 5.0f) // Check every 5 seconds
    {
        return;
    }
    
    LastActivityCheckTime = CurrentTime;
    
    ENPC_ActivityType ScheduledActivity = GetScheduledActivityForTime(CurrentGameHour);
    if (ScheduledActivity != CurrentActivity)
    {
        ExecuteActivityTransition(ScheduledActivity);
    }
}

void UNPC_DailyRoutineSystem::SetCurrentActivity(ENPC_ActivityType NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        CurrentActivity = NewActivity;
        UE_LOG(LogTemp, Log, TEXT("NPC %s activity changed to: %d"), *GetOwner()->GetName(), (int32)NewActivity);
    }
}

ENPC_ActivityType UNPC_DailyRoutineSystem::GetScheduledActivityForTime(float GameHour)
{
    FNPC_ScheduledActivity* BestActivity = nullptr;
    float HighestPriority = -1.0f;
    
    for (FNPC_ScheduledActivity& Activity : DailySchedule)
    {
        bool bIsInTimeRange = false;
        
        if (Activity.StartHour <= Activity.EndHour)
        {
            // Normal time range (e.g., 8:00 to 16:00)
            bIsInTimeRange = (GameHour >= Activity.StartHour && GameHour <= Activity.EndHour);
        }
        else
        {
            // Overnight time range (e.g., 22:00 to 6:00)
            bIsInTimeRange = (GameHour >= Activity.StartHour || GameHour <= Activity.EndHour);
        }
        
        if (bIsInTimeRange && Activity.Priority > HighestPriority)
        {
            BestActivity = &Activity;
            HighestPriority = Activity.Priority;
        }
    }
    
    return BestActivity ? BestActivity->ActivityType : ENPC_ActivityType::Resting;
}

void UNPC_DailyRoutineSystem::AddScheduledActivity(const FNPC_ScheduledActivity& NewActivity)
{
    DailySchedule.Add(NewActivity);
    UE_LOG(LogTemp, Log, TEXT("Added new scheduled activity for NPC %s"), *GetOwner()->GetName());
}

void UNPC_DailyRoutineSystem::RemoveScheduledActivity(int32 ActivityIndex)
{
    if (DailySchedule.IsValidIndex(ActivityIndex))
    {
        DailySchedule.RemoveAt(ActivityIndex);
        UE_LOG(LogTemp, Log, TEXT("Removed scheduled activity %d for NPC %s"), ActivityIndex, *GetOwner()->GetName());
    }
}

FVector UNPC_DailyRoutineSystem::GetCurrentTargetLocation() const
{
    for (const FNPC_ScheduledActivity& Activity : DailySchedule)
    {
        if (Activity.ActivityType == CurrentActivity)
        {
            return Activity.TargetLocation;
        }
    }
    
    return GetOwner()->GetActorLocation();
}

bool UNPC_DailyRoutineSystem::ShouldTransitionToNewActivity()
{
    ENPC_ActivityType ScheduledActivity = GetScheduledActivityForTime(CurrentGameHour);
    return (ScheduledActivity != CurrentActivity && !bIsTransitioning);
}

void UNPC_DailyRoutineSystem::ForceActivityTransition(ENPC_ActivityType NewActivity)
{
    ExecuteActivityTransition(NewActivity);
}

void UNPC_DailyRoutineSystem::ExecuteActivityTransition(ENPC_ActivityType NewActivity)
{
    if (bIsTransitioning)
    {
        return;
    }
    
    bIsTransitioning = true;
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s transitioning from %d to %d"), 
           *GetOwner()->GetName(), (int32)CurrentActivity, (int32)NewActivity);
    
    // Set timer for transition completion
    GetWorld()->GetTimerManager().SetTimer(ActivityTransitionTimer, 
        [this, NewActivity]()
        {
            SetCurrentActivity(NewActivity);
            bIsTransitioning = false;
        }, 
        ActivityTransitionTime, false);
}