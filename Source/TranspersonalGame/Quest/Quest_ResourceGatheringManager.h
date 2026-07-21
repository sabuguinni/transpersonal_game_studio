#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Quest_ResourceGatheringManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Plant       UMETA(DisplayName = "Plant"),
    Water       UMETA(DisplayName = "Water"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide")
};

USTRUCT(BlueprintType)
struct FQuest_ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    FQuest_ResourceNode()
    {
        ResourceType = EQuest_ResourceType::Stone;
        Location = FVector::ZeroVector;
        Quantity = 1;
        bIsActive = true;
        RespawnTime = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_GatheringObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ResourceType TargetResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveText;

    FQuest_GatheringObjective()
    {
        TargetResource = EQuest_ResourceType::Stone;
        RequiredAmount = 5;
        CurrentAmount = 0;
        ObjectiveText = TEXT("Gather resources");
    }
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourceGatheringManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourceGatheringManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_GatheringObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveNodes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartGatheringQuest(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteGatheringQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool GatherResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SpawnResourceNodes();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FVector> GetNearbyResourceLocations(EQuest_ResourceType ResourceType, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestComplete();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FString GetQuestProgressText();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RespawnResourceNode(int32 NodeIndex);

private:
    void InitializeResourceNodes();
    void CheckPlayerProximity();
    FVector GetRandomResourceLocation();
    bool IsLocationValid(FVector Location);
};