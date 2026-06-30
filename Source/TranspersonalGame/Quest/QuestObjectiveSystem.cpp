// QuestObjectiveSystem.cpp
// Agent #15 — Narrative & Dialogue | PROD_CYCLE_AUTO_20260630_007
// Full implementation of quest objective tracking for dinosaur survival game

#include "QuestObjectiveSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_QuestObjectiveComponent
// ============================================================

UNarr_QuestObjectiveComponent::UNarr_QuestObjectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ActiveQuestID = NAME_None;
    bQuestActive = false;
    CurrentObjectiveIndex = 0;
    TotalObjectivesCompleted = 0;
}

void UNarr_QuestObjectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    // Register with world subsystem if available
    UWorld* World = GetWorld();
    if (World)
    {
        unregisterTimer = false;
        World->GetTimerManager().SetTimer(
            ObjectiveCheckTimer,
            this,
            &UNarr_QuestObjectiveComponent::CheckObjectiveConditions,
            2.0f,
            true
        );
    }
}

void UNarr_QuestObjectiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(ObjectiveCheckTimer);
    }
    Super::EndPlay(EndPlayReason);
}

bool UNarr_QuestObjectiveComponent::StartQuest(FName QuestID, const TArray<FNarr_ObjectiveData>& Objectives)
{
    if (bQuestActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestObjectiveSystem: Cannot start quest %s — quest %s already active"),
            *QuestID.ToString(), *ActiveQuestID.ToString());
        return false;
    }

    if (Objectives.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestObjectiveSystem: Cannot start quest %s — no objectives provided"), *QuestID.ToString());
        return false;
    }

    ActiveQuestID = QuestID;
    ActiveObjectives = Objectives;
    CurrentObjectiveIndex = 0;
    bQuestActive = true;

    // Mark first objective as active
    if (ActiveObjectives.IsValidIndex(0))
    {
        ActiveObjectives[0].Status = ENarr_ObjectiveStatus::Active;
    }

    UE_LOG(LogTemp, Log, TEXT("QuestObjectiveSystem: Quest '%s' started with %d objectives"),
        *QuestID.ToString(), Objectives.Num());

    OnQuestStarted.Broadcast(QuestID);
    return true;
}

bool UNarr_QuestObjectiveComponent::CompleteCurrentObjective()
{
    if (!bQuestActive || !ActiveObjectives.IsValidIndex(CurrentObjectiveIndex))
    {
        return false;
    }

    FNarr_ObjectiveData& Obj = ActiveObjectives[CurrentObjectiveIndex];
    Obj.Status = ENarr_ObjectiveStatus::Completed;
    Obj.CurrentCount = Obj.RequiredCount;
    TotalObjectivesCompleted++;

    UE_LOG(LogTemp, Log, TEXT("QuestObjectiveSystem: Objective '%s' completed (%d/%d)"),
        *Obj.ObjectiveID.ToString(), CurrentObjectiveIndex + 1, ActiveObjectives.Num());

    OnObjectiveCompleted.Broadcast(ActiveQuestID, Obj.ObjectiveID);

    // Advance to next objective
    int32 NextIndex = CurrentObjectiveIndex + 1;
    if (ActiveObjectives.IsValidIndex(NextIndex))
    {
        CurrentObjectiveIndex = NextIndex;
        ActiveObjectives[NextIndex].Status = ENarr_ObjectiveStatus::Active;
        UE_LOG(LogTemp, Log, TEXT("QuestObjectiveSystem: Advancing to objective '%s'"),
            *ActiveObjectives[NextIndex].ObjectiveID.ToString());
    }
    else
    {
        // All objectives done — complete the quest
        CompleteQuest();
    }

    return true;
}

void UNarr_QuestObjectiveComponent::CompleteQuest()
{
    if (!bQuestActive) return;

    FName CompletedQuestID = ActiveQuestID;
    bQuestActive = false;
    ActiveQuestID = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("QuestObjectiveSystem: Quest '%s' COMPLETED — total objectives: %d"),
        *CompletedQuestID.ToString(), TotalObjectivesCompleted);

    OnQuestCompleted.Broadcast(CompletedQuestID);
}

bool UNarr_QuestObjectiveComponent::FailQuest(FName Reason)
{
    if (!bQuestActive) return false;

    FName FailedQuestID = ActiveQuestID;
    bQuestActive = false;
    ActiveQuestID = NAME_None;

    // Mark current objective as failed
    if (ActiveObjectives.IsValidIndex(CurrentObjectiveIndex))
    {
        ActiveObjectives[CurrentObjectiveIndex].Status = ENarr_ObjectiveStatus::Failed;
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestObjectiveSystem: Quest '%s' FAILED — reason: %s"),
        *FailedQuestID.ToString(), *Reason.ToString());

    OnQuestFailed.Broadcast(FailedQuestID, Reason);
    return true;
}

void UNarr_QuestObjectiveComponent::UpdateObjectiveCount(FName ObjectiveID, int32 Delta)
{
    for (FNarr_ObjectiveData& Obj : ActiveObjectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && Obj.Status == ENarr_ObjectiveStatus::Active)
        {
            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + Delta, 0, Obj.RequiredCount);

            UE_LOG(LogTemp, Log, TEXT("QuestObjectiveSystem: Objective '%s' progress: %d/%d"),
                *ObjectiveID.ToString(), Obj.CurrentCount, Obj.RequiredCount);

            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                CompleteCurrentObjective();
            }
            return;
        }
    }
}

FNarr_ObjectiveData UNarr_QuestObjectiveComponent::GetCurrentObjective() const
{
    if (ActiveObjectives.IsValidIndex(CurrentObjectiveIndex))
    {
        return ActiveObjectives[CurrentObjectiveIndex];
    }
    return FNarr_ObjectiveData();
}

TArray<FNarr_ObjectiveData> UNarr_QuestObjectiveComponent::GetAllObjectives() const
{
    return ActiveObjectives;
}

void UNarr_QuestObjectiveComponent::CheckObjectiveConditions()
{
    // Periodic check — subclasses or Blueprint can override via event
    if (!bQuestActive) return;

    if (ActiveObjectives.IsValidIndex(CurrentObjectiveIndex))
    {
        const FNarr_ObjectiveData& Obj = ActiveObjectives[CurrentObjectiveIndex];
        if (Obj.Status == ENarr_ObjectiveStatus::Active)
        {
            // Broadcast for Blueprint polling
            OnObjectiveProgressCheck.Broadcast(ActiveQuestID, Obj.ObjectiveID, Obj.CurrentCount, Obj.RequiredCount);
        }
    }
}

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bDialogueTriggered = false;
    bOneShot = true;
    TriggerRadius = 300.0f;
    SpeakerName = FName("Unknown");
    DialogueLineIndex = 0;

    // Root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    SetRootComponent(RootSceneComponent);

    // Trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootSceneComponent);
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTriggerActor::OnTriggerOverlap);
    TriggerSphere->SetSphereRadius(TriggerRadius);
}

void ANarr_DialogueTriggerActor::OnTriggerOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    if (bOneShot && bDialogueTriggered) return;

    // Only trigger for player pawn
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    APawn* PlayerPawn = PC->GetPawn();
    if (OtherActor != PlayerPawn) return;

    bDialogueTriggered = true;

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Player entered trigger zone — speaker: %s, line index: %d"),
        *SpeakerName.ToString(), DialogueLineIndex);

    OnDialogueTriggered.Broadcast(SpeakerName, DialogueLineIndex, DialogueLines.IsValidIndex(DialogueLineIndex)
        ? DialogueLines[DialogueLineIndex]
        : FText::FromString(TEXT("")));
}

void ANarr_DialogueTriggerActor::ResetTrigger()
{
    bDialogueTriggered = false;
    DialogueLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Reset — speaker: %s"), *SpeakerName.ToString());
}
