#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "World_BiomeTransitionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTransitionZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float TransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EWorld_BiomeType PrimaryBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EWorld_BiomeType SecondaryBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float BlendStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    bool bIsActive;

    FWorld_BiomeTransitionZone()
    {
        ZoneName = TEXT("DefaultTransition");
        ZoneLocation = FVector::ZeroVector;
        TransitionRadius = 1000.0f;
        PrimaryBiome = EWorld_BiomeType::Temperate_Forest;
        SecondaryBiome = EWorld_BiomeType::Grasslands;
        BlendStrength = 0.5f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_EcosystemResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector ResourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EWorld_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float ResourceDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float RegenerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bIsRenewable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float InfluenceRadius;

    FWorld_EcosystemResource()
    {
        ResourceName = TEXT("DefaultResource");
        ResourceLocation = FVector::ZeroVector;
        ResourceType = EWorld_ResourceType::Water;
        ResourceDensity = 1.0f;
        RegenerationRate = 0.1f;
        bIsRenewable = true;
        InfluenceRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_MigrationPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FString PathName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EWorld_CreatureType CreatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float PathWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsSeasonalPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float UsageFrequency;

    FWorld_MigrationPath()
    {
        PathName = TEXT("DefaultPath");
        CreatureType = EWorld_CreatureType::Herbivore;
        PathWidth = 200.0f;
        bIsSeasonalPath = false;
        UsageFrequency = 1.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeTransitionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeTransitionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Transition Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transitions")
    TArray<FWorld_BiomeTransitionZone> TransitionZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Resources")
    TArray<FWorld_EcosystemResource> EcosystemResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Paths")
    TArray<FWorld_MigrationPath> MigrationPaths;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    void InitializeBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    float GetTransitionBlendAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    void CreateTransitionZone(const FString& ZoneName, const FVector& Location, EWorld_BiomeType Primary, EWorld_BiomeType Secondary, float Radius);

    // Resource Management
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnEcosystemResources();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    TArray<FWorld_EcosystemResource> GetResourcesInRadius(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RegenerateResources(float DeltaTime);

    // Migration Path Management
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void CreateMigrationPaths();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    FVector GetNearestPathPoint(const FVector& Location, EWorld_CreatureType CreatureType) const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool IsLocationOnMigrationPath(const FVector& Location, float Tolerance = 100.0f) const;

    // Validation and Debug
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateTransitionSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawTransitions();

private:
    // Internal state
    bool bIsInitialized;
    float ResourceUpdateTimer;
    float TransitionUpdateTimer;

    // Helper functions
    float CalculateDistanceToTransition(const FVector& Location, const FWorld_BiomeTransitionZone& Zone) const;
    void UpdateTransitionInfluence(float DeltaTime);
    void ValidateResourceDistribution();
};