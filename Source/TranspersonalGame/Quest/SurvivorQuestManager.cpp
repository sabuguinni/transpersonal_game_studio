// SurvivorQuestManager.cpp
// Agent #14 — Quest & Mission Designer
// Implements: Stampede Survival quest with 4 objectives, NPC dialogue trigger, reward system

#include "SurvivorQuestManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_NPCInteractionTrigger
// ─────────────────────────────────────────────────────────────────────────────

AQuest_NPCInteractionTrigger::AQuest_NPCInteractionTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(200.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    NPCName = FText::FromString(TEXT("Elder Hunter"));
    DialogueLine = FText::FromString(TEXT("The herd moves at dusk. Strike before they scatter."));
    LinkedQuestName = FName("StampedeEscape");
    bHasTriggered = false;
}

void AQuest_NPCInteractionTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_NPCInteractionTrigger::OnPlayerEnter);
}

void AQuest_NPCInteractionTrigger::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHasTriggered) return;
    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    bHasTriggered = true;
    UE_LOG(LogTemp, Log, TEXT("[Quest] NPC '%s' triggered: %s"), *NPCName.ToString(), *DialogueLine.ToString());
    OnNPCInteraction.Broadcast(NPCName, DialogueLine);

    // Activate linked quest
    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_SurvivorQuestManager::StaticClass(), Managers);
    if (Managers.Num() > 0)
    {
        AQuest_SurvivorQuestManager* QM = Cast<AQuest_SurvivorQuestManager>(Managers[0]);
        if (QM) QM->ActivateQuest(LinkedQuestName);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_SurvivorQuestManager
// ─────────────────────────────────────────────────────────────────────────────

AQuest_SurvivorQuestManager::AQuest_SurvivorQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = EQuest_SurvivorState::Inactive;
    ActiveQuestName = NAME_None;
    StampedeTimeLimit = 120.0f;
    StampedeElapsed = 0.0f;
    bStampedeTimerRunning = false;
    RegisterQuests();
}

void AQuest_SurvivorQuestManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterQuests();
    UE_LOG(LogTemp, Log, TEXT("[Quest] SurvivorQuestManager initialized with %d quests"), QuestDefinitions.Num());
}

void AQuest_SurvivorQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bStampedeTimerRunning && CurrentState == EQuest_SurvivorState::StampedeActive)
    {
        StampedeElapsed += DeltaTime;
        if (StampedeElapsed >= StampedeTimeLimit)
        {
            // Time expired — quest failed
            FailQuest(ActiveQuestName);
        }
    }
}

