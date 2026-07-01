#include "QuestManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    TutorialWaterSourceLocation = FVector(8000.0f, 0.0f, 50.0f);
    ProximityCheckInterval = 0.5f;
    TimeSinceLastProximityCheck = 0.0f;
    CurrentObjectiveText = TEXT("Survive the prehistoric world.");
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    // Auto-start tutorial quest on begin play
    StartTutorialQuest_FindWater();
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastProximityCheck += DeltaTime;
    if (TimeSinceLastProximityCheck >= ProximityCheckInterval)
    {
        TimeSinceLastProximityCheck = 0.0f;
        // Check player location for proximity
        if (GetWorld())
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC && PC->GetPawn())
            {
                CheckQuestProximity(PC->GetPawn()->GetActorLocation());
            }
        }
    }
}

void AQuestManager::StartTutorialQuest_FindWater()
{
    FPerf_QuestData TutorialQuest;
    TutorialQuest.QuestID = TEXT("TUTORIAL_FIND_WATER");
    TutorialQuest.QuestTitle = TEXT("Find Water");
    TutorialQuest.QuestDescription = TEXT("You are thirsty. Find a water source to survive. Follow the sound of water.");
    TutorialQuest.QuestType = EPerf_QuestType::Tutorial;
    TutorialQuest.QuestState = EPerf_QuestState::Active;

    // Objective 1: Reach the water source
    FPerf_QuestObjective WaterObjective;
    WaterObjective.ObjectiveText = TEXT("Find a water source (follow the blue light to the north)");
    WaterObjective.bCompleted = false;
    WaterObjective.TargetLocation = TutorialWaterSourceLocation;
    WaterObjective.ProximityRadius = 600.0f;
    TutorialQuest.Objectives.Add(WaterObjective);

    // Objective 2: Drink water (auto-complete after reaching)
    FPerf_QuestObjective DrinkObjective;
    DrinkObjective.ObjectiveText = TEXT("Drink from the water source");
    DrinkObjective.bCompleted = false;
    DrinkObjective.TargetLocation = TutorialWaterSourceLocation;
    DrinkObjective.ProximityRadius = 300.0f;
    TutorialQuest.Objectives.Add(DrinkObjective);

    ActiveQuests.Add(TutorialQuest);
    UpdateCurrentObjectiveText();

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Tutorial quest 'Find Water' started. Water at (%.0f, %.0f, %.0f)"),
        TutorialWaterSourceLocation.X, TutorialWaterSourceLocation.Y, TutorialWaterSourceLocation.Z);
}

bool AQuestManager::ActivateQuest(const FString& QuestID)
{
    for (FPerf_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EPerf_QuestState::Inactive)
        {
            Quest.QuestState = EPerf_QuestState::Active;
            UpdateCurrentObjectiveText();
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' activated"), *QuestID);
            return true;
        }
    }
    return false;
}

bool AQuestManager::CompleteObjective(const FString& QuestID, int32 ObjectiveIndex)
{
    for (FPerf_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EPerf_QuestState::Active)
        {
            if (Quest.Objectives.IsValidIndex(ObjectiveIndex))
            {
                Quest.Objectives[ObjectiveIndex].bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %d completed for quest '%s'"), ObjectiveIndex, *QuestID);

                // Check if all objectives complete
                bool bAllComplete = true;
                for (const FPerf_QuestObjective& Obj : Quest.Objectives)
                {
                    if (!Obj.bCompleted)
                    {
                        bAllComplete = false;
                        break;
                    }
                }

                if (bAllComplete)
                {
                    Quest.QuestState = EPerf_QuestState::Completed;
                    CompletedQuests.Add(Quest);
                    ActiveQuests.Remove(Quest);
                    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' COMPLETED!"), *QuestID);
                }

                UpdateCurrentObjectiveText();
                return true;
            }
        }
    }
    return false;
}

void AQuestManager::CheckQuestProximity(FVector PlayerLocation)
{
    for (FPerf_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestState != EPerf_QuestState::Active) continue;

        for (int32 i = 0; i < Quest.Objectives.Num(); i++)
        {
            FPerf_QuestObjective& Obj = Quest.Objectives[i];
            if (Obj.bCompleted) continue;

            float Distance = FVector::Dist(PlayerLocation, Obj.TargetLocation);
            if (Distance <= Obj.ProximityRadius)
            {
                CompleteObjective(Quest.QuestID, i);
                break; // Complete one at a time
            }
        }
    }
}

int32 AQuestManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const FPerf_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestState == EPerf_QuestState::Active) Count++;
    }
    return Count;
}

void AQuestManager::UpdateCurrentObjectiveText()
{
    for (const FPerf_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestState == EPerf_QuestState::Active)
        {
            for (const FPerf_QuestObjective& Obj : Quest.Objectives)
            {
                if (!Obj.bCompleted)
                {
                    CurrentObjectiveText = FString::Printf(TEXT("[%s] %s"), *Quest.QuestTitle, *Obj.ObjectiveText);
                    return;
                }
            }
        }
    }
    CurrentObjectiveText = TEXT("Explore the prehistoric world. Stay alive.");
}
