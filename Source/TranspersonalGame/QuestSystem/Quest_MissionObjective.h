#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Quest_MissionObjective.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Dinosaur,
    Gather_Resources,
    Explore_Area,
    Survive_Time,
    Craft_Item,
    Reach_Location,
    Defend_Area,
    Rescue_NPC,
    Follow_Herd,
    Build_Shelter
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive,
    Active,
    Completed,
    Failed,
    Abandoned
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float ElapsedTime;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetCount = 1;
        CurrentCount = 0;
        TargetTag = TEXT("");
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
        bIsOptional = false;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionObjective : public UObject
{
    GENERATED_BODY()

public:
    UQuest_MissionObjective();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FQuest_ObjectiveData ObjectiveData;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void ActivateObjective();

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void CompleteObjective();

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void FailObjective();

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void UpdateProgress(int32 IncrementAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    bool IsCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    bool IsActive() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    float GetProgressPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void UpdateTimer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    bool IsTimeLimitExceeded() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void SetTargetLocation(const FVector& NewLocation, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    bool IsPlayerInTargetArea(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Objective")
    void ResetObjective();

protected:
    UFUNCTION()
    void OnObjectiveStatusChanged();
};