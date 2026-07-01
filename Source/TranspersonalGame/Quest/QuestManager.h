#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_QuestState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EPerf_QuestType : uint8
{
    Tutorial    UMETA(DisplayName = "Tutorial"),
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct FPerf_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ProximityRadius;

    FPerf_QuestObjective()
        : ObjectiveText(TEXT(""))
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , ProximityRadius(500.0f)
    {}
};

USTRUCT(BlueprintType)
struct FPerf_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EPerf_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EPerf_QuestState QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FPerf_QuestObjective> Objectives;

    FPerf_QuestData()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EPerf_QuestType::Tutorial)
        , QuestState(EPerf_QuestState::Inactive)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Active quests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FPerf_QuestData> ActiveQuests;

    // Completed quests
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FPerf_QuestData> CompletedQuests;

    // Current objective text for HUD
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    FString CurrentObjectiveText;

    // Tutorial quest: Find Water
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartTutorialQuest_FindWater();

    // Activate a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    // Complete an objective
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, int32 ObjectiveIndex);

    // Check proximity to quest targets
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckQuestProximity(FVector PlayerLocation);

    // Get active quest count
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Water source location for tutorial
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Tutorial")
    FVector TutorialWaterSourceLocation;

private:
    void UpdateCurrentObjectiveText();
    float ProximityCheckInterval;
    float TimeSinceLastProximityCheck;
};
