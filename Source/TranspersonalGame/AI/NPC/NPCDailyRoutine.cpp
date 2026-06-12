#include "NPCDailyRoutine.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_DailyRoutineComponent::UNPC_DailyRoutineComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    TimeScale = 1.0f;
    CurrentGameTime = 6.0f;
}

void UNPC_DailyRoutineComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize daily activities from data table if provided
    if (RoutineDataTable)
    {
        TArray<FNPC_RoutineActivity*> AllActivities;
        RoutineDataTable->GetAllRows<FNPC_RoutineActivity>(TEXT("NPCDailyRoutine"), AllActivities);
        
        for (FNPC_RoutineActivity* Activity : AllActivities)
        {
            if (Activity)
            {
                DailyActivities.Add(*Activity);
            }
        }
    }
    
    // Sort activities by start time
    DailyActivities.Sort([](const FNPC_RoutineActivity& A, const FNPC_RoutineActivity& B) {
        return A.StartTime < B.StartTime;
    });
    
    UpdateCurrentActivity();
}

void UNPC_DailyRoutineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update game time
    CurrentGameTime += DeltaTime * TimeScale / 3600.0f; // Convert seconds to hours
    
    // Wrap around 24 hours
    if (CurrentGameTime >= 24.0f)
    {
        CurrentGameTime -= 24.0f;
    }
    
    UpdateCurrentActivity();
    ProcessSocialInteractions();
}

FNPC_RoutineActivity UNPC_DailyRoutineComponent::GetCurrentActivity()
{
    return CurrentActivity;
}

void UNPC_DailyRoutineComponent::SetCurrentActivity(const FNPC_RoutineActivity& NewActivity)
{
    CurrentActivity = NewActivity;
}

void UNPC_DailyRoutineComponent::UpdateSocialRelation(AActor* OtherNPC, ENPC_RelationType NewRelation, float Strength)
{
    if (!OtherNPC) return;
    
    // Find existing relation or create new one
    FNPC_SocialRelation* ExistingRelation = SocialRelations.FindByPredicate([OtherNPC](const FNPC_SocialRelation& Relation) {
        return Relation.RelatedNPC == OtherNPC;
    });
    
    if (ExistingRelation)
    {
        ExistingRelation->RelationType = NewRelation;
        ExistingRelation->RelationStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
        ExistingRelation->LastInteractionTime = CurrentGameTime;
    }
    else
    {
        FNPC_SocialRelation NewRelation_Struct;
        NewRelation_Struct.RelatedNPC = OtherNPC;
        NewRelation_Struct.RelationType = NewRelation;
        NewRelation_Struct.RelationStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
        NewRelation_Struct.LastInteractionTime = CurrentGameTime;
        SocialRelations.Add(NewRelation_Struct);
    }
}

FNPC_SocialRelation UNPC_DailyRoutineComponent::GetSocialRelation(AActor* OtherNPC)
{
    FNPC_SocialRelation DefaultRelation;
    if (!OtherNPC) return DefaultRelation;
    
    FNPC_SocialRelation* ExistingRelation = SocialRelations.FindByPredicate([OtherNPC](const FNPC_SocialRelation& Relation) {
        return Relation.RelatedNPC == OtherNPC;
    });
    
    return ExistingRelation ? *ExistingRelation : DefaultRelation;
}

void UNPC_DailyRoutineComponent::UpdateCurrentActivity()
{
    if (DailyActivities.Num() == 0) return;
    
    // Find the current activity based on time
    FNPC_RoutineActivity* BestActivity = nullptr;
    float BestPriority = 0.0f;
    
    for (FNPC_RoutineActivity& Activity : DailyActivities)
    {
        float EndTime = Activity.StartTime + Activity.Duration;
        
        // Handle activities that span midnight
        bool IsActiveNow = false;
        if (EndTime <= 24.0f)
        {
            IsActiveNow = (CurrentGameTime >= Activity.StartTime && CurrentGameTime < EndTime);
        }
        else
        {
            // Activity spans midnight
            float WrappedEndTime = EndTime - 24.0f;
            IsActiveNow = (CurrentGameTime >= Activity.StartTime || CurrentGameTime < WrappedEndTime);
        }
        
        if (IsActiveNow && Activity.Priority > BestPriority)
        {
            BestActivity = &Activity;
            BestPriority = Activity.Priority;
        }
    }
    
    if (BestActivity && (CurrentActivity.ActivityType != BestActivity->ActivityType || 
        !CurrentActivity.ActivityLocation.Equals(BestActivity->ActivityLocation, 100.0f)))
    {
        CurrentActivity = *BestActivity;
    }
}

void UNPC_DailyRoutineComponent::ProcessSocialInteractions()
{
    // Decay social relations over time
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        float TimeSinceInteraction = CurrentGameTime - Relation.LastInteractionTime;
        if (TimeSinceInteraction > 24.0f) // More than a day
        {
            Relation.RelationStrength *= 0.99f; // Slow decay
        }
    }
    
    // Remove very weak relations
    SocialRelations.RemoveAll([](const FNPC_SocialRelation& Relation) {
        return Relation.RelationStrength < 0.1f;
    });
}