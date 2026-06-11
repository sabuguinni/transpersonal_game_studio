#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_ObjectiveTracker.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    float CompletionRadius;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveDescription = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Hunt;
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ObjectiveTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ObjectiveTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void AddObjective(const FQuest_ObjectiveData& NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void RemoveObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    TArray<FQuest_ObjectiveData> GetAllObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    TArray<FQuest_ObjectiveData> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void CompleteObjective(const FString& ObjectiveID);

    // Location-based objectives
    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    // Hunt objectives
    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void OnDinosaurKilled(const FString& DinosaurType);

    // Gather objectives
    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    // Build objectives
    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void OnStructureBuilt(const FString& StructureType);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, const FString&, ObjectiveID);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdated, const FString&, ObjectiveID);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> CompletedObjectives;

    // Objective tracking settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float LocationCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bAutoCompleteLocationObjectives;

private:
    float LastLocationCheckTime;
    
    void CheckObjectiveCompletion(FQuest_ObjectiveData& Objective);
    void BroadcastObjectiveEvents(const FQuest_ObjectiveData& Objective);
};