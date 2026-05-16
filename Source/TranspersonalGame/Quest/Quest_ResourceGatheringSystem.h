#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_ResourceGatheringSystem.generated.h"

// Forward declarations
class AStaticMeshActor;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Plant           UMETA(DisplayName = "Plant"),
    Water           UMETA(DisplayName = "Water"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Meat            UMETA(DisplayName = "Meat"),
    Fire            UMETA(DisplayName = "Fire")
};

UENUM(BlueprintType)
enum class EQuest_GatheringDifficulty : uint8
{
    Easy            UMETA(DisplayName = "Easy"),
    Medium          UMETA(DisplayName = "Medium"),
    Hard            UMETA(DisplayName = "Hard"),
    Dangerous       UMETA(DisplayName = "Dangerous")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    float RespawnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    EQuest_GatheringDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    float LastHarvestTime;

    FQuest_ResourceNode()
    {
        ResourceType = EQuest_ResourceType::Stone;
        Location = FVector::ZeroVector;
        Quantity = 1;
        RespawnTime = 300.0f; // 5 minutes
        Difficulty = EQuest_GatheringDifficulty::Easy;
        bIsActive = true;
        LastHarvestTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_GatheringMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    TMap<EQuest_ResourceType, int32> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    FVector TargetArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float AreaRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float StartTime;

    FQuest_GatheringMission()
    {
        MissionName = TEXT("Gather Resources");
        TimeLimit = 600.0f; // 10 minutes
        TargetArea = FVector::ZeroVector;
        AreaRadius = 5000.0f;
        bIsCompleted = false;
        StartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Resource node management
    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void SpawnResourceNode(EQuest_ResourceType ResourceType, FVector Location, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void RemoveResourceNode(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    bool HarvestResource(FVector Location, int32& OutQuantity, EQuest_ResourceType& OutResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    TArray<FQuest_ResourceNode> GetNearbyResources(FVector PlayerLocation, float SearchRadius = 1000.0f);

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void StartGatheringMission(const FQuest_GatheringMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    bool CheckMissionProgress(const FString& MissionName, TMap<EQuest_ResourceType, int32>& OutProgress);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void CompleteMission(const FString& MissionName);

    // Resource spawning for biomes
    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void PopulateBiomeWithResources(EBiomeType BiomeType, FVector BiomeCenter, float BiomeRadius);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    FString GetResourceTypeName(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Gathering")
    void UpdateResourceNodes(float DeltaTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    TArray<FQuest_ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    TArray<FQuest_GatheringMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    TMap<EQuest_ResourceType, int32> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gathering")
    float ResourceRespawnMultiplier;

private:
    void CreateDefaultResourceNodes();
    void SpawnResourceActor(const FQuest_ResourceNode& ResourceNode);
    FQuest_ResourceNode* FindResourceNodeAtLocation(FVector Location);
    void CheckMissionTimeouts();
};