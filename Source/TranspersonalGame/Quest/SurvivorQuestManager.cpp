#include "SurvivorQuestManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================

AQuest_SurvivorQuestManager::AQuest_SurvivorQuestManager()
    : CurrentState(EQuest_SurvivorState::Inactive)
    , bPlayerInHerdZone(false)
    , bPlayerInSafeZone(false)
    , StampedeElapsedTime(0.f)
    , HerdZoneRadius(800.f)
    , SafeZoneRadius(1500.f)
    , StampedeTimeLimit(45.f)
    , StampedeActivationDelay(3.f)
    , ActivationTimer(0.f)
    , bQuestInitialised(false)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz — sufficient for quest logic

    // Root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("QuestRoot"));

    // Herd zone — player enters this to trigger stampede watch
    HerdZoneTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("HerdZoneTrigger"));
    HerdZoneTrigger->SetupAttachment(RootComponent);
    HerdZoneTrigger->SetSphereRadius(HerdZoneRadius);
    HerdZoneTrigger->SetCollisionProfileName(TEXT("Trigger"));
    HerdZoneTrigger->ShapeColor = FColor::Orange;

    // Safe zone — player must reach this to complete the quest
    SafeZoneTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZoneTrigger"));
    SafeZoneTrigger->SetupAttachment(RootComponent);
    SafeZoneTrigger->SetSphereRadius(SafeZoneRadius);
    SafeZoneTrigger->SetCollisionProfileName(TEXT("Trigger"));
    SafeZoneTrigger->ShapeColor = FColor::Green;

    // Audio URLs (populated from ElevenLabs TTS output)
    WarningSoundURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782952412700_QuestNPC_Survive_Stampede.mp3");
    CompleteSoundURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782952430655_QuestNPC_Stampede_Complete.mp3");
}

// ============================================================
// BeginPlay
// ============================================================

void AQuest_SurvivorQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap delegates
    HerdZoneTrigger->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivorQuestManager::OnHerdZoneBeginOverlap);
    HerdZoneTrigger->OnComponentEndOverlap.AddDynamic(this, &AQuest_SurvivorQuestManager::OnHerdZoneEndOverlap);
    SafeZoneTrigger->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivorQuestManager::OnSafeZoneBeginOverlap);

    InitialiseObjectives();
    bQuestInitialised = true;

    // Quest starts in WatchingHerd state — player must approach the herd
    CurrentState = EQuest_SurvivorState::WatchingHerd;

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Survive the Stampede — initialised. Waiting for player to enter herd zone."));
}

// ============================================================
// Tick
// ============================================================

void AQuest_SurvivorQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bQuestInitialised) return;

    switch (CurrentState)
    {
        case EQuest_SurvivorState::WatchingHerd:
            // Passive — waiting for player to enter herd zone (handled by overlap)
            break;

        case EQuest_SurvivorState::EscapeWindow:
            // Count down activation delay before full stampede
            ActivationTimer += DeltaTime;
            if (ActivationTimer >= StampedeActivationDelay)
            {
                TriggerStampedePhase();
            }
            break;

        case EQuest_SurvivorState::StampedeActive:
            // Track elapsed time — fail if player doesn't escape in time
            StampedeElapsedTime += DeltaTime;
            if (StampedeElapsedTime >= StampedeTimeLimit)
            {
                FailQuest();
            }
            CheckObjectiveCompletion();
            break;

        default:
            break;
    }
}

// ============================================================
// Quest flow
// ============================================================

void AQuest_SurvivorQuestManager::ActivateQuest()
{
    if (CurrentState != EQuest_SurvivorState::WatchingHerd) return;

    CurrentState = EQuest_SurvivorState::EscapeWindow;
    ActivationTimer = 0.f;

    // Activate objective 0: "Escape the herd zone"
    if (Objectives.IsValidIndex(0))
    {
        Objectives[0].Status = EQuest_ObjectiveStatus::Active;
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: STAMPEDE INCOMING — player has %.1f seconds to react!"), StampedeActivationDelay);
}

void AQuest_SurvivorQuestManager::TriggerStampedePhase()
{
    CurrentState = EQuest_SurvivorState::StampedeActive;
    StampedeElapsedTime = 0.f;

    // Activate objective 1: "Reach high ground before the herd arrives"
    if (Objectives.IsValidIndex(1))
    {
        Objectives[1].Status = EQuest_ObjectiveStatus::Active;
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: STAMPEDE ACTIVE — %.1f seconds to escape!"), StampedeTimeLimit);
}

void AQuest_SurvivorQuestManager::CompleteQuest()
{
    if (CurrentState == EQuest_SurvivorState::Completed) return;

    CurrentState = EQuest_SurvivorState::Completed;

    // Mark all active objectives complete
    for (FQuest_SurvivorObjective& Obj : Objectives)
    {
        if (Obj.Status == EQuest_ObjectiveStatus::Active)
        {
            Obj.Status = EQuest_ObjectiveStatus::Completed;
        }
    }

    AwardRewards();

    UE_LOG(LogTemp, Log, TEXT("QuestManager: QUEST COMPLETE — Survived the Stampede! XP awarded: %d survival, %d crafting"),
           QuestReward.SurvivalXP, QuestReward.CraftingXP);
}

void AQuest_SurvivorQuestManager::FailQuest()
{
    if (CurrentState == EQuest_SurvivorState::Failed) return;

    CurrentState = EQuest_SurvivorState::Failed;

    for (FQuest_SurvivorObjective& Obj : Objectives)
    {
        if (Obj.Status == EQuest_ObjectiveStatus::Active)
        {
            Obj.Status = EQuest_ObjectiveStatus::Failed;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("QuestManager: QUEST FAILED — player was overtaken by the stampede."));
}

// ============================================================
// Objective management
// ============================================================

void AQuest_SurvivorQuestManager::InitialiseObjectives()
{
    Objectives.Empty();

    FQuest_SurvivorObjective Obj0;
    Obj0.ObjectiveText = TEXT("Escape the herd zone before the stampede begins");
    Obj0.Status = EQuest_ObjectiveStatus::Pending;
    Obj0.ProgressRequired = 1.f;
    Objectives.Add(Obj0);

    FQuest_SurvivorObjective Obj1;
    Obj1.ObjectiveText = TEXT("Reach high ground — get clear of the stampede path");
    Obj1.Status = EQuest_ObjectiveStatus::Pending;
    Obj1.ProgressRequired = 1.f;
    Objectives.Add(Obj1);

    FQuest_SurvivorObjective Obj2;
    Obj2.ObjectiveText = TEXT("Survive until the herd disperses");
    Obj2.Status = EQuest_ObjectiveStatus::Pending;
    Obj2.ProgressRequired = StampedeTimeLimit;
    Objectives.Add(Obj2);
}

void AQuest_SurvivorQuestManager::CheckObjectiveCompletion()
{
    // Obj2 tracks elapsed survival time
    if (Objectives.IsValidIndex(2) && CurrentState == EQuest_SurvivorState::StampedeActive)
    {
        Objectives[2].ProgressCurrent = StampedeElapsedTime;
        if (StampedeElapsedTime >= StampedeTimeLimit * 0.5f &&
            Objectives[2].Status == EQuest_ObjectiveStatus::Pending)
        {
            Objectives[2].Status = EQuest_ObjectiveStatus::Active;
        }
    }
}

void AQuest_SurvivorQuestManager::UpdateObjectiveProgress(int32 ObjectiveIndex, float Progress)
{
    if (!Objectives.IsValidIndex(ObjectiveIndex)) return;
    Objectives[ObjectiveIndex].ProgressCurrent = FMath::Clamp(Progress, 0.f, Objectives[ObjectiveIndex].ProgressRequired);
}

void AQuest_SurvivorQuestManager::AwardRewards()
{
    // Reward hook — integrate with TranspersonalGameState XP system
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Awarding %d SurvivalXP, %d CraftingXP, unlocking recipe: %s"),
           QuestReward.SurvivalXP, QuestReward.CraftingXP, *QuestReward.UnlockedRecipe);
}

// ============================================================
// Utility
// ============================================================

float AQuest_SurvivorQuestManager::GetStampedeTimeRemaining() const
{
    if (CurrentState != EQuest_SurvivorState::StampedeActive) return StampedeTimeLimit;
    return FMath::Max(0.f, StampedeTimeLimit - StampedeElapsedTime);
}

void AQuest_SurvivorQuestManager::DebugPrintQuestState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QUEST DEBUG: Survive the Stampede ==="));
    UE_LOG(LogTemp, Warning, TEXT("State: %d | InHerd: %d | InSafe: %d | Elapsed: %.1f"),
           (int32)CurrentState, bPlayerInHerdZone, bPlayerInSafeZone, StampedeElapsedTime);
    for (int32 i = 0; i < Objectives.Num(); ++i)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Obj[%d]: %s — Status: %d (%.1f/%.1f)"),
               i, *Objectives[i].ObjectiveText, (int32)Objectives[i].Status,
               Objectives[i].ProgressCurrent, Objectives[i].ProgressRequired);
    }
}

// ============================================================
// Overlap callbacks
// ============================================================

void AQuest_SurvivorQuestManager::OnHerdZoneBeginOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    bPlayerInHerdZone = true;
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Player entered herd zone — activating quest!"));

    if (CurrentState == EQuest_SurvivorState::WatchingHerd)
    {
        ActivateQuest();
    }
}

void AQuest_SurvivorQuestManager::OnHerdZoneEndOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    bPlayerInHerdZone = false;

    // Player escaped the herd zone — complete objective 0
    if (Objectives.IsValidIndex(0) && Objectives[0].Status == EQuest_ObjectiveStatus::Active)
    {
        Objectives[0].Status = EQuest_ObjectiveStatus::Completed;
        Objectives[0].ProgressCurrent = 1.f;
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective 0 complete — player escaped herd zone."));
    }
}

void AQuest_SurvivorQuestManager::OnSafeZoneBeginOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    bPlayerInSafeZone = true;

    if (CurrentState == EQuest_SurvivorState::StampedeActive ||
        CurrentState == EQuest_SurvivorState::EscapeWindow)
    {
        // Complete objective 1
        if (Objectives.IsValidIndex(1))
        {
            Objectives[1].Status = EQuest_ObjectiveStatus::Completed;
            Objectives[1].ProgressCurrent = 1.f;
        }
        CompleteQuest();
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Player reached safe zone — QUEST COMPLETE!"));
    }
}
