#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Quest_MissionObjective.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt Target"),
    Gather      UMETA(DisplayName = "Gather Resources"),
    Explore     UMETA(DisplayName = "Explore Location"),
    Defend      UMETA(DisplayName = "Defend Area"),
    Escort      UMETA(DisplayName = "Escort NPC"),
    Craft       UMETA(DisplayName = "Craft Item"),
    Survive     UMETA(DisplayName = "Survive Duration")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText ObjectiveTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetRadius;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveTitle = FText::FromString(TEXT(""));
        ObjectiveDescription = FText::FromString(TEXT(""));
        ObjectiveType = EQuest_ObjectiveType::Hunt;
        TargetTag = TEXT("");
        RequiredCount = 1;
        TimeLimit = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionObjective : public UObject
{
    GENERATED_BODY()

public:
    UQuest_MissionObjective();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FQuest_ObjectiveData ObjectiveData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveStatus CurrentStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    void ActivateObjective();

    UFUNCTION(BlueprintCallable, Category = "Objective")
    void UpdateProgress(int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Objective")
    void CompleteObjective();

    UFUNCTION(BlueprintCallable, Category = "Objective")
    void FailObjective();

    UFUNCTION(BlueprintCallable, Category = "Objective")
    bool IsCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    bool IsFailed() const;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    bool IsActive() const;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    float GetProgressPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    float GetRemainingTime() const;

    UFUNCTION(BlueprintCallable, Category = "Objective")
    bool IsTimedOut() const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveStatusChanged, UQuest_MissionObjective*, Objective);
    UPROPERTY(BlueprintAssignable, Category = "Objective")
    FOnObjectiveStatusChanged OnObjectiveStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressUpdated, UQuest_MissionObjective*, Objective, float, ProgressPercentage);
    UPROPERTY(BlueprintAssignable, Category = "Objective")
    FOnObjectiveProgressUpdated OnObjectiveProgressUpdated;
};

#include "Quest_MissionObjective.generated.h"