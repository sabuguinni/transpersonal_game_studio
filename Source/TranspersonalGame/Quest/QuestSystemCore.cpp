#include "QuestSystemCore.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// AQuestTriggerVolume Implementation
// ============================================================

AQuestTriggerVolume::AQuestTriggerVolume()
    : bStartsQuest(true)
    , bCompletesObjective(true)
    , bOneTimeUse(true)
    , bHasTriggered(false)
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetBoxExtent(FVector(150.f, 150.f, 100.f));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerBox;
}

void AQuestTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AQuestTriggerVolume::OnOverlapBegin);
}

void AQuestTriggerVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    if (bOneTimeUse && bHasTriggered) return;

    // Only respond to player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bHasTriggered = true;

    // Find QuestManagerComponent on GameState or GameMode
    UWorld* World = GetWorld();
    if (!World) return;

    AGameStateBase* GS = World->GetGameState();
    if (!GS) return;

    UQuestManagerComponent* QuestMgr = GS->FindComponentByClass<UQuestManagerComponent>();
    if (!QuestMgr) return;

    if (bStartsQuest && !QuestID.IsEmpty())
    {
        QuestMgr->StartQuest(QuestID);
        UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Started quest '%s'"), *QuestID);
    }

    if (bCompletesObjective && !QuestID.IsEmpty() && !ObjectiveID.IsEmpty())
    {
        QuestMgr->AdvanceObjective(QuestID, ObjectiveID, 1);
        UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Advanced objective '%s' in quest '%s'"), *ObjectiveID, *QuestID);
    }
}

// ============================================================
// UQuestManagerComponent Implementation
// ============================================================

UQuestManagerComponent::UQuestManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuestManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void UQuestManagerComponent::InitializeDefaultQuests()
{
    QuestDefinitions.Empty();
    BuildRaptorDenQuest();
    BuildFirstHuntQuest();
    BuildSurvivalNightQuest();
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized %d quest definitions"), QuestDefinitions.Num());
}

void UQuestManagerComponent::BuildRaptorDenQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = TEXT("QUEST_RAPTOR_DEN");
    Quest.QuestName = TEXT("The Raptor Den");
    Quest.Description = TEXT("A pack of raptors has been spotted near the eastern valley. Track them to their den and destroy the nest before the pack grows.");
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsMainQuest = false;

    // Objective 1: Reach the eastern valley
    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_REACH_VALLEY");
    Obj1.Description = TEXT("Track the raptor pack to the eastern valley");
    Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bIsCompleted = false;
    Quest.Objectives.Add(Obj1);

    // Objective 2: Find the nest
    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_FIND_NEST");
    Obj2.Description = TEXT("Locate the raptor nest site");
    Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bIsCompleted = false;
    Quest.Objectives.Add(Obj2);

    // Objective 3: Destroy eggs
    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("OBJ_DESTROY_EGGS");
    Obj3.Description = TEXT("Destroy the raptor eggs (0/4)");
    Obj3.ObjectiveType = EQuest_ObjectiveType::Destroy;
    Obj3.RequiredCount = 4;
    Obj3.CurrentCount = 0;
    Obj3.bIsCompleted = false;
    Quest.Objectives.Add(Obj3);

    // Objective 4: Escape
    FQuest_Objective Obj4;
    Obj4.ObjectiveID = TEXT("OBJ_ESCAPE");
    Obj4.Description = TEXT("Escape from the raptor territory");
    Obj4.ObjectiveType = EQuest_ObjectiveType::Reach;
    Obj4.RequiredCount = 1;
    Obj4.CurrentCount = 0;
    Obj4.bIsCompleted = false;
    Quest.Objectives.Add(Obj4);

    // Reward
    Quest.Reward.ExperiencePoints = 250;
    Quest.Reward.ItemRewards.Add(TEXT("RaptorClaw_x3"));
    Quest.Reward.ItemRewards.Add(TEXT("RaptorHide_x2"));
    Quest.Reward.UnlockedArea = TEXT("EasternValley");

    QuestDefinitions.Add(Quest);
}

