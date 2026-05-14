#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Core_TerrainPhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Stability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SoundDampening = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanSink = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SinkRate = 0.0f;

    FCore_TerrainPhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        Stability = 1.0f;
        MovementSpeedMultiplier = 1.0f;
        SoundDampening = 0.5f;
        bCanSink = false;
        SinkRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_BiomeTerrainMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<ECore_TerrainType> PrimaryTerrainTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<ECore_TerrainType> SecondaryTerrainTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainVariation = 0.3f;

    FCore_BiomeTerrainMapping()
    {
        BiomeName = TEXT("Default");
        TerrainVariation = 0.3f;
    }
};

/**
 * Core Terrain Physics Manager
 * Manages realistic terrain physics properties for the prehistoric survival game.
 * Integrates with landscape system and provides terrain-aware physics simulation.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN PHYSICS PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, FCore_TerrainPhysicsProperties> TerrainProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TArray<FCore_BiomeTerrainMapping> BiomeTerrainMappings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float TerrainSampleRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    int32 TerrainSamplePoints = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableRealTimeUpdates = true;

    // === LANDSCAPE INTEGRATION ===

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    TWeakObjectPtr<ALandscape> CachedLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float LandscapeQueryRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    bool bUseLandscapePhysicalMaterials = true;

    // === PERFORMANCE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousQueries = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    // === PUBLIC INTERFACE ===

    /**
     * Get terrain type at a specific world location
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation);

    /**
     * Get terrain physics properties at a specific location
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainPropertiesAtLocation(const FVector& WorldLocation);

    /**
     * Sample terrain in an area around a location
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    TArray<ECore_TerrainType> SampleTerrainInArea(const FVector& CenterLocation, float Radius, int32 SampleCount = 9);

    /**
     * Apply terrain physics to an actor
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysicsToActor(AActor* Actor, const FVector& Location);

    /**
     * Get movement speed multiplier for terrain at location
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetMovementSpeedMultiplierAtLocation(const FVector& WorldLocation);

    /**
     * Check if terrain at location can cause sinking
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanSinkAtLocation(const FVector& WorldLocation, float& OutSinkRate);

    /**
     * Initialize terrain physics system
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    /**
     * Update terrain physics for all tracked actors
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics();

    /**
     * Register actor for terrain physics updates
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterActorForTerrainPhysics(AActor* Actor);

    /**
     * Unregister actor from terrain physics updates
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UnregisterActorFromTerrainPhysics(AActor* Actor);

    /**
     * Get biome terrain mapping for a specific biome
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    FCore_BiomeTerrainMapping GetBiomeTerrainMapping(const FString& BiomeName);

    /**
     * Set terrain properties for a terrain type
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainProperties(ECore_TerrainType TerrainType, const FCore_TerrainPhysicsProperties& Properties);

private:
    // === INTERNAL STATE ===

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedActors;

    UPROPERTY()
    TMap<AActor*, FCore_TerrainPhysicsProperties> ActorTerrainCache;

    float LastUpdateTime;
    int32 CurrentQueryCount;

    // === INTERNAL METHODS ===

    void InitializeDefaultTerrainProperties();
    void InitializeBiomeTerrainMappings();
    void FindLandscapeActor();
    ECore_TerrainType SampleTerrainAtPoint(const FVector& WorldLocation);
    FCore_TerrainPhysicsProperties GetDefaultPropertiesForTerrainType(ECore_TerrainType TerrainType);
    void UpdateActorTerrainPhysics(AActor* Actor);
    bool IsValidForTerrainPhysics(AActor* Actor);
    void CleanupInvalidActors();
};