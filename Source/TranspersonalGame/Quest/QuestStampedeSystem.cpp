#include "QuestStampedeSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AQuest_StampedeManager::AQuest_StampedeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    QuestState        = EQuest_StampedeState::Inactive;
    TimeLimit         = 90.0f;
    TimeRemaining     = 90.0f;
    HighGroundMinZ    = 350.0f;
    HighGroundRadius  = 400.0f;
    HighGroundCenter  = FVector(1200.0f, -800.0f, 400.0f);
    CampLocation      = FVector(-500.0f, 300.0f, 100.0f);
    CampDangerRadius  = 600.0f;
    HerdActor         = nullptr;
    bHerdTriggered    = false;

    // Define objectives
    FQuest_StampedeObjective Obj1;
    Obj1.ObjectiveText  = TEXT("Reach high ground before the herd overruns camp");
    Obj1.bCompleted     = false;
    Obj1.TargetLocation = HighGroundCenter;
    Objectives.Add(Obj1);

    FQuest_StampedeObjective Obj2;
    Obj2.ObjectiveText  = TEXT("Survive until the herd passes");
    Obj2.bCompleted     = false;
    Obj2.TargetLocation = HighGroundCenter;
    Objectives.Add(Obj2);
}

void AQuest_StampedeManager::BeginPlay()
{
    Super::BeginPlay();
    // Quest starts automatically on BeginPlay for prototype testing
    StartQuest();
}

void AQuest_StampedeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (QuestState == EQuest_StampedeState::Active)
    {
        CheckQuestConditions(DeltaTime);
    }
}

void AQuest_StampedeManager::StartQuest()
{
    if (QuestState != EQuest_StampedeState::Inactive)
    {
        return;
    }

    QuestState    = EQuest_StampedeState::Active;
    TimeRemaining = TimeLimit;

    // Reset objectives
    for (FQuest_StampedeObjective& Obj : Objectives)
    {
        Obj.bCompleted = false;
    }

    UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE_STARTED: Player has %.0f seconds to reach high ground"), TimeLimit);

    // Trigger the herd stampede immediately
    TriggerHerdStampede();
}

void AQuest_StampedeManager::TriggerHerdStampede()
{
    if (bHerdTriggered)
    {
        return;
    }

    bHerdTriggered = true;

    // If we have a herd actor reference, call TriggerFlee on it
    // CrowdHerdBehaviorActor exposes TriggerFlee(FVector ThreatLocation, float ThreatRadius)
    if (HerdActor)
    {
        // Call via UFunction reflection — works with ACrowd_HerdBehaviorActor
        FVector ThreatLocation = CampLocation;
        UFunction* FleeFunc = HerdActor->FindFunction(TEXT("TriggerFlee"));
        if (FleeFunc)
        {
            struct { FVector Loc; float Radius; } Params;
            Params.Loc    = ThreatLocation;
            Params.Radius = 800.0f;
            HerdActor->ProcessEvent(FleeFunc, &Params);
            UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE: TriggerFlee called on HerdActor"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE: No HerdActor assigned — herd flee simulated"));
    }
}

void AQuest_StampedeManager::CheckQuestConditions(float DeltaTime)
{
    TimeRemaining -= DeltaTime;

    APawn* Player = GetPlayerPawn();
    if (!Player)
    {
        return;
    }

    // SUCCESS: Player reached high ground
    if (IsPlayerOnHighGround())
    {
        if (Objectives.IsValidIndex(0) && !Objectives[0].bCompleted)
        {
            Objectives[0].bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE_OBJ1_COMPLETE: Player on high ground"));
        }

        // Wait for herd to pass (time runs out = herd passed)
        if (TimeRemaining <= 0.0f)
        {
            if (Objectives.IsValidIndex(1))
            {
                Objectives[1].bCompleted = true;
            }
            CompleteQuest(true);
        }
        return;
    }

    // FAIL: Herd reached camp while player is still in danger zone
    if (IsHerdNearCamp() && TimeRemaining < TimeLimit * 0.5f)
    {
        FVector PlayerLoc = Player->GetActorLocation();
        float DistToCamp  = FVector::Dist(PlayerLoc, CampLocation);
        if (DistToCamp < CampDangerRadius)
        {
            CompleteQuest(false);
            return;
        }
    }

    // FAIL: Time ran out and player never reached high ground
    if (TimeRemaining <= 0.0f)
    {
        CompleteQuest(false);
    }
}

void AQuest_StampedeManager::CompleteQuest(bool bSuccess)
{
    if (QuestState != EQuest_StampedeState::Active)
    {
        return;
    }

    QuestState = bSuccess ? EQuest_StampedeState::Succeeded : EQuest_StampedeState::Failed;

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE_SUCCESS: Player survived the stampede!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE_FAILED: Player was caught in the stampede."));
    }
}

bool AQuest_StampedeManager::IsPlayerOnHighGround() const
{
    APawn* Player = GetPlayerPawn();
    if (!Player)
    {
        return false;
    }

    FVector PlayerLoc = Player->GetActorLocation();
    float HorizDist   = FVector::Dist2D(PlayerLoc, HighGroundCenter);

    return (PlayerLoc.Z >= HighGroundMinZ && HorizDist <= HighGroundRadius);
}

bool AQuest_StampedeManager::IsHerdNearCamp() const
{
    if (!HerdActor)
    {
        // Simulate: herd is "near camp" when half the time has elapsed
        return (TimeRemaining < TimeLimit * 0.5f);
    }

    FVector HerdLoc = HerdActor->GetActorLocation();
    float Dist      = FVector::Dist(HerdLoc, CampLocation);
    return (Dist < CampDangerRadius * 2.0f);
}

float AQuest_StampedeManager::GetTimeRemainingNormalized() const
{
    if (TimeLimit <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp(TimeRemaining / TimeLimit, 0.0f, 1.0f);
}

void AQuest_StampedeManager::ResetQuest()
{
    QuestState     = EQuest_StampedeState::Inactive;
    TimeRemaining  = TimeLimit;
    bHerdTriggered = false;

    for (FQuest_StampedeObjective& Obj : Objectives)
    {
        Obj.bCompleted = false;
    }

    UE_LOG(LogTemp, Warning, TEXT("QUEST_STAMPEDE_RESET"));
}

APawn* AQuest_StampedeManager::GetPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }

    return PC->GetPawn();
}
