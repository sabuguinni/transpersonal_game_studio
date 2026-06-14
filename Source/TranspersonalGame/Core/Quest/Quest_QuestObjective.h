#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SharedTypes.h"
#include "Quest_QuestObjective.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget,
    CollectItems,
    ReachLocation,
    InteractWithNPC,
    SurviveTime,
    CraftItem,
    ExploreArea
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive,
    Active,
    Completed,
    Failed
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bIsOptional;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::KillTarget;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetID = TEXT("");
        RequiredAmount = 1;
        CurrentAmount = 0;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
        bIsOptional = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestObjective : public UObject
{
    GENERATED_BODY()

public:
    UQuest_QuestObjective();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FQuest_ObjectiveData ObjectiveData;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void UpdateProgress(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void SetStatus(EQuest_ObjectiveStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool IsCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool IsFailed() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    float GetCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool CheckLocationObjective(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void ResetObjective();

private:
    void CheckCompletion();
};