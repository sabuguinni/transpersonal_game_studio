#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_ResourceGatheringSystem.generated.h"

// Resource types for gathering quests
UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Berry       UMETA(DisplayName = "Berry"),
    Clay        UMETA(DisplayName = "Clay"),
    Flint       UMETA(DisplayName = "Flint"),
    Hide        UMETA(DisplayName = "Hide"),
    Bone        UMETA(DisplayName = "Bone"),
    Fiber       UMETA(DisplayName = "Fiber")
};

// Resource gathering quest data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceGatheringData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    float GatherRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    FVector ResourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quest")
    bool bIsCompleted;

    FQuest_ResourceGatheringData()
    {
        ResourceType = EQuest_ResourceType::Stone;
        RequiredAmount = 5;
        CurrentAmount = 0;
        GatherRadius = 200.0f;
        ResourceLocation = FVector::ZeroVector;
        bIsCompleted = false;
    }
};

// Resource node actor for world placement
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourceNode : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourceNode();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    int32 MaxResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    int32 CurrentResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    float RegenerationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    bool bCanRegenerate;

public:
    UFUNCTION(BlueprintCallable, Category = "Resource Node")
    bool CanGatherResource() const;

    UFUNCTION(BlueprintCallable, Category = "Resource Node")
    int32 GatherResource(int32 AmountToGather = 1);

    UFUNCTION(BlueprintCallable, Category = "Resource Node")
    void RegenerateResources();

private:
    FTimerHandle RegenerationTimer;
};

// Resource gathering quest system
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    TArray<FQuest_ResourceGatheringData> ActiveGatheringQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    TArray<AQuest_ResourceNode*> DiscoveredResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    float GatheringTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    bool bIsGathering;

public:
    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void StartResourceGatheringQuest(EQuest_ResourceType ResourceType, int32 RequiredAmount, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    bool CompleteResourceGatheringQuest(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void UpdateResourceProgress(EQuest_ResourceType ResourceType, int32 AmountGathered);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    TArray<AQuest_ResourceNode*> FindNearbyResourceNodes(EQuest_ResourceType ResourceType, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    bool StartGathering(AQuest_ResourceNode* TargetNode);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void StopGathering();

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    FQuest_ResourceGatheringData* GetActiveQuest(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    int32 GetTotalResourcesGathered(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    float GetGatheringProgress() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Gathering")
    void OnResourceGathered(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Gathering")
    void OnQuestCompleted(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Gathering")
    void OnGatheringStarted(AQuest_ResourceNode* Node);

private:
    void ScanForResourceNodes();
    void ProcessGathering(float DeltaTime);
    
    AQuest_ResourceNode* CurrentGatheringNode;
    float GatheringProgress;
    FTimerHandle GatheringTimer;
};