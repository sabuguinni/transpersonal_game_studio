#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_ObjectiveSystem.generated.h"

// Forward declarations
class UQuest_QuestManager;
class AActor;

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resource     UMETA(DisplayName = "Gather Resource"),
    Explore_Location    UMETA(DisplayName = "Explore Location"),
    Craft_Item          UMETA(DisplayName = "Craft Item"),
    Survive_Duration    UMETA(DisplayName = "Survive Duration"),
    Reach_Location      UMETA(DisplayName = "Reach Location"),
    Interact_NPC        UMETA(DisplayName = "Interact with NPC"),
    Defend_Area         UMETA(DisplayName = "Defend Area")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Optional        UMETA(DisplayName = "Optional")
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
    EQuest_ObjectiveType Type;

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
    bool bTrackProgress;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        Type = EQuest_ObjectiveType::Hunt_Dinosaur;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetCount = 1;
        CurrentCount = 0;
        TargetTag = TEXT("");
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
        bIsOptional = false;
        bTrackProgress = true;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ObjectiveSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ObjectiveSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CreateObjective(const FQuest_ObjectiveData& ObjectiveData);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void FailObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveComplete(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> GetActiveObjectives() const;

    // Event Handlers
    UFUNCTION(BlueprintCallable, Category = "Quest Events")
    void OnDinosaurKilled(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Events")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Events")
    void OnLocationReached(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest Events")
    void OnItemCrafted(const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest Events")
    void OnNPCInteraction(const FString& NPCID);

    // Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Quest Progress")
    float GetObjectiveProgress(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Progress")
    FString GetObjectiveProgressText(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Progress")
    void SetObjectiveTrackingEnabled(const FString& ObjectiveID, bool bEnabled);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Objectives")
    TMap<FString, FQuest_ObjectiveData> ActiveObjectives;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Objectives")
    TArray<FString> CompletedObjectives;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Objectives")
    TArray<FString> FailedObjectives;

    // Helper functions
    void CheckLocationObjectives(const FVector& PlayerLocation);
    void NotifyObjectiveComplete(const FString& ObjectiveID);
    void NotifyObjectiveFailed(const FString& ObjectiveID);
    bool IsPlayerInRange(const FVector& TargetLocation, float Radius) const;

private:
    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;

    float LocationCheckTimer;
    static constexpr float LocationCheckInterval = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FString, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveFailed, FString, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressUpdated, FString, ObjectiveID, float, Progress);