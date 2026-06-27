#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestSystemManager.generated.h"

// ─── Quest state enum ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// ─── Quest objective struct ───────────────────────────────────────────────────
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;
};

// ─── Quest data struct ────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardXP = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;
};

// ─── Quest System Manager Actor ───────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest lifecycle ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ── Quest registry ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    int32 ActiveQuestCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    int32 CompletedQuestCount = 0;

private:
    void InitializeDefaultQuests();
    void CheckQuestCompletion(FQuest_Data& Quest);
};
