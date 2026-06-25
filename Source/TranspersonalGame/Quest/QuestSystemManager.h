#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "QuestSystemManager.generated.h"

// ── Quest State Enum ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
};

// ── Quest Type Enum ───────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft"),
};

// ── Quest Objective Struct ────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted = false;
};

// ── Quest Data Struct ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Hunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestZoneLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestZoneRadius = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardXP = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RewardItems;
};

// ── Quest System Manager Actor ────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Quest Registry ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FQuest_Data> CompletedQuests;

    // ── Quest Operations ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ── Quest Initialization ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
    void PrintQuestStatus();

    // ── Events (Blueprint-implementable) ─────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Events")
    void OnQuestStarted(const FQuest_Data& Quest);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Events")
    void OnQuestCompleted(const FQuest_Data& Quest);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Events")
    void OnObjectiveCompleted(const FQuest_Data& Quest, const FQuest_Objective& Objective);

private:
    FQuest_Data* FindQuestByID(const FString& QuestID);
    void BuildDefaultQuestLibrary();
};
