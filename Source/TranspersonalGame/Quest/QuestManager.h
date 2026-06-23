#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Core quest system for prehistoric survival
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Survive     UMETA(DisplayName = "Survive"),
    Explore     UMETA(DisplayName = "Explore"),
    Defend      UMETA(DisplayName = "Defend"),
    Follow      UMETA(DisplayName = "Follow")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "InProgress"),
    Done        UMETA(DisplayName = "Done"),
    Skipped     UMETA(DisplayName = "Skipped")
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
    EQuest_ObjectiveStatus Status = EQuest_ObjectiveStatus::Pending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector ObjectiveLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius = 300.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Gather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;  // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bFailOnHerdPanic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString NPCGiverLabel;
};

USTRUCT(BlueprintType)
struct FQuest_Inventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Count = 0;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Registry ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedQuestIDs;

    // ---- Player Inventory (for gather quests) ----
    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    TArray<FQuest_Inventory> PlayerInventory;

    // ---- Quest Control ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AddToInventory(const FString& ItemID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetInventoryCount(const FString& ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestDefinition(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    // ---- Herd Panic Integration (from CrowdSimulationManager) ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnHerdPanic(const FString& HerdID);

    // ---- Player Proximity Check ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void CheckPlayerProximityToObjectives(const FVector& PlayerLocation);

    // ---- Built-in Quest Definitions ----
    UFUNCTION(CallInEditor, Category = "Quest|Setup")
    void RegisterDefaultQuests();

private:
    void InitializeDefaultQuests();
    void TickActiveQuests(float DeltaTime);
    bool AreAllObjectivesDone(const FQuest_Definition& Quest) const;
    int32 FindQuestIndex(const FString& QuestID) const;
};
