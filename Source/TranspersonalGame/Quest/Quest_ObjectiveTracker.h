#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest_ObjectiveTracker.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Kill_Dinosaur      UMETA(DisplayName = "Kill Dinosaur"),
    Gather_Resource    UMETA(DisplayName = "Gather Resource"),
    Explore_Area       UMETA(DisplayName = "Explore Area"),
    Craft_Item         UMETA(DisplayName = "Craft Item"),
    Survive_Duration   UMETA(DisplayName = "Survive Duration"),
    Reach_Location     UMETA(DisplayName = "Reach Location"),
    Interact_NPC       UMETA(DisplayName = "Interact with NPC"),
    Build_Structure    UMETA(DisplayName = "Build Structure")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive          UMETA(DisplayName = "Inactive"),
    Active            UMETA(DisplayName = "Active"),
    Completed         UMETA(DisplayName = "Completed"),
    Failed            UMETA(DisplayName = "Failed"),
    Optional          UMETA(DisplayName = "Optional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsHidden;

    FQuest_Objective()
    {
        ObjectiveID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Kill_Dinosaur;
        Status = EQuest_ObjectiveStatus::Inactive;
        RequiredCount = 1;
        CurrentCount = 0;
        TargetName = TEXT("");
        TargetLocation = FVector::ZeroVector;
        bIsOptional = false;
        bIsHidden = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ProgressEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    EQuest_ObjectiveType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 ProgressAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    FVector EventLocation;

    FQuest_ProgressEvent()
    {
        ObjectiveID = TEXT("");
        EventType = EQuest_ObjectiveType::Kill_Dinosaur;
        TargetName = TEXT("");
        ProgressAmount = 1;
        EventLocation = FVector::ZeroVector;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_ObjectiveUpdated, const FQuest_Objective&, UpdatedObjective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_ObjectiveCompleted, const FQuest_Objective&, CompletedObjective);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ObjectiveTracker : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ObjectiveTracker();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void AddObjective(const FQuest_Objective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void RemoveObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void ActivateObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void FailObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UpdateObjectiveProgress(const FQuest_ProgressEvent& ProgressEvent);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FQuest_Objective GetObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_Objective> GetActiveObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_Objective> GetCompletedObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveComplete(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    int32 GetObjectiveProgress(const FString& ObjectiveID);

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_ObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_ObjectiveCompleted OnObjectiveCompleted;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest Objectives")
    TMap<FString, FQuest_Objective> Objectives;

private:
    void InitializeDefaultObjectives();
    bool MatchesProgressEvent(const FQuest_Objective& Objective, const FQuest_ProgressEvent& Event);
};