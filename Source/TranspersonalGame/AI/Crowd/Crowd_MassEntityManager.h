#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_DensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low Density"),
    Medium      UMETA(DisplayName = "Medium Density"),
    High        UMETA(DisplayName = "High Density"),
    Critical    UMETA(DisplayName = "Critical Density")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECrowd_DensityLevel DensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxCrowdEntities;

    FCrowd_BiomeData()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        DensityLevel = ECrowd_DensityLevel::Medium;
        MaxCrowdEntities = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float AvoidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    bool bIsActive;

    FCrowd_EntityData()
    {
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        Speed = 300.0f;
        AvoidanceRadius = 150.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core crowd management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    int32 MaxTotalEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float LODDistance3;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FCrowd_BiomeData> BiomeConfigurations;

    // Entity management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entities")
    TArray<FCrowd_EntityData> ActiveEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float WaypointReachDistance;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 EntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseOcclusion;

public:
    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntitiesInBiome(const FString& BiomeName, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdEntities(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void ClearAllCrowdEntities();

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterBiome(const FCrowd_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FCrowd_BiomeData GetBiomeByName(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FString GetBiomeAtLocation(const FVector& Location);

    // Entity queries
    UFUNCTION(BlueprintCallable, Category = "Entity Queries")
    int32 GetActiveEntityCount();

    UFUNCTION(BlueprintCallable, Category = "Entity Queries")
    TArray<FCrowd_EntityData> GetEntitiesInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Entity Queries")
    bool IsLocationCrowded(const FVector& Location, float CheckRadius);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEntities(const FVector& ViewerLocation, float MaxDistance);

protected:
    // Internal methods
    void SetupDefaultBiomes();
    void UpdateEntityMovement(FCrowd_EntityData& Entity, float DeltaTime);
    FVector CalculateAvoidanceForce(const FCrowd_EntityData& Entity);
    FVector FindNearestWaypoint(const FVector& Location);
    bool IsWithinBiomeBounds(const FVector& Location, const FCrowd_BiomeData& Biome);

private:
    float LastUpdateTime;
    int32 CurrentEntityIndex;
    bool bSystemInitialized;
};