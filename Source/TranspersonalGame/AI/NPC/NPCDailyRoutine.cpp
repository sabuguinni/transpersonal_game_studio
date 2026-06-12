#include "NPCDailyRoutine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNPC_DailyRoutine::UNPC_DailyRoutine()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    CurrentActivityIndex = -1;
    TimeScale = 1.0f;
    bUseGameTimeOfDay = true;
}

void UNPC_DailyRoutine::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up default routine if none exists
    if (DailyActivities.Num() == 0)
    {
        AddActivity(ENPC_ActivityType::Sleep, 22.0f, 8.0f, FVector::ZeroVector, 3.0f);
        AddActivity(ENPC_ActivityType::Eat, 7.0f, 1.0f, FVector::ZeroVector, 2.0f);
        AddActivity(ENPC_ActivityType::Work, 8.0f, 8.0f, FVector(1000, 0, 0), 2.0f);
        AddActivity(ENPC_ActivityType::Eat, 12.0f, 1.0f, FVector::ZeroVector, 2.0f);
        AddActivity(ENPC_ActivityType::Socialize, 18.0f, 2.0f, FVector(500, 500, 0), 1.0f);
        AddActivity(ENPC_ActivityType::Eat, 19.0f, 1.0f, FVector::ZeroVector, 2.0f);
        AddActivity(ENPC_ActivityType::Relax, 20.0f, 2.0f, FVector::ZeroVector, 1.0f);
    }
    
    UpdateCurrentActivity();
}

void UNPC_DailyRoutine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCurrentActivity();
}

void UNPC_DailyRoutine::AddActivity(ENPC_ActivityType Type, float StartHour, float Duration, FVector Location, float Priority)
{
    FNPC_RoutineActivity NewActivity;
    NewActivity.ActivityType = Type;
    NewActivity.StartHour = FMath::Fmod(StartHour, 24.0f);
    NewActivity.Duration = Duration;
    NewActivity.TargetLocation = Location;
    NewActivity.Priority = Priority;
    NewActivity.bCanBeInterrupted = (Priority < 2.5f);
    
    DailyActivities.Add(NewActivity);
    
    // Sort activities by start time
    DailyActivities.Sort([](const FNPC_RoutineActivity& A, const FNPC_RoutineActivity& B)
    {
        return A.StartHour < B.StartHour;
    });
}

void UNPC_DailyRoutine::RemoveActivity(int32 ActivityIndex)
{
    if (DailyActivities.IsValidIndex(ActivityIndex))
    {
        DailyActivities.RemoveAt(ActivityIndex);
        UpdateCurrentActivity();
    }
}

FNPC_RoutineActivity UNPC_DailyRoutine::GetCurrentActivity()
{
    if (DailyActivities.IsValidIndex(CurrentActivityIndex))
    {
        return DailyActivities[CurrentActivityIndex];
    }
    
    return FNPC_RoutineActivity();
}

FNPC_RoutineActivity UNPC_DailyRoutine::GetNextActivity()
{
    int32 NextIndex = (CurrentActivityIndex + 1) % DailyActivities.Num();
    
    if (DailyActivities.IsValidIndex(NextIndex))
    {
        return DailyActivities[NextIndex];
    }
    
    return FNPC_RoutineActivity();
}

void UNPC_DailyRoutine::InterruptCurrentActivity(ENPC_ActivityType NewActivity, float Duration)
{
    if (CanInterruptCurrentActivity())
    {
        FNPC_RoutineActivity InterruptActivity;
        InterruptActivity.ActivityType = NewActivity;
        InterruptActivity.StartHour = GetCurrentGameHour();
        InterruptActivity.Duration = Duration;
        InterruptActivity.Priority = 5.0f; // High priority for interruptions
        InterruptActivity.bCanBeInterrupted = false;
        
        CurrentActivity = InterruptActivity;
    }
}

bool UNPC_DailyRoutine::CanInterruptCurrentActivity()
{
    return CurrentActivity.bCanBeInterrupted;
}

void UNPC_DailyRoutine::ResetRoutine()
{
    CurrentActivityIndex = -1;
    UpdateCurrentActivity();
}

float UNPC_DailyRoutine::GetCurrentGameHour()
{
    if (bUseGameTimeOfDay)
    {
        // Try to get time from game state or world
        UWorld* World = GetWorld();
        if (World)
        {
            float GameTime = World->GetTimeSeconds() * TimeScale;
            float HoursInDay = 24.0f;
            return FMath::Fmod(GameTime / 3600.0f, HoursInDay);
        }
    }
    
    // Fallback to real time
    FDateTime Now = FDateTime::Now();
    return Now.GetHour() + (Now.GetMinute() / 60.0f);
}

void UNPC_DailyRoutine::UpdateCurrentActivity()
{
    if (DailyActivities.Num() == 0)
    {
        return;
    }
    
    float CurrentHour = GetCurrentGameHour();
    int32 NewActivityIndex = FindActivityForTime(CurrentHour);
    
    if (NewActivityIndex != CurrentActivityIndex)
    {
        CurrentActivityIndex = NewActivityIndex;
        if (DailyActivities.IsValidIndex(CurrentActivityIndex))
        {
            CurrentActivity = DailyActivities[CurrentActivityIndex];
        }
    }
}

int32 UNPC_DailyRoutine::FindActivityForTime(float GameHour)
{
    int32 BestActivityIndex = 0;
    
    for (int32 i = 0; i < DailyActivities.Num(); i++)
    {
        const FNPC_RoutineActivity& Activity = DailyActivities[i];
        float EndHour = FMath::Fmod(Activity.StartHour + Activity.Duration, 24.0f);
        
        // Handle activities that cross midnight
        if (Activity.StartHour <= EndHour)
        {
            if (GameHour >= Activity.StartHour && GameHour < EndHour)
            {
                return i;
            }
        }
        else
        {
            if (GameHour >= Activity.StartHour || GameHour < EndHour)
            {
                return i;
            }
        }
        
        // Keep track of the closest activity
        if (Activity.StartHour <= GameHour)
        {
            BestActivityIndex = i;
        }
    }
    
    return BestActivityIndex;
}