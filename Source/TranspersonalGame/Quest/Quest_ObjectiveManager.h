#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_ObjectiveManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt Target"),
    Gather      UMETA(DisplayName = "Gather Resources"),
    Explore     UMETA(DisplayName = "Explore Area"),
    Defend      UMETA(DisplayName = "Defend Location"),
    Escort      UMETA(DisplayName = "Escort NPC"),
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
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorLabel;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        ObjectiveName = "";
        Description = "";
        ObjectiveType = EQuest_ObjectiveType::Hunt;
        Status = EQuest_ObjectiveStatus::Inactive;
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        TargetActorLabel = "";
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<ATriggerBox*> ObjectiveTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    bool bDebugObjectives;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CreateObjective(const FString& ObjectiveID, const FString& Name, const FString& Description, 
                        EQuest_ObjectiveType Type, const FVector& Location, int32 RequiredCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void ActivateObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void FailObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveActive(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckHuntObjectives(const FString& KilledActorLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CheckGatherObjectives(const FString& GatheredItem, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void SetupDefaultObjectives();

private:
    void CheckObjectiveCompletion();
    void UpdateObjectiveTriggers();
    FQuest_ObjectiveData* FindObjectiveByID(const FString& ObjectiveID);
};