void UQuestManagerComponent::BuildFirstHuntQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = TEXT("QUEST_FIRST_HUNT");
    Quest.QuestName = TEXT("First Blood");
    Quest.Description = TEXT("The tribe needs food. Hunt a herbivore dinosaur and bring back meat to the camp.");
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsMainQuest = true;

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_FIND_PREY");
    Obj1.Description = TEXT("Locate a herbivore dinosaur");
    Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bIsCompleted = false;
    Quest.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_KILL_PREY");
    Obj2.Description = TEXT("Kill the dinosaur");
    Obj2.ObjectiveType = EQuest_ObjectiveType::Kill;
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bIsCompleted = false;
    Quest.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("OBJ_COLLECT_MEAT");
    Obj3.Description = TEXT("Collect meat from the carcass");
    Obj3.ObjectiveType = EQuest_ObjectiveType::Collect;
    Obj3.RequiredCount = 3;
    Obj3.CurrentCount = 0;
    Obj3.bIsCompleted = false;
    Quest.Objectives.Add(Obj3);

    FQuest_Objective Obj4;
    Obj4.ObjectiveID = TEXT("OBJ_RETURN_CAMP");
    Obj4.Description = TEXT("Return to camp with the meat");
    Obj4.ObjectiveType = EQuest_ObjectiveType::Reach;
    Obj4.RequiredCount = 1;
    Obj4.CurrentCount = 0;
    Obj4.bIsCompleted = false;
    Quest.Objectives.Add(Obj4);

    Quest.Reward.ExperiencePoints = 150;
    Quest.Reward.ItemRewards.Add(TEXT("DinoMeat_x3"));
    Quest.Reward.ItemRewards.Add(TEXT("Bone_x2"));

    QuestDefinitions.Add(Quest);
}

void UQuestManagerComponent::BuildSurvivalNightQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = TEXT("QUEST_SURVIVE_NIGHT");
    Quest.QuestName = TEXT("Survive the Dark");
    Quest.Description = TEXT("Predators are most active at night. Build a fire, fortify your position, and survive until dawn.");
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsMainQuest = false;

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_BUILD_FIRE");
    Obj1.Description = TEXT("Build a campfire");
    Obj1.ObjectiveType = EQuest_ObjectiveType::Collect;
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bIsCompleted = false;
    Quest.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_SURVIVE_NIGHT");
    Obj2.Description = TEXT("Survive until dawn (5 minutes)");
    Obj2.ObjectiveType = EQuest_ObjectiveType::Survive;
    Obj2.RequiredCount = 300; // seconds
    Obj2.CurrentCount = 0;
    Obj2.bIsCompleted = false;
    Quest.Objectives.Add(Obj2);

    Quest.Reward.ExperiencePoints = 200;
    Quest.Reward.ItemRewards.Add(TEXT("FlintStone_x2"));
    Quest.Reward.UnlockedArea = TEXT("NorthernForest");

    QuestDefinitions.Add(Quest);
}

bool UQuestManagerComponent::StartQuest(const FString& QuestID)
{
    // Check if already active
    if (IsQuestActive(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' is already active"), *QuestID);
        return false;
    }

    // Find definition
    for (const FQuest_Data& Def : QuestDefinitions)
    {
        if (Def.QuestID == QuestID)
        {
            FQuest_Data NewQuest = Def;
            NewQuest.Status = EQuest_Status::Active;
            ActiveQuests.Add(NewQuest);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' started"), *QuestID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest definition '%s' not found"), *QuestID);
    return false;
}

bool UQuestManagerComponent::AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count)
{
    FQuest_Data* Quest = FindActiveQuest(QuestID);
    if (!Quest) return false;

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' completed in quest '%s'"), *ObjectiveID, *QuestID);
            }
            CheckQuestCompletion(*Quest);
            return true;
        }
    }
    return false;
}

bool UQuestManagerComponent::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindActiveQuest(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Completed;
    CompletedQuests.Add(*Quest);

    // Remove from active
    ActiveQuests.RemoveAll([&QuestID](const FQuest_Data& Q) {
        return Q.QuestID == QuestID;
    });

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' COMPLETED. Reward: %d XP"), *QuestID, Quest->Reward.ExperiencePoints);
    return true;
}

bool UQuestManagerComponent::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindActiveQuest(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Failed;
    ActiveQuests.RemoveAll([&QuestID](const FQuest_Data& Q) {
        return Q.QuestID == QuestID;
    });

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' FAILED"), *QuestID);
    return true;
}

FQuest_Data UQuestManagerComponent::GetQuestData(const FString& QuestID) const
{
    for (const FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID) return Q;
    }
    for (const FQuest_Data& Q : CompletedQuests)
    {
        if (Q.QuestID == QuestID) return Q;
    }
    return FQuest_Data();
}

bool UQuestManagerComponent::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID) return true;
    }
    return false;
}

void UQuestManagerComponent::RegisterQuestDefinition(const FQuest_Data& QuestDef)
{
    // Remove existing definition with same ID if present
    QuestDefinitions.RemoveAll([&QuestDef](const FQuest_Data& Q) {
        return Q.QuestID == QuestDef.QuestID;
    });
    QuestDefinitions.Add(QuestDef);
}

FQuest_Data* UQuestManagerComponent::FindActiveQuest(const FString& QuestID)
{
    for (FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID) return &Q;
    }
    return nullptr;
}

void UQuestManagerComponent::CheckQuestCompletion(FQuest_Data& Quest)
{
    bool bAllComplete = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        CompleteQuest(Quest.QuestID);
    }
}
