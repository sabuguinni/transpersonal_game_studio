#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Quest_QuestObjective.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget,
    CollectItem,
    ReachLocation,
    TalkToNPC,
    CraftItem,
    SurviveTime
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bCompleted;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        Type = EQuest_ObjectiveType::CollectItem;
        TargetID = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
        bCompleted = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_QuestObjective : public UDataAsset
{
    GENERATED_BODY()

public:
    UQuest_QuestObjective();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FQuest_ObjectiveData ObjectiveData;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool UpdateProgress(int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool CheckLocationProgress(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool IsCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    float GetCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void ResetObjective();
};

#include "Quest_QuestObjective.generated.h"