#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Quest_ObjectiveManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Dinosaur,
    Explore_Location,
    Gather_Resources,
    Craft_Item,
    Survive_Time,
    Reach_Location
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    TArray<FString> RequiredItems;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        ObjectiveTitle = "";
        ObjectiveDescription = "";
        ObjectiveType = EQuest_ObjectiveType::Reach_Location;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
        RequiredCount = 1;
        CurrentCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ObjectiveMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float ObjectiveCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bAutoActivateObjectives;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void AddObjective(const FQuest_ObjectiveData& NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void ActivateObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void FailObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveActive(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckHuntingObjectives(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckGatheringObjectives(const FString& ItemType, int32 Amount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveActivated(const FQuest_ObjectiveData& Objective);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveCompleted(const FQuest_ObjectiveData& Objective);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveFailed(const FQuest_ObjectiveData& Objective);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveProgressUpdated(const FQuest_ObjectiveData& Objective);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

private:
    void InitializeDefaultObjectives();
    void UpdateObjectiveMarkers();
    FQuest_ObjectiveData* FindObjectiveByID(const FString& ObjectiveID);
};

#include "Quest_ObjectiveManager.generated.h"