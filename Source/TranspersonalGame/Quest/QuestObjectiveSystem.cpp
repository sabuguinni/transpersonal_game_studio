#include "Quest/QuestObjectiveSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// AQuest_ObjectiveTrigger Implementation
// ============================================================

AQuest_ObjectiveTrigger::AQuest_ObjectiveTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    TriggerRadius = 300.0f;
    bIsActive = false;
    RequiredKillCount = 0;
    CurrentKillCount = 0;
    TimeLimitSeconds = 0.0f;
    ElapsedTime = 0.0f;
    bCompleted = false;
    bFailed = false;
    ObjectiveIndex = 0;
}

void AQuest_ObjectiveTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->SetSphereRadius(TriggerRadius);
}

void AQuest_ObjectiveTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive || bCompleted || bFailed)
        return;

    // Time limit check
    if (TimeLimitSeconds > 0.0f)
    {
        ElapsedTime += DeltaTime;
        if (ElapsedTime >= TimeLimitSeconds)
        {
            bFailed = true;
            bIsActive = false;
            OnObjectiveFailed.Broadcast(QuestID, ObjectiveIndex);
            UE_LOG(LogTemp, Warning, TEXT("Quest [%s] Objective %d FAILED — time limit exceeded"), *QuestID.ToString(), ObjectiveIndex);
            return;
        }
    }

    // Proximity check for location objectives
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Player)
    {
        float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
        if (DistToPlayer <= TriggerRadius)
        {
            OnPlayerEnterZone();
        }
    }
}

void AQuest_ObjectiveTrigger::ActivateTrigger()
{
    bIsActive = true;
    bCompleted = false;
    bFailed = false;
    ElapsedTime = 0.0f;
    CurrentKillCount = 0;
    UE_LOG(LogTemp, Log, TEXT("Quest [%s] Objective %d ACTIVATED"), *QuestID.ToString(), ObjectiveIndex);
}

void AQuest_ObjectiveTrigger::DeactivateTrigger()
{
    bIsActive = false;
    UE_LOG(LogTemp, Log, TEXT("Quest [%s] Objective %d DEACTIVATED"), *QuestID.ToString(), ObjectiveIndex);
}

void AQuest_ObjectiveTrigger::OnPlayerEnterZone()
{
    if (!bIsActive || bCompleted || bFailed)
        return;

    // For location-based objectives, entering the zone completes it
    if (RequiredKillCount == 0)
    {
        bCompleted = true;
        bIsActive = false;
        OnObjectiveCompleted.Broadcast(QuestID, ObjectiveIndex);
        UE_LOG(LogTemp, Log, TEXT("Quest [%s] Objective %d COMPLETED — player reached zone"), *QuestID.ToString(), ObjectiveIndex);
    }
}

void AQuest_ObjectiveTrigger::RegisterKill(FName DinosaurSpecies)
{
    if (!bIsActive || bCompleted || bFailed)
        return;

    CurrentKillCount++;
    UE_LOG(LogTemp, Log, TEXT("Quest [%s] Objective %d — Kill registered: %s (%d/%d)"),
        *QuestID.ToString(), ObjectiveIndex, *DinosaurSpecies.ToString(), CurrentKillCount, RequiredKillCount);

    if (RequiredKillCount > 0 && CurrentKillCount >= RequiredKillCount)
    {
        bCompleted = true;
        bIsActive = false;
        OnObjectiveCompleted.Broadcast(QuestID, ObjectiveIndex);
        UE_LOG(LogTemp, Log, TEXT("Quest [%s] Objective %d COMPLETED — kill count reached"), *QuestID.ToString(), ObjectiveIndex);
    }
}

bool AQuest_ObjectiveTrigger::IsObjectiveComplete() const
{
    return bCompleted;
}

// ============================================================
// AQuest_Manager Implementation
// ============================================================

AQuest_Manager::AQuest_Manager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AQuest_Manager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCoreQuests();
    RegisterAllTriggers();
    UE_LOG(LogTemp, Log, TEXT("QuestManager initialized — %d quests loaded"), QuestDatabase.Num());
}

void AQuest_Manager::InitializeCoreQuests()
{
    QuestDatabase.Empty();
    BuildRaptorGauntletQuest();
    BuildSurviveStampedeQuest();
    BuildHuntLonePredatorQuest();
    UE_LOG(LogTemp, Log, TEXT("Core quests initialized: %d quests"), QuestDatabase.Num());
}

void AQuest_Manager::BuildRaptorGauntletQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = FName("Quest_RaptorGauntlet");
    Quest.QuestName = FText::FromString("Raptor Gauntlet");
    Quest.QuestDescription = FText::FromString(
        "A pack of raptors is hunting between you and the river crossing. "
        "Move through the tall grass without being detected. "
        "Reach the river before they cut off your escape route."
    );
    Quest.QuestType = EQuest_Type::MainMission;
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsRepeatable = false;
    Quest.RecommendedLevel = 1;

    // Objective 1: Leave the safe zone
    FQuest_Objective Obj1;
    Obj1.ObjectiveID = FName("Obj_LeaveSafeZone");
    Obj1.Description = FText::FromString("Leave the camp and head east");
    Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj1.TargetLocation = FVector(-200, 0, 100);
    Obj1.CompletionRadius = 400.0f;
    Obj1.bIsOptional = false;
    Quest.Objectives.Add(Obj1);

    // Objective 2: Pass through raptor territory without being seen
    FQuest_Objective Obj2;
    Obj2.ObjectiveID = FName("Obj_PassRaptorTerritory");
    Obj2.Description = FText::FromString("Move through the raptor hunting ground undetected");
    Obj2.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj2.TargetLocation = FVector(1200, 2000, 100);
    Obj2.CompletionRadius = 500.0f;
    Obj2.TimeLimitSeconds = 120.0f;
    Obj2.bIsOptional = false;
    Quest.Objectives.Add(Obj2);

    // Objective 3: Reach the river crossing
    FQuest_Objective Obj3;
    Obj3.ObjectiveID = FName("Obj_ReachRiverCrossing");
    Obj3.Description = FText::FromString("Reach the river crossing");
    Obj3.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj3.TargetLocation = FVector(2500, -500, 150);
    Obj3.CompletionRadius = 600.0f;
    Obj3.bIsOptional = false;
    Quest.Objectives.Add(Obj3);

    // Rewards
    Quest.Rewards.ExperiencePoints = 150;
    Quest.Rewards.ResourceRewards.Add(FName("StoneTool"), 2);
    Quest.Rewards.ResourceRewards.Add(FName("DriedMeat"), 3);

    QuestDatabase.Add(Quest);
}

void AQuest_Manager::BuildSurviveStampedeQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = FName("Quest_SurviveStampede");
    Quest.QuestName = FText::FromString("Survive the Stampede");
    Quest.QuestDescription = FText::FromString(
        "A massive Triceratops herd has been spooked and is stampeding through the valley. "
        "The ground shakes beneath their feet. Reach high ground before the herd crushes everything in its path."
    );
    Quest.QuestType = EQuest_Type::MainMission;
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsRepeatable = false;
    Quest.RecommendedLevel = 1;

    // Objective 1: Detect the stampede (triggered by proximity to herd)
    FQuest_Objective Obj1;
    Obj1.ObjectiveID = FName("Obj_DetectStampede");
    Obj1.Description = FText::FromString("Investigate the rumbling from the north");
    Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj1.TargetLocation = FVector(2200, 2000, 100);
    Obj1.CompletionRadius = 800.0f;
    Obj1.bIsOptional = false;
    Quest.Objectives.Add(Obj1);

    // Objective 2: Escape to high ground — timed
    FQuest_Objective Obj2;
    Obj2.ObjectiveID = FName("Obj_ReachHighGround");
    Obj2.Description = FText::FromString("Reach high ground before the stampede arrives!");
    Obj2.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj2.TargetLocation = FVector(1200, 4500, 400);
    Obj2.CompletionRadius = 600.0f;
    Obj2.TimeLimitSeconds = 60.0f;
    Obj2.bIsOptional = false;
    Quest.Objectives.Add(Obj2);

    // Rewards
    Quest.Rewards.ExperiencePoints = 200;
    Quest.Rewards.ResourceRewards.Add(FName("TriceratopsHide"), 1);
    Quest.Rewards.ResourceRewards.Add(FName("Bone"), 4);

    QuestDatabase.Add(Quest);
}

void AQuest_Manager::BuildHuntLonePredatorQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = FName("Quest_HuntLonePredator");
    Quest.QuestName = FText::FromString("Hunt the Lone Predator");
    Quest.QuestDescription = FText::FromString(
        "A lone T-Rex has been stalking the western hunting grounds, killing game and making the area dangerous. "
        "Track it to its territory, study its patrol pattern, then drive it away or bring it down."
    );
    Quest.QuestType = EQuest_Type::SideQuest;
    Quest.Status = EQuest_Status::Inactive;
    Quest.bIsRepeatable = false;
    Quest.RecommendedLevel = 3;

    // Objective 1: Find the T-Rex territory
    FQuest_Objective Obj1;
    Obj1.ObjectiveID = FName("Obj_FindTRexTerritory");
    Obj1.Description = FText::FromString("Find signs of the T-Rex in the western grounds");
    Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj1.TargetLocation = FVector(-3000, -2000, 100);
    Obj1.CompletionRadius = 700.0f;
    Obj1.bIsOptional = false;
    Quest.Objectives.Add(Obj1);

    // Objective 2: Observe the T-Rex (stealth — don't be detected)
    FQuest_Objective Obj2;
    Obj2.ObjectiveID = FName("Obj_ObserveTRex");
    Obj2.Description = FText::FromString("Observe the T-Rex patrol pattern without being detected");
    Obj2.ObjectiveType = EQuest_ObjectiveType::StealthObjective;
    Obj2.TargetCount = 1;
    Obj2.bIsOptional = false;
    Quest.Objectives.Add(Obj2);

    // Objective 3: Kill or drive away the T-Rex
    FQuest_Objective Obj3;
    Obj3.ObjectiveID = FName("Obj_DefeatTRex");
    Obj3.Description = FText::FromString("Defeat the T-Rex (kill or drive it from the territory)");
    Obj3.ObjectiveType = EQuest_ObjectiveType::KillTarget;
    Obj3.TargetSpecies = FName("TyrannosaurusRex");
    Obj3.TargetCount = 1;
    Obj3.bIsOptional = false;
    Quest.Objectives.Add(Obj3);

    // Rewards
    Quest.Rewards.ExperiencePoints = 500;
    Quest.Rewards.ResourceRewards.Add(FName("TRexTooth"), 3);
    Quest.Rewards.ResourceRewards.Add(FName("TRexHide"), 2);
    Quest.Rewards.ResourceRewards.Add(FName("Bone"), 8);

    QuestDatabase.Add(Quest);
}

bool AQuest_Manager::StartQuest(FName QuestID)
{
    if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest [%s] cannot start — already active or completed"), *QuestID.ToString());
        return false;
    }

    for (FQuest_Definition& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.Status = EQuest_Status::Active;
            ActiveQuestIDs.Add(QuestID);
            ActiveQuestProgress.Add(QuestID, 0);

            // Activate first objective trigger
            for (AQuest_ObjectiveTrigger* Trigger : RegisteredTriggers)
            {
                if (Trigger && Trigger->QuestID == QuestID && Trigger->ObjectiveIndex == 0)
                {
                    Trigger->ActivateTrigger();
                }
            }

            OnQuestStarted.Broadcast(QuestID, EQuest_Status::Active);
            UE_LOG(LogTemp, Log, TEXT("Quest [%s] STARTED"), *QuestID.ToString());
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Quest [%s] not found in database"), *QuestID.ToString());
    return false;
}

void AQuest_Manager::CompleteQuest(FName QuestID)
{
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.Add(QuestID);
    ActiveQuestProgress.Remove(QuestID);

    for (FQuest_Definition& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.Status = EQuest_Status::Completed;
            break;
        }
    }

    OnQuestCompleted.Broadcast(QuestID, EQuest_Status::Completed);
    UE_LOG(LogTemp, Log, TEXT("Quest [%s] COMPLETED"), *QuestID.ToString());
}

void AQuest_Manager::FailQuest(FName QuestID)
{
    ActiveQuestIDs.Remove(QuestID);
    FailedQuestIDs.Add(QuestID);
    ActiveQuestProgress.Remove(QuestID);

    for (FQuest_Definition& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.Status = EQuest_Status::Failed;
            break;
        }
    }

    OnQuestFailed.Broadcast(QuestID, EQuest_Status::Failed);
    UE_LOG(LogTemp, Warning, TEXT("Quest [%s] FAILED"), *QuestID.ToString());
}

void AQuest_Manager::AbandonQuest(FName QuestID)
{
    ActiveQuestIDs.Remove(QuestID);
    ActiveQuestProgress.Remove(QuestID);

    for (FQuest_Definition& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.Status = EQuest_Status::Abandoned;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Quest [%s] ABANDONED"), *QuestID.ToString());
}

void AQuest_Manager::AdvanceQuestObjective(FName QuestID)
{
    if (!ActiveQuestProgress.Contains(QuestID))
        return;

    int32& CurrentObjIndex = ActiveQuestProgress[QuestID];
    CurrentObjIndex++;

    FQuest_Definition QuestDef;
    if (!GetQuestDefinition(QuestID, QuestDef))
        return;

    if (CurrentObjIndex >= QuestDef.Objectives.Num())
    {
        // All objectives done — complete the quest
        CompleteQuest(QuestID);
        return;
    }

    // Activate next objective trigger
    for (AQuest_ObjectiveTrigger* Trigger : RegisteredTriggers)
    {
        if (Trigger && Trigger->QuestID == QuestID)
        {
            if (Trigger->ObjectiveIndex == CurrentObjIndex)
                Trigger->ActivateTrigger();
            else
                Trigger->DeactivateTrigger();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Quest [%s] advanced to objective %d"), *QuestID.ToString(), CurrentObjIndex);
}

FQuest_Objective AQuest_Manager::GetCurrentObjective(FName QuestID) const
{
    if (!ActiveQuestProgress.Contains(QuestID))
        return FQuest_Objective();

    int32 ObjIndex = ActiveQuestProgress[QuestID];

    FQuest_Definition QuestDef;
    if (GetQuestDefinition(QuestID, QuestDef) && ObjIndex < QuestDef.Objectives.Num())
    {
        return QuestDef.Objectives[ObjIndex];
    }

    return FQuest_Objective();
}

bool AQuest_Manager::IsQuestActive(FName QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool AQuest_Manager::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

bool AQuest_Manager::GetQuestDefinition(FName QuestID, FQuest_Definition& OutDefinition) const
{
    for (const FQuest_Definition& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            OutDefinition = Quest;
            return true;
        }
    }
    return false;
}

void AQuest_Manager::RegisterAllTriggers()
{
    RegisteredTriggers.Empty();

    TArray<AActor*> FoundTriggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_ObjectiveTrigger::StaticClass(), FoundTriggers);

    for (AActor* Actor : FoundTriggers)
    {
        AQuest_ObjectiveTrigger* Trigger = Cast<AQuest_ObjectiveTrigger>(Actor);
        if (Trigger)
        {
            RegisteredTriggers.Add(Trigger);
            UE_LOG(LogTemp, Log, TEXT("Registered trigger: Quest[%s] Obj[%d]"),
                *Trigger->QuestID.ToString(), Trigger->ObjectiveIndex);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: %d triggers registered"), RegisteredTriggers.Num());
}