void AQuest_SurvivorQuestManager::RegisterQuests()
{
    QuestDefinitions.Empty();

    // ── Quest: Stampede Escape ────────────────────────────────────────────────
    FQuest_SurvivorDefinition StampedeQuest;
    StampedeQuest.QuestName = FName("StampedeEscape");
    StampedeQuest.DisplayTitle = FText::FromString(TEXT("Stampede Escape"));
    StampedeQuest.Description = FText::FromString(
        TEXT("A Triceratops herd is about to stampede through the valley. "
             "Warn the camp, find high ground, and survive until the herd passes."));
    StampedeQuest.RewardDescription = FText::FromString(
        TEXT("Triceratops hide x2, Bone fragment x3, +50 XP"));
    StampedeQuest.bIsMainQuest = false;
    StampedeQuest.bRepeatable = false;

    // Objective 1: Talk to the Elder
    FQuest_SurvivorObjective Obj1;
    Obj1.ObjectiveName = FName("TalkToElder");
    Obj1.Description = FText::FromString(TEXT("Talk to the Elder Hunter near the river crossing"));
    Obj1.Status = EQuest_ObjectiveStatus::Active;
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    StampedeQuest.Objectives.Add(Obj1);

    // Objective 2: Warn the camp (reach marker)
    FQuest_SurvivorObjective Obj2;
    Obj2.ObjectiveName = FName("WarnCamp");
    Obj2.Description = FText::FromString(TEXT("Run to the camp and warn the hunters (200m north)"));
    Obj2.Status = EQuest_ObjectiveStatus::Pending;
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    StampedeQuest.Objectives.Add(Obj2);

    // Objective 3: Reach high ground before stampede
    FQuest_SurvivorObjective Obj3;
    Obj3.ObjectiveName = FName("ReachHighGround");
    Obj3.Description = FText::FromString(TEXT("Climb to the rocky ridge before the herd arrives (2 min)"));
    Obj3.Status = EQuest_ObjectiveStatus::Pending;
    Obj3.RequiredCount = 1;
    Obj3.CurrentCount = 0;
    StampedeQuest.Objectives.Add(Obj3);

    // Objective 4: Survive the stampede
    FQuest_SurvivorObjective Obj4;
    Obj4.ObjectiveName = FName("SurviveStampede");
    Obj4.Description = FText::FromString(TEXT("Stay on high ground until the herd passes"));
    Obj4.Status = EQuest_ObjectiveStatus::Pending;
    Obj4.RequiredCount = 1;
    Obj4.CurrentCount = 0;
    StampedeQuest.Objectives.Add(Obj4);

    QuestDefinitions.Add(StampedeQuest);

    // ── Quest: Missing Hunters ────────────────────────────────────────────────
    FQuest_SurvivorDefinition RescueQuest;
    RescueQuest.QuestName = FName("MissingHunters");
    RescueQuest.DisplayTitle = FText::FromString(TEXT("Missing Hunters"));
    RescueQuest.Description = FText::FromString(
        TEXT("Three hunters went north to gather reeds and never returned. "
             "Find them before nightfall — the cold and predators won't wait."));
    RescueQuest.RewardDescription = FText::FromString(
        TEXT("Reed bundle x5, Dried meat x2, +75 XP"));
    RescueQuest.bIsMainQuest = false;
    RescueQuest.bRepeatable = false;

    FQuest_SurvivorObjective RObj1;
    RObj1.ObjectiveName = FName("FindFirstHunter");
    RObj1.Description = FText::FromString(TEXT("Find the first missing hunter near the flooded river bank"));
    RObj1.Status = EQuest_ObjectiveStatus::Pending;
    RObj1.RequiredCount = 1;
    RObj1.CurrentCount = 0;
    RescueQuest.Objectives.Add(RObj1);

    FQuest_SurvivorObjective RObj2;
    RObj2.ObjectiveName = FName("FindSecondHunter");
    RObj2.Description = FText::FromString(TEXT("Search the reed beds for the second hunter"));
    RObj2.Status = EQuest_ObjectiveStatus::Pending;
    RObj2.RequiredCount = 1;
    RObj2.CurrentCount = 0;
    RescueQuest.Objectives.Add(RObj2);

    FQuest_SurvivorObjective RObj3;
    RObj3.ObjectiveName = FName("FindThirdHunter");
    RObj3.Description = FText::FromString(TEXT("The third hunter was dragged off — follow the tracks"));
    RObj3.Status = EQuest_ObjectiveStatus::Pending;
    RObj3.RequiredCount = 1;
    RObj3.CurrentCount = 0;
    RescueQuest.Objectives.Add(RObj3);

    FQuest_SurvivorObjective RObj4;
    RObj4.ObjectiveName = FName("EscortBack");
    RObj4.Description = FText::FromString(TEXT("Escort all survivors back to camp before nightfall"));
    RObj4.Status = EQuest_ObjectiveStatus::Pending;
    RObj4.RequiredCount = 3;
    RObj4.CurrentCount = 0;
    RescueQuest.Objectives.Add(RObj4);

    QuestDefinitions.Add(RescueQuest);
}

