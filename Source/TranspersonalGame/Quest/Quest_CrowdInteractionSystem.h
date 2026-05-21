#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Quest_CrowdInteractionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    FString InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    FVector TargetLocation;

    FQuest_CrowdInteractionData()
    {
        InteractionType = TEXT("Observe");
        RequiredCrowdSize = 5;
        InteractionRadius = 1000.0f;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CrowdInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core crowd interaction functionality
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    void InitializeCrowdQuest(const FString& QuestType, int32 CrowdSize, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    bool CheckCrowdInteraction(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    void CompleteCrowdInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    void UpdateCrowdObjective(const FString& NewObjective);

    // Crowd behavior analysis
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    int32 CountNearbyDinosaurs(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    bool ValidateHerdBehavior(const FVector& HerdCenter);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    void TrackMigrationPattern(const FVector& StartPoint, const FVector& EndPoint);

    // Quest progression
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    float GetInteractionProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Interaction")
    FString GetCurrentObjective() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    FQuest_CrowdInteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float CompletionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    float CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    FString CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    TArray<AActor*> TrackedCrowdActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    FVector LastKnownHerdLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    float InteractionTimer;

private:
    void UpdateQuestProgress(float DeltaTime);
    void CheckProximityToHerd();
    void AnalyzeCrowdBehavior();
    bool ValidateQuestCompletion();
};

UCLASS()
class TRANSPERSONALGAME_API AQuest_CrowdInteractionMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdInteractionMarker();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EBiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsActive;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void ActivateQuestMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void DeactivateQuestMarker();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnQuestActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnQuestCompleted();
};