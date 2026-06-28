#include "QuestSystemCore.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// AQuestTriggerVolume — Implementation
// ============================================================

AQuestTriggerVolume::AQuestTriggerVolume()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuestTriggerVolume::OnSphereBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AQuestTriggerVolume::OnSphereEndOverlap);

    LinkedQuestID = TEXT("");
    LinkedObjectiveID = TEXT("");
    TriggerType = EQuest_TriggerType::ProximityEnter;
    TriggerRadius = 300.0f;
    bSingleUse = true;
    bHasTriggered = false;
}

void AQuestTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->SetSphereRadius(TriggerRadius);
}

void AQuestTriggerVolume::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    // Draw debug sphere in editor for visibility
    DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 16,
        bHasTriggered ? FColor::Green : FColor::Yellow, false, -1.0f, 0, 2.0f);
#endif
}

void AQuestTriggerVolume::ActivateTrigger()
{
    if (bSingleUse && bHasTriggered)
    {
        return;
    }

    bHasTriggered = true;

    // Find QuestManager in world and notify it
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuestManager::StaticClass(), FoundManagers);

    for (AActor* ManagerActor : FoundManagers)
    {
        AQuestManager* Manager = Cast<AQuestManager>(ManagerActor);
        if (Manager && !LinkedQuestID.IsEmpty())
        {
            if (TriggerType == EQuest_TriggerType::ProximityEnter)
            {
                // Start quest if not already active
                EQuest_State State = Manager->GetQuestState(LinkedQuestID);
                if (State == EQuest_State::Available)
                {
                    Manager->StartQuest(LinkedQuestID);
                }
                // Complete objective if quest is active
                else if (State == EQuest_State::Active && !LinkedObjectiveID.IsEmpty())
                {
                    Manager->CompleteObjective(LinkedQuestID, LinkedObjectiveID);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("QuestTriggerVolume activated: Quest=%s Objective=%s"),
        *LinkedQuestID, *LinkedObjectiveID);
}

void AQuestTriggerVolume::ResetTrigger()
{
    bHasTriggered = false;
}

bool AQuestTriggerVolume::IsPlayerInRange() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player)
    {
        return false;
    }
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Dist <= TriggerRadius;
}

void AQuestTriggerVolume::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Only trigger for player character
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) == Character)
    {
        if (TriggerType == EQuest_TriggerType::ProximityEnter)
        {
            ActivateTrigger();
        }
    }
}

void AQuestTriggerVolume::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) == Character)
    {
        if (TriggerType == EQuest_TriggerType::ProximityExit)
        {
            ActivateTrigger();
        }
    }
}

