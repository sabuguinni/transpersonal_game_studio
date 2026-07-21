// QuestManager.h — Performance Optimizer #04 — PROD_CYCLE_AUTO_20260702_002
// Quest management system: tutorial "Find Water" quest + quest state machine
// Dinosaur survival game — no spiritual/mystical content
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// --- Quest state enum ---
UENUM(BlueprintType)
enum class EPerf_QuestState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// --- Quest objective struct ---
USTRUCT(BlueprintType)
struct FPerf_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius = 300.0f;
};

// --- Quest data struct ---
USTRUCT(BlueprintType)
struct FPerf_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EPerf_QuestState State = EPerf_QuestState::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FPerf_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentObjectiveIndex = 0;
};

// --- QuestManager Actor ---
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Quest registry ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FPerf_QuestData> AllQuests;

    // --- Active quest tracking ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    FString ActiveQuestID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    int32 ActiveQuestIndex = -1;

    // --- Tutorial quest config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Tutorial")
    float WaterSourceCheckRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Tutorial")
    FVector WaterSourceLocation = FVector(8000.0f, 0.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Tutorial")
    float ObjectiveCheckInterval = 0.5f;

    // --- Quest functions ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeTutorialQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCurrentObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EPerf_QuestState GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FString GetCurrentObjectiveText() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetDistanceToCurrentObjective() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool HasActiveQuest() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FPerf_QuestData GetActiveQuestData() const;

    // --- Delegate for UI ---
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, FString, QuestID, EPerf_QuestState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStateChanged OnQuestStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdated, FString, ObjectiveText);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    float ObjectiveCheckTimer = 0.0f;

    void CheckObjectiveProximity();
    int32 FindQuestIndex(const FString& QuestID) const;
    APawn* GetPlayerPawn() const;
};
