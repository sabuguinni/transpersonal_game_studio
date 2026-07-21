#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerBox.h"
#include "Quest_ObjectiveManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Target,
    Gather_Resource,
    Build_Structure,
    Tame_Creature,
    Explore_Area,
    Survive_Duration
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
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Title;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQuest_ObjectiveType Type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQuest_ObjectiveStatus Status;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 TargetCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float TargetRadius;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Title = "";
        Description = "";
        Type = EQuest_ObjectiveType::Hunt_Target;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_ObjectiveManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ObjectiveManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateObjective(const FString& ObjectiveID, const FString& Title, const FString& Description, EQuest_ObjectiveType Type, int32 TargetCount, FVector TargetLocation, float TargetRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ActivateObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_ObjectiveData> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationObjectives(FVector PlayerLocation);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_ObjectiveData> Objectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveObjectiveIDs;

private:
    void InitializeDefaultObjectives();
    void BroadcastObjectiveUpdate(const FString& ObjectiveID);
};

#include "Quest_ObjectiveManager.generated.h"