// ============================================================
// AQuestManager — Implementation
// ============================================================

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ActiveQuestID = TEXT("");
    TotalQuestsCompleted = 0;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // -------------------------------------------------------
    // Register built-in quests for the MinPlayableMap
    // -------------------------------------------------------

    // QUEST 1: Stampede Warning — warn the eastern camp
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("Q_StampedeWarning");
        Q.QuestTitle = TEXT("Stampede Warning");
        Q.QuestDescription = TEXT("The Iguanodon herd is moving fast. Warn the eastern camp before the stampede reaches the river crossing.");
        Q.State = EQuest_State::Available;
        Q.bHasTimeLimit = true;
        Q.TimeLimit = 180.0f; // 3 minutes
        Q.RewardDescription = TEXT("Camp shelter unlocked. Tribal elder shares knowledge of safe river crossings.");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_ReachCamp");
        Obj1.Description = TEXT("Reach the tribal camp before the herd blocks the path");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.TargetLocation = FVector(-500.0f, 800.0f, 100.0f);
        Obj1.ProximityRadius = 400.0f;
        Obj1.RequiredCount = 1;

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_WarnElder");
        Obj2.Description = TEXT("Speak to the tribal elder");
        Obj2.ObjectiveType = EQuest_ObjectiveType::WarnNPC;
        Obj2.TargetLocation = FVector(-500.0f, 800.0f, 100.0f);
        Obj2.ProximityRadius = 200.0f;
        Obj2.RequiredCount = 1;

        Q.Objectives.Add(Obj1);
        Q.Objectives.Add(Obj2);
        QuestRegistry.Add(Q);
    }

    // QUEST 2: Observe the Herd Migration
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("Q_HerdObservation");
        Q.QuestTitle = TEXT("Track the Migration");
        Q.QuestDescription = TEXT("Follow the Iguanodon herd along the migration path. Observe their behaviour at each waypoint without disturbing them.");
        Q.State = EQuest_State::Available;
        Q.bHasTimeLimit = false;
        Q.RewardDescription = TEXT("Migration route mapped. New area unlocked: River Delta.");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_Waypoint1");
        Obj1.Description = TEXT("Reach the first observation point near the herd");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ObserveCreature;
        Obj1.TargetLocation = FVector(3000.0f, 1500.0f, 100.0f);
        Obj1.ProximityRadius = 600.0f;
        Obj1.RequiredCount = 1;

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_Waypoint2");
        Obj2.Description = TEXT("Follow the herd to the river crossing");
        Obj2.ObjectiveType = EQuest_ObjectiveType::FollowTrail;
        Obj2.TargetLocation = FVector(4500.0f, 2500.0f, 100.0f);
        Obj2.ProximityRadius = 500.0f;
        Obj2.RequiredCount = 1;

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_Waypoint3");
        Obj3.Description = TEXT("Observe the herd at the valley exit");
        Obj3.ObjectiveType = EQuest_ObjectiveType::ObserveCreature;
        Obj3.TargetLocation = FVector(6000.0f, 3500.0f, 100.0f);
        Obj3.ProximityRadius = 500.0f;
        Obj3.RequiredCount = 1;

        Q.Objectives.Add(Obj1);
        Q.Objectives.Add(Obj2);
        Q.Objectives.Add(Obj3);
        QuestRegistry.Add(Q);
    }

    // QUEST 3: Predator Threat — escape before the predator arrives
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("Q_PredatorEscape");
        Q.QuestTitle = TEXT("Predator at the River");
        Q.QuestDescription = TEXT("A large predator has been spotted following the herd. Help the camp pack supplies and move to higher ground before nightfall.");
        Q.State = EQuest_State::Locked; // Unlocks after Q_StampedeWarning completes
        Q.bHasTimeLimit = true;
        Q.TimeLimit = 240.0f; // 4 minutes
        Q.RewardDescription = TEXT("New camp location established. Crafting bench unlocked.");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_CollectSupplies");
        Obj1.Description = TEXT("Collect 3 supply bundles from the camp");
        Obj1.ObjectiveType = EQuest_ObjectiveType::CollectResource;
        Obj1.TargetLocation = FVector(-500.0f, 800.0f, 100.0f);
        Obj1.ProximityRadius = 300.0f;
        Obj1.RequiredCount = 3;

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_EscapeToHighGround");
        Obj2.Description = TEXT("Reach the high ground before the predator arrives");
        Obj2.ObjectiveType = EQuest_ObjectiveType::EscapeArea;
        Obj2.TargetLocation = FVector(-2000.0f, -1500.0f, 500.0f);
        Obj2.ProximityRadius = 400.0f;
        Obj2.RequiredCount = 1;

        Q.Objectives.Add(Obj1);
        Q.Objectives.Add(Obj2);
        QuestRegistry.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager initialized with %d quests"), QuestRegistry.Num());
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckTimeLimits(DeltaTime);
}

bool AQuestManager::StartQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest not found: %s"), *QuestID);
        return false;
    }

    FQuest_Definition& Quest = QuestRegistry[Idx];
    if (Quest.State != EQuest_State::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not available (state=%d)"), *QuestID, (int32)Quest.State);
        return false;
    }

    Quest.State = EQuest_State::Active;
    ActiveQuestID = QuestID;
    QuestStartTimes.Add(QuestID, GetWorld()->GetTimeSeconds());
    QuestElapsedTimes.Add(QuestID, 0.0f);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest STARTED — %s"), *Quest.QuestTitle);
    return true;
}

bool AQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    int32 QIdx = FindQuestIndex(QuestID);
    if (QIdx == INDEX_NONE)
    {
        return false;
    }

    FQuest_Definition& Quest = QuestRegistry[QIdx];
    if (Quest.State != EQuest_State::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective COMPLETE — %s / %s"), *QuestID, *ObjectiveID);
            }

            // Check if all objectives done
            if (AreAllObjectivesComplete(Quest))
            {
                Quest.State = EQuest_State::Completed;
                TotalQuestsCompleted++;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s"), *Quest.QuestTitle);

                // Unlock dependent quests
                if (QuestID == TEXT("Q_StampedeWarning"))
                {
                    int32 PredIdx = FindQuestIndex(TEXT("Q_PredatorEscape"));
                    if (PredIdx != INDEX_NONE && QuestRegistry[PredIdx].State == EQuest_State::Locked)
                    {
                        QuestRegistry[PredIdx].State = EQuest_State::Available;
                        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest UNLOCKED — Q_PredatorEscape"));
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool AQuestManager::FailQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return false;
    }

    FQuest_Definition& Quest = QuestRegistry[Idx];
    if (Quest.State == EQuest_State::Active)
    {
        Quest.State = EQuest_State::Failed;
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest FAILED — %s"), *Quest.QuestTitle);
        return true;
    }
    return false;
}

EQuest_State AQuestManager::GetQuestState(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return EQuest_State::Locked;
    }
    return QuestRegistry[Idx].State;
}

FQuest_Definition AQuestManager::GetQuestDefinition(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx != INDEX_NONE)
    {
        return QuestRegistry[Idx];
    }
    return FQuest_Definition();
}

void AQuestManager::RegisterQuest(const FQuest_Definition& QuestDef)
{
    // Prevent duplicates
    int32 Existing = FindQuestIndex(QuestDef.QuestID);
    if (Existing != INDEX_NONE)
    {
        QuestRegistry[Existing] = QuestDef;
    }
    else
    {
        QuestRegistry.Add(QuestDef);
    }
}

TArray<FString> AQuestManager::GetActiveQuestIDs() const
{
    TArray<FString> Result;
    for (const FQuest_Definition& Q : QuestRegistry)
    {
        if (Q.State == EQuest_State::Active)
        {
            Result.Add(Q.QuestID);
        }
    }
    return Result;
}

int32 AQuestManager::GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return 0;
    }
    for (const FQuest_Objective& Obj : QuestRegistry[Idx].Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            return Obj.CurrentCount;
        }
    }
    return 0;
}

float AQuestManager::GetQuestTimeRemaining(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return 0.0f;
    }
    const FQuest_Definition& Quest = QuestRegistry[Idx];
    if (!Quest.bHasTimeLimit || Quest.State != EQuest_State::Active)
    {
        return Quest.TimeLimit;
    }

    const float* StartTime = QuestStartTimes.Find(QuestID);
    if (!StartTime)
    {
        return Quest.TimeLimit;
    }

    float Elapsed = GetWorld()->GetTimeSeconds() - *StartTime;
    return FMath::Max(0.0f, Quest.TimeLimit - Elapsed);
}

void AQuestManager::CheckTimeLimits(float DeltaTime)
{
    for (FQuest_Definition& Quest : QuestRegistry)
    {
        if (Quest.State != EQuest_State::Active || !Quest.bHasTimeLimit)
        {
            continue;
        }

        const float* StartTime = QuestStartTimes.Find(Quest.QuestID);
        if (!StartTime)
        {
            continue;
        }

        float Elapsed = GetWorld()->GetTimeSeconds() - *StartTime;
        if (Elapsed >= Quest.TimeLimit)
        {
            Quest.State = EQuest_State::Failed;
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest TIMED OUT — %s"), *Quest.QuestTitle);
        }
    }
}

bool AQuestManager::AreAllObjectivesComplete(const FQuest_Definition& Quest) const
{
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            return false;
        }
    }
    return Quest.Objectives.Num() > 0;
}

int32 AQuestManager::FindQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < QuestRegistry.Num(); ++i)
    {
        if (QuestRegistry[i].QuestID == QuestID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}
