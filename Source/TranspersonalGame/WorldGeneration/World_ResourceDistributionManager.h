#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_ResourceDistributionManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_ResourceType : uint8
{
    None = 0,
    Iron,
    Stone,
    Flint,
    Clay,
    Salt,
    Obsidian,
    Wood,
    Plant_Fiber,
    Water,
    Food_Berries,
    Food_Nuts,
    Food_Roots
};

UENUM(BlueprintType)
enum class EWorld_ResourceQuality : uint8
{
    Poor = 0,
    Common,
    Good,
    Excellent,
    Rare
};

UENUM(BlueprintType)
enum class EWorld_ResourceDensity : uint8
{
    Sparse = 0,
    Low,
    Medium,
    High,
    Dense
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EWorld_ResourceType ResourceType = EWorld_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EWorld_ResourceQuality Quality = EWorld_ResourceQuality::Common;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 MaxYield = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 CurrentYield = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RegenerationRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsRenewable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bRequiresTools = false;

    FWorld_ResourceNode()
    {
        ResourceType = EWorld_ResourceType::None;
        Quality = EWorld_ResourceQuality::Common;
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        MaxYield = 100;
        CurrentYield = 100;
        RegenerationRate = 1.0f;
        bIsRenewable = true;
        bRequiresTools = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeResourceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    EBiomeType BiomeType = EBiomeType::Temperate_Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    TArray<EWorld_ResourceType> PrimaryResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    TArray<EWorld_ResourceType> SecondaryResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    TArray<EWorld_ResourceType> RareResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    EWorld_ResourceDensity ResourceDensity = EWorld_ResourceDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    float WaterAvailability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Resources")
    float ShelterAvailability = 0.3f;

    FWorld_BiomeResourceProfile()
    {
        BiomeType = EBiomeType::Temperate_Forest;
        ResourceDensity = EWorld_ResourceDensity::Medium;
        WaterAvailability = 0.5f;
        ShelterAvailability = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ResourceDistributionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ResourceDistributionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Resource Distribution
    UFUNCTION(BlueprintCallable, Category = "Resource Distribution")
    void GenerateResourceNodes();

    UFUNCTION(BlueprintCallable, Category = "Resource Distribution")
    void GenerateBiomeResources(EBiomeType BiomeType, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Resource Distribution")
    TArray<FWorld_ResourceNode> GetResourcesInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Resource Distribution")
    TArray<FWorld_ResourceNode> GetResourcesByType(EWorld_ResourceType ResourceType);

    // Resource Management
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool HarvestResource(int32 NodeIndex, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void RegenerateResources(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    int32 GetNearestResourceNode(FVector Location, EWorld_ResourceType ResourceType);

    // Water Sources
    UFUNCTION(BlueprintCallable, Category = "Water Sources")
    void GenerateWaterSources();

    UFUNCTION(BlueprintCallable, Category = "Water Sources")
    TArray<FVector> GetNearbyWaterSources(FVector Location, float SearchRadius);

    // Shelter Locations
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void GenerateShelterLocations();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetNearbyShelters(FVector Location, float SearchRadius);

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void SetupBiomeResourceProfiles();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    FWorld_BiomeResourceProfile GetBiomeResourceProfile(EBiomeType BiomeType);

    // Performance Integration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeResourceDistribution(int32 MaxActiveNodes);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateResourceLOD(FVector PlayerLocation);

protected:
    // Resource Storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Distribution")
    TArray<FWorld_ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Distribution")
    TArray<FVector> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Distribution")
    TArray<FVector> ShelterLocations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Resources")
    TArray<FWorld_BiomeResourceProfile> BiomeResourceProfiles;

    // Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxResourceNodes = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float MinResourceDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float ResourceClusterRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxWaterSources = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxShelterLocations = 30;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ResourceUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxResourceDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveResourceNodes = 100;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float LastResourceUpdate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    int32 ActiveResourceNodes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bResourceSystemInitialized = false;

private:
    // Internal Methods
    void InitializeDefaultBiomeProfiles();
    FVector FindValidResourceLocation(EWorld_ResourceType ResourceType, FVector PreferredLocation, float SearchRadius);
    bool IsLocationValidForResource(FVector Location, EWorld_ResourceType ResourceType);
    void CreateResourceCluster(EWorld_ResourceType ResourceType, FVector CenterLocation, int32 NodeCount);
    float CalculateResourceQualityMultiplier(EWorld_ResourceQuality Quality);
    void UpdateResourceVisibility(FVector PlayerLocation);
};

#include "World_ResourceDistributionManager.generated.h"