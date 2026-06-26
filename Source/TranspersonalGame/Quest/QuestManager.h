#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Reach           UMETA(DisplayName = "Reach Location"),
    Observe         UMETA(DisplayName = "Observe Without Alert"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Collect         UMETA(DisplayName = "Collect Resource"),
    Escape          UMETA(DisplayName = "Escape Zone"),
    Follow          UMETA(DisplayName = "Follow Entity"),
    Craft           UMETA(DisplayName = "Craft Item")
};

UENUM(BlueprintType)
enum class EQuest_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

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
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredDuration;   // For Survive/Observe objectives (seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;   // Distance to target to count as reached

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::Reach)
        , TargetLocation(FVector::ZeroVector)
        , RequiredDuration(0.f)
        , CompletionRadius(300.f)
        , bIsCompleted(false)
        , bIsOptional(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceFlint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceStick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceLeaf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlocksQuestID;

    FQuest_Reward()
        : ResourceFlint(0)
        , ResourceStick(0)
        , ResourceLeaf(0)
        , HealthBonus(0.f)
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
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_DangerLevel DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TriggerActorLabel;   // Label of TriggerBox that starts this quest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredQuestID;     // Must be completed before this unlocks

    FQuest_Definition()
        : State(EQuest_State::Locked)
        , DangerLevel(EQuest_DangerLevel::Medium)
        , CurrentObjectiveIndex(0)
    {}
};

// ============================================================
// UCLASS
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // --- UWorldSubsystem interface ---
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // --- Quest registration ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    // --- Objective progression ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCurrentObjective(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsObjectiveActive(const FString& QuestID, int32 ObjectiveIndex) const;

    // --- Player location check (called from Tick or trigger overlap) ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredTrigger(const FString& TriggerActorLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerLocationUpdated(const FVector& PlayerLocation);

    // --- Alert integration (from CrowdZoneManager) ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnCrowdAlertTriggered(const FString& ZoneID, uint8 AlertLevel);

    // --- Queries ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FString GetCurrentObjectiveText(const FString& QuestID) const;

    // --- Debug ---
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void PrintQuestStatus() const;

private:
    void InitializeDefaultQuests();
    void GrantReward(const FQuest_Reward& Reward);
    void CheckObjectiveCompletion(FQuest_Definition& Quest, const FVector& PlayerLocation);
    bool AreAllObjectivesComplete(const FQuest_Definition& Quest) const;

    UPROPERTY()
    TMap<FString, FQuest_Definition> QuestRegistry;

    // Trigger label → QuestID mapping
    TMap<FString, FString> TriggerToQuestMap;

    float ObjectiveTimer;   // Accumulated time for duration-based objectives
};
