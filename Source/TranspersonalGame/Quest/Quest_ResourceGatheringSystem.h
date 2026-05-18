#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "../Shared/SharedTypes.h"
#include "Quest_ResourceGatheringSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    float RespawnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
    AActor* NodeActor;

    FQuest_ResourceNode()
    {
        ResourceType = TEXT("Stone");
        Quantity = 1;
        Location = FVector::ZeroVector;
        bIsActive = true;
        RespawnTime = 300.0f;
        NodeActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_GatheringMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    FString TargetResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    int32 CurrentQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    TArray<FVector> HintLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gathering Mission")
    float RewardExperience;

    FQuest_GatheringMission()
    {
        MissionName = TEXT("Gather Resources");
        TargetResource = TEXT("Stone");
        RequiredQuantity = 5;
        CurrentQuantity = 0;
        TimeLimit = 600.0f;
        TimeRemaining = 600.0f;
        bIsActive = false;
        bIsCompleted = false;
        RewardExperience = 100.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Resource Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
    TArray<FQuest_ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
    TMap<FString, int32> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
    float ResourceScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
    int32 MaxResourcesPerBiome;

    // Mission Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_GatheringMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxConcurrentMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float LastMissionGenerationTime;

    // Resource Functions
    UFUNCTION(BlueprintCallable, Category = "Resource System")
    void SpawnResourceNodes();

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    void SpawnResourceNodeAtLocation(const FString& ResourceType, const FVector& Location, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    bool GatherResource(const FVector& Location, const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    int32 GetPlayerResourceCount(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    void AddToPlayerInventory(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    TArray<FQuest_ResourceNode> GetNearbyResources(const FVector& Location, float Radius);

    // Mission Functions
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void GenerateGatheringMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void StartMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateMissionProgress(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsMissionActive(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_GatheringMission> GetActiveMissions();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetRandomResourceType();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void CleanupExpiredMissions();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void RespawnResourceNode(int32 NodeIndex);

private:
    void UpdateMissionTimers(float DeltaTime);
    void UpdateResourceRespawn(float DeltaTime);
    void CheckMissionCompletion();
    FVector GetBiomeCenter(EBiomeType BiomeType);
    TArray<FString> GetAvailableResourceTypes();
};