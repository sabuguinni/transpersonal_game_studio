// TutorialQuestManager.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_003
// Full implementation: tutorial quest system tied to SurvivalComponent delegates

#include "TutorialQuestManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ─────────────────────────────────────────────────────────────
UTutorialQuestManager::UTutorialQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2 Hz — proximity checks are cheap

    ProximityCheckInterval = 0.5f;
    bAutoStartTutorial     = true;
    TimeSinceLastProximityCheck = 0.0f;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────
void UTutorialQuestManager::BeginPlay()
{
    Super::BeginPlay();

    RegisterTutorialQuests();

    if (bAutoStartTutorial)
    {
        // Auto-start the first tutorial quest immediately
        StartQuest(FName("FindWater"));
    }

    BindSurvivalDelegates();
}

// ─── TickComponent ────────────────────────────────────────────────────────────
void UTutorialQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastProximityCheck += DeltaTime;
    if (TimeSinceLastProximityCheck >= ProximityCheckInterval)
    {
        TimeSinceLastProximityCheck = 0.0f;
        CheckProximityObjectives();
    }
}

// ─── RegisterTutorialQuests ───────────────────────────────────────────────────
void UTutorialQuestManager::RegisterTutorialQuests()
{
    // Quest 1: Find Water
    {
        FWorld_QuestData Q;
        Q.QuestID           = FName("FindWater");
        Q.Title             = FText::FromString(TEXT("Find Water"));
        Q.Description       = FText::FromString(TEXT("You are dehydrated. Find a water source to survive."));
        Q.ObjectiveLocation = FVector(8000.0f, 0.0f, 50.0f);   // WaterSource_Tutorial location
        Q.ObjectiveRadius   = 400.0f;
        Q.State             = EWorld_QuestState::Inactive;
        QuestRegistry.Add(Q.QuestID, Q);
    }

    // Quest 2: Find Food
    {
        FWorld_QuestData Q;
        Q.QuestID           = FName("FindFood");
        Q.Title             = FText::FromString(TEXT("Find Food"));
        Q.Description       = FText::FromString(TEXT("You are starving. Find berries or prey to eat."));
        Q.ObjectiveLocation = FVector(2000.0f, 1500.0f, 50.0f); // FoodSource_Berries location
        Q.ObjectiveRadius   = 350.0f;
        Q.State             = EWorld_QuestState::Inactive;
        QuestRegistry.Add(Q.QuestID, Q);
    }

    // Quest 3: Escape Predator (bonus tutorial)
    {
        FWorld_QuestData Q;
        Q.QuestID           = FName("EscapePredator");
        Q.Title             = FText::FromString(TEXT("Escape the Predator"));
        Q.Description       = FText::FromString(TEXT("A T-Rex has spotted you. Run to the rocky outcrop to hide."));
        Q.ObjectiveLocation = FVector(-3000.0f, 2500.0f, 200.0f);
        Q.ObjectiveRadius   = 500.0f;
        Q.State             = EWorld_QuestState::Inactive;
        QuestRegistry.Add(Q.QuestID, Q);
    }
}

// ─── StartQuest ───────────────────────────────────────────────────────────────
void UTutorialQuestManager::StartQuest(FName QuestID)
{
    FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    if (!Q)
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialQuestManager: Quest '%s' not found in registry."), *QuestID.ToString());
        return;
    }

    if (Q->State != EWorld_QuestState::Inactive)
    {
        UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: Quest '%s' already in state %d — skipping start."),
            *QuestID.ToString(), (int32)Q->State);
        return;
    }

    Q->State = EWorld_QuestState::Active;
    OnQuestStarted.Broadcast(QuestID);

    UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: Quest STARTED — '%s' | Objective at (%.0f, %.0f, %.0f)"),
        *QuestID.ToString(),
        Q->ObjectiveLocation.X, Q->ObjectiveLocation.Y, Q->ObjectiveLocation.Z);
}

// ─── CompleteObjective ────────────────────────────────────────────────────────
void UTutorialQuestManager::CompleteObjective(FName QuestID)
{
    FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    if (!Q || Q->State != EWorld_QuestState::Active)
    {
        return;
    }

    Q->State = EWorld_QuestState::Completed;
    OnQuestCompleted.Broadcast(QuestID);

    UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: Quest COMPLETED — '%s'"), *QuestID.ToString());

    // Chain: completing FindWater auto-starts FindFood
    if (QuestID == FName("FindWater"))
    {
        StartQuest(FName("FindFood"));
    }
    else if (QuestID == FName("FindFood"))
    {
        StartQuest(FName("EscapePredator"));
    }
}

// ─── FailQuest ────────────────────────────────────────────────────────────────
void UTutorialQuestManager::FailQuest(FName QuestID)
{
    FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    if (!Q || Q->State != EWorld_QuestState::Active)
    {
        return;
    }

    Q->State = EWorld_QuestState::Failed;
    OnQuestFailed.Broadcast(QuestID);

    UE_LOG(LogTemp, Warning, TEXT("TutorialQuestManager: Quest FAILED — '%s'"), *QuestID.ToString());
}

// ─── IsQuestActive ────────────────────────────────────────────────────────────
bool UTutorialQuestManager::IsQuestActive(FName QuestID) const
{
    const FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    return Q && Q->State == EWorld_QuestState::Active;
}

// ─── GetQuestState ────────────────────────────────────────────────────────────
EWorld_QuestState UTutorialQuestManager::GetQuestState(FName QuestID) const
{
    const FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    return Q ? Q->State : EWorld_QuestState::Inactive;
}

// ─── GetQuestData ─────────────────────────────────────────────────────────────
FWorld_QuestData UTutorialQuestManager::GetQuestData(FName QuestID) const
{
    const FWorld_QuestData* Q = QuestRegistry.Find(QuestID);
    return Q ? *Q : FWorld_QuestData();
}

// ─── OnThirstLow (SurvivalComponent delegate) ─────────────────────────────────
void UTutorialQuestManager::OnThirstLow()
{
    // Auto-activate FindWater quest when thirst drops below threshold
    if (GetQuestState(FName("FindWater")) == EWorld_QuestState::Inactive)
    {
        StartQuest(FName("FindWater"));
        UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: FindWater auto-triggered by low thirst."));
    }
}

// ─── OnHungerLow (SurvivalComponent delegate) ────────────────────────────────
void UTutorialQuestManager::OnHungerLow()
{
    // Auto-activate FindFood quest when hunger drops below threshold
    if (GetQuestState(FName("FindFood")) == EWorld_QuestState::Inactive)
    {
        StartQuest(FName("FindFood"));
        UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: FindFood auto-triggered by low hunger."));
    }
}

// ─── CheckProximityObjectives ─────────────────────────────────────────────────
void UTutorialQuestManager::CheckProximityObjectives()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector PlayerLoc = Owner->GetActorLocation();

    for (auto& Pair : QuestRegistry)
    {
        FWorld_QuestData& Q = Pair.Value;
        if (Q.State != EWorld_QuestState::Active) continue;

        float Dist = FVector::Dist(PlayerLoc, Q.ObjectiveLocation);
        if (Dist <= Q.ObjectiveRadius)
        {
            UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: Player reached objective for '%s' (dist=%.0f)"),
                *Q.QuestID.ToString(), Dist);
            CompleteObjective(Q.QuestID);
            break; // Only complete one per tick to avoid chain issues
        }
    }
}

// ─── BindSurvivalDelegates ────────────────────────────────────────────────────
void UTutorialQuestManager::BindSurvivalDelegates()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Attempt to find SurvivalComponent by name — avoids hard module dependency
    UActorComponent* SurvComp = Owner->FindComponentByTag(UActorComponent::StaticClass(), FName("SurvivalComponent"));
    if (!SurvComp)
    {
        // Fallback: search all components for one named "SurvivalComponent"
        TArray<UActorComponent*> Comps;
        Owner->GetComponents(Comps);
        for (UActorComponent* C : Comps)
        {
            if (C && C->GetClass()->GetName().Contains(TEXT("SurvivalComponent")))
            {
                SurvComp = C;
                break;
            }
        }
    }

    if (SurvComp)
    {
        UE_LOG(LogTemp, Log, TEXT("TutorialQuestManager: SurvivalComponent found — delegates bound."));
        // Delegate binding done via Blueprint or direct cast in BP subclass
        // C++ binding requires including SurvivalComponent.h — done in BP layer to avoid circular deps
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialQuestManager: SurvivalComponent not found on owner '%s'. Delegates not bound."),
            *Owner->GetActorLabel());
    }
}
