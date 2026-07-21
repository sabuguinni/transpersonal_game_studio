#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestSystemManager.generated.h"

// ============================================================
//  Quest & Mission Designer — Agent #14
//  PROD_CYCLE_AUTO_20260630_003
//  QuestSystemManager — core quest tracking and objective system
// ============================================================

// --- Enums (global scope, Quest_ prefix) ---

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveMet    UMETA(DisplayName = "Objective Met"),
    Failed          UMETA(DisplayName = "Failed"),
    Completed       UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt            UMETA(DisplayName = "Hunt Target"),
    Collect         UMETA(DisplayName = "Collect Item"),
    Reach           UMETA(DisplayName = "Reach Location"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Protect         UMETA(DisplayName = "Protect NPC"),
    Observe         UMETA(DisplayName = "Observe Without Detection"),
    Craft           UMETA(DisplayName = "Craft Item"),
    Escape          UMETA(DisplayName = "Escape Danger Zone")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Resources       UMETA(DisplayName = "Resources"),
    CraftingBlueprint UMETA(DisplayName = "Crafting Blueprint"),
    TribeReputation UMETA(DisplayName = "Tribe Reputation"),
    WeaponUpgrade   UMETA(DisplayName = "Weapon Upgrade"),
    ShelterAccess   UMETA(DisplayName = "Shelter Access")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Scavenger       UMETA(DisplayName = "Scavenger"),   // Beginner
    Hunter          UMETA(DisplayName = "Hunter"),       // Intermediate
    Apex            UMETA(DisplayName = "Apex"),         // Hard
    Extinction      UMETA(DisplayName = "Extinction")   // Extreme
};

// --- Structs (global scope) ---

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetTag;  // e.g. "Raptor", "ObsidianBlade", "RiverCrossing"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector WorldLocation;  // For Reach/Observe objectives

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
        , TargetTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsOptional(false)
        , bIsCompleted(false)
        , WorldLocation(FVector::ZeroVector)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::Resources)
        , RewardID(TEXT(""))
        , Quantity(1)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;  // 0 = no time limit

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Status(EQuest_Status::Inactive)
        , Difficulty(EQuest_Difficulty::Hunter)
        , GiverNPCTag(TEXT(""))
        , bIsMainQuest(false)
        , bIsRepeatable(false)
        , TimeLimit(0.0f)
    {}
};

// ============================================================
//  AQuestSystemManager — world actor managing all active quests
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Active quest registry ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> FailedQuests;

    // --- Predefined quest library (loaded at BeginPlay) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Library")
    TArray<FQuest_Definition> QuestLibrary;

    // --- Settings ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    int32 MaxConcurrentQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    bool bDebugDrawObjectives;

    // --- Core quest operations ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // --- Objective progress ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportTagProgress(const FString& TargetTag, int32 Delta);

    // --- Query ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // --- Crowd integration ---
    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnStampedeTriggered(FVector StampedeTriggerLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnHerdObserved(const FString& HerdSpeciesTag, float ObservationDuration);

    // --- Initialization ---
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void PrintActiveQuests();

private:
    void RegisterDefaultQuestLibrary();
    void CheckQuestCompletion(FQuest_Definition& Quest);
    void GrantRewards(const FQuest_Definition& Quest);
    float QuestTimerAccumulator;
};
