// QuestSystem.h
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_002
// Prehistoric survival quest system — hunt, rescue, migration, defense missions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "QuestSystem.generated.h"

// ─── Enums ──────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),        // Track and kill a specific dinosaur
    Rescue          UMETA(DisplayName = "Rescue"),      // Save NPCs from danger
    Gather          UMETA(DisplayName = "Gather"),      // Collect resources in the world
    Explore         UMETA(DisplayName = "Explore"),     // Reach and mark new locations
    Defend          UMETA(DisplayName = "Defend"),      // Protect camp from predator attack
    Migration       UMETA(DisplayName = "Migration"),   // Follow herd to new territory
    Craft           UMETA(DisplayName = "Craft"),       // Build a specific tool or shelter
    Scout           UMETA(DisplayName = "Scout")        // Observe and report on dinosaur behaviour
};

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectivesMet   UMETA(DisplayName = "ObjectivesMet"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget          UMETA(DisplayName = "KillTarget"),
    ReachLocation       UMETA(DisplayName = "ReachLocation"),
    CollectItem         UMETA(DisplayName = "CollectItem"),
    EscortNPC           UMETA(DisplayName = "EscortNPC"),
    SurviveTimer        UMETA(DisplayName = "SurviveTimer"),
    InteractWithActor   UMETA(DisplayName = "InteractWithActor"),
    FollowTrail         UMETA(DisplayName = "FollowTrail"),
    ObserveDinosaur     UMETA(DisplayName = "ObserveDinosaur")
};

UENUM(BlueprintType)
enum class EQuest_Reward : uint8
{
    Flint           UMETA(DisplayName = "Flint"),
    DriedMeat       UMETA(DisplayName = "DriedMeat"),
    AnimalHide      UMETA(DisplayName = "AnimalHide"),
    BoneWeapon      UMETA(DisplayName = "BoneWeapon"),
    FireKit         UMETA(DisplayName = "FireKit"),
    WaterContainer  UMETA(DisplayName = "WaterContainer"),
    MapKnowledge    UMETA(DisplayName = "MapKnowledge")  // Reveals fog-of-war area
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::ReachLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName TargetTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bOptional = false;
};

USTRUCT(BlueprintType)
struct FQuest_RewardEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Reward RewardType = EQuest_Reward::Flint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Hunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_RewardEntry> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;  // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName PrerequisiteQuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName GiverActorTag = NAME_None;  // Tag of the NPC that gives this quest
};

// ─── Quest Manager Actor ─────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest Lifecycle ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(FName QuestID);

    // ── Objective Progress ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyKill(FName TargetTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyItemCollected(FName ItemTag, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyLocationReached(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyActorInteracted(FName ActorTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyDinosaurObserved(FName DinoSpeciesTag);

    // ── Crowd Integration ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnCrowdFleeTriggered(FVector ThreatLocation);  // Spawns rescue quest

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdMigrationStarted(FVector MigrationDirection);  // Activates migration quest

    // ── Query ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Quest")
    EQuest_State GetQuestState(FName QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FName> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Definition GetQuestDefinition(FName QuestID) const;

    // ── Data ─────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Data")
    TArray<FQuest_Definition> QuestLibrary;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State")
    TMap<FName, EQuest_State> QuestStateMap;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State")
    TMap<FName, FQuest_Definition> ActiveQuestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    int32 MaxActiveQuests = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    float ObjectiveCheckInterval = 1.0f;

private:
    void InitializeDefaultQuests();
    void CheckObjectiveCompletion();
    void GrantRewards(const FQuest_Definition& Quest);
    bool AreAllObjectivesMet(const FQuest_Definition& Quest) const;

    float ObjectiveCheckTimer = 0.0f;

    // Camp anchor locations (set from Agent #13 crowd spawner)
    FVector CampAlphaLocation  = FVector(2000.0f,  1500.0f, 0.0f);
    FVector CampBetaLocation   = FVector(-1800.0f, 2200.0f, 0.0f);
    FVector CampGammaLocation  = FVector(3500.0f, -1000.0f, 0.0f);
};