void AQuest_SurvivorQuestManager::ActivateQuest(FName QuestName)
{
    if (ActiveQuestName == QuestName) return;

    FQuest_SurvivorDefinition* Quest = FindQuest(QuestName);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Quest] ActivateQuest: Quest '%s' not found"), *QuestName.ToString());
        return;
    }

    ActiveQuestName = QuestName;
    CurrentState = EQuest_SurvivorState::WatchingHerd;
    UE_LOG(LogTemp, Log, TEXT("[Quest] Activated: %s"), *Quest->DisplayTitle.ToString());
    OnQuestActivated.Broadcast(QuestName);
}

bool AQuest_SurvivorQuestManager::AdvanceObjective(FName QuestName, FName ObjectiveName, int32 Count)
{
    FQuest_SurvivorDefinition* Quest = FindQuest(QuestName);
    if (!Quest) return false;

    for (FQuest_SurvivorObjective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveName == ObjectiveName && Obj.Status == EQuest_ObjectiveStatus::Active)
        {
            Obj.CurrentCount += Count;
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.Status = EQuest_ObjectiveStatus::Completed;
                UE_LOG(LogTemp, Log, TEXT("[Quest] Objective completed: %s"), *ObjectiveName.ToString());
                OnObjectiveCompleted.Broadcast(QuestName, ObjectiveName);
                ActivateNextObjective(Quest);
            }
            return true;
        }
    }
    return false;
}

void AQuest_SurvivorQuestManager::ActivateNextObjective(FQuest_SurvivorDefinition* Quest)
{
    if (!Quest) return;
    for (FQuest_SurvivorObjective& Obj : Quest->Objectives)
    {
        if (Obj.Status == EQuest_ObjectiveStatus::Pending)
        {
            Obj.Status = EQuest_ObjectiveStatus::Active;
            UE_LOG(LogTemp, Log, TEXT("[Quest] Next objective activated: %s"), *Obj.ObjectiveName.ToString());

            // Transition state machine
            if (Obj.ObjectiveName == FName("ReachHighGround"))
            {
                CurrentState = EQuest_SurvivorState::EscapeWindow;
            }
            else if (Obj.ObjectiveName == FName("SurviveStampede"))
            {
                CurrentState = EQuest_SurvivorState::StampedeActive;
                bStampedeTimerRunning = true;
                StampedeElapsed = 0.0f;
            }
            return;
        }
    }

    // All objectives done — complete quest
    CompleteQuest(Quest->QuestName);
}

void AQuest_SurvivorQuestManager::CompleteQuest(FName QuestName)
{
    CurrentState = EQuest_SurvivorState::Completed;
    bStampedeTimerRunning = false;
    UE_LOG(LogTemp, Log, TEXT("[Quest] Quest COMPLETED: %s"), *QuestName.ToString());
    OnQuestCompleted.Broadcast(QuestName, true);
}

void AQuest_SurvivorQuestManager::FailQuest(FName QuestName)
{
    CurrentState = EQuest_SurvivorState::Failed;
    bStampedeTimerRunning = false;
    UE_LOG(LogTemp, Warning, TEXT("[Quest] Quest FAILED: %s"), *QuestName.ToString());
    OnQuestCompleted.Broadcast(QuestName, false);
}

FQuest_SurvivorDefinition* AQuest_SurvivorQuestManager::FindQuest(FName QuestName)
{
    for (FQuest_SurvivorDefinition& Q : QuestDefinitions)
    {
        if (Q.QuestName == QuestName) return &Q;
    }
    return nullptr;
}

EQuest_SurvivorState AQuest_SurvivorQuestManager::GetCurrentState() const
{
    return CurrentState;
}

TArray<FQuest_SurvivorObjective> AQuest_SurvivorQuestManager::GetActiveObjectives(FName QuestName) const
{
    TArray<FQuest_SurvivorObjective> Result;
    for (const FQuest_SurvivorDefinition& Q : QuestDefinitions)
    {
        if (Q.QuestName == QuestName)
        {
            for (const FQuest_SurvivorObjective& Obj : Q.Objectives)
            {
                if (Obj.Status == EQuest_ObjectiveStatus::Active)
                    Result.Add(Obj);
            }
        }
    }
    return Result;
}
