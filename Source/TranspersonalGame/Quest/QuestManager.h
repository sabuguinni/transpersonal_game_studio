#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — World subsystem managing all active quests
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Escort      UMETA(DisplayName = "Escort"),
    Defend      UMETA(DisplayName = "Defend"),
    Scout       UMETA(DisplayName = "Scout"),
    Craft       UMETA(DisplayName = "Craft"),
    Survive     UMETA(DisplayName = "Survive")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ProtectTarget   UMETA(DisplayName = "Protect Target"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Time")
};

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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float HealthRestorePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipeID;

    FQuest_Reward()
        : ResourceBonus(0)
        , HealthRestorePercent(0.0f)
        , UnlockedRecipeID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Hunt)
        , Status(EQuest_Status::Locked)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
    {}
};

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_Data& OutData) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Threat integration — called by CrowdSimulationManager on dino threat
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinoThreatDetected(const FString& DinoActorLabel, const FVector& ThreatLocation);

    // Tick for time-limited quests
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

private:
    void RegisterDefaultQuests();
    void CheckQuestCompletion(const FString& QuestID);
    void GrantReward(const FQuest_Reward& Reward);

    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    float ElapsedTime;
};
