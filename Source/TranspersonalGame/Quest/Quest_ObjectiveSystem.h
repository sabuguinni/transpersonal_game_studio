#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Quest_ObjectiveSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Kill        UMETA(DisplayName = "Kill Target"),
    Collect     UMETA(DisplayName = "Collect Items"),
    Reach       UMETA(DisplayName = "Reach Location"),
    Interact    UMETA(DisplayName = "Interact With Object"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Craft       UMETA(DisplayName = "Craft Items")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float InteractionRadius;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Kill;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetTag = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        InteractionRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ObjectiveComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FQuest_ObjectiveData ObjectiveData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bAutoActivateOnBeginPlay;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void ActivateObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void CompleteObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void FailObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    void UpdateProgress(int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    bool IsObjectiveComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objective")
    float GetCompletionPercentage() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnActorKilled(AActor* KilledActor, AActor* Killer);

    UFUNCTION()
    void OnItemCollected(const FString& ItemTag, int32 Amount);

    UFUNCTION()
    void OnLocationReached(const FVector& Location, AActor* ReachingActor);

private:
    void BindToGameEvents();
    void CheckLocationObjective();
    void CheckInteractionObjective();
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_ObjectiveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Objectives")
    TArray<UQuest_ObjectiveComponent*> ActiveObjectives;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void RegisterObjective(UQuest_ObjectiveComponent* Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UnregisterObjective(UQuest_ObjectiveComponent* Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<UQuest_ObjectiveComponent*> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    UQuest_ObjectiveComponent* FindObjectiveByID(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void NotifyActorKilled(AActor* KilledActor, AActor* Killer);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void NotifyItemCollected(const FString& ItemTag, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void NotifyLocationReached(const FVector& Location, AActor* ReachingActor);

private:
    void ProcessKillObjectives(AActor* KilledActor, AActor* Killer);
    void ProcessCollectionObjectives(const FString& ItemTag, int32 Amount);
    void ProcessLocationObjectives(const FVector& Location, AActor* ReachingActor);
};

#include "Quest_ObjectiveSystem.generated.h"