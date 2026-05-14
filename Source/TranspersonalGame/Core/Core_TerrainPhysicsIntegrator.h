#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeProxy.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Eng_BiomeArchitecture.h"
#include "Core_TerrainPhysicsIntegrator.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Grass UMETA(DisplayName = "Grass"),
    Dirt UMETA(DisplayName = "Dirt"), 
    Rock UMETA(DisplayName = "Rock"),
    Sand UMETA(DisplayName = "Sand"),
    Mud UMETA(DisplayName = "Mud"),
    Snow UMETA(DisplayName = "Snow"),
    Water UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Restitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Density = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float WalkSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float StabilityFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCanSupportHeavyObjects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SinkDepth = 0.0f;

    FCore_TerrainPhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.1f;
        Density = 2.0f;
        WalkSpeedMultiplier = 1.0f;
        StabilityFactor = 1.0f;
        bCanSupportHeavyObjects = true;
        SinkDepth = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_TerrainInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    AActor* InteractingActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    FVector ContactPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    float ContactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    float ContactTime = 0.0f;

    FCore_TerrainInteractionData()
    {
        InteractingActor = nullptr;
        ContactPoint = FVector::ZeroVector;
        TerrainType = ECore_TerrainType::Grass;
        ContactForce = 0.0f;
        ContactTime = 0.0f;
    }
};

/**
 * Core Terrain Physics Integrator - Manages realistic terrain physics interactions
 * Handles terrain material properties, character movement on different surfaces,
 * and physics-based environmental interactions for the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysicsIntegrator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsIntegrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Terrain physics setup
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetupLandscapePhysics(ALandscapeProxy* Landscape);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysicsProperties(UStaticMeshComponent* TerrainComponent, ECore_TerrainType TerrainType);

    // Terrain type detection and management
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainPropertiesAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainTypeForBiome(EBiomeType BiomeType, ECore_TerrainType TerrainType);

    // Character movement integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetMovementSpeedMultiplier(AActor* Character, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanCharacterStandStably(AActor* Character, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainEffectsToCharacter(AActor* Character, const FVector& Location);

    // Physics object interaction with terrain
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterTerrainInteraction(const FCore_TerrainInteractionData& InteractionData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void HandleObjectTerrainCollision(AActor* Object, const FVector& ImpactPoint, float ImpactForce);

    // Terrain deformation and destruction
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateTerrainDeformation(const FVector& Location, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SimulateTerrainErosion(const FVector& Location, float Intensity);

    // Environmental physics effects
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyWeatherEffectsToTerrain(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainMoisture(float DeltaTime);

    // Validation and testing
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ValidateTerrainPhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateTerrainPhysicsTestScenario();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void OptimizeTerrainPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    int32 GetActiveTerrainInteractionCount() const { return ActiveInteractions.Num(); }

protected:
    // Terrain physics properties per type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TMap<ECore_TerrainType, FCore_TerrainPhysicsProperties> TerrainPhysicsMap;

    // Biome to terrain type mapping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TMap<EBiomeType, ECore_TerrainType> BiomeTerrainMapping;

    // Active terrain interactions
    UPROPERTY()
    TArray<FCore_TerrainInteractionData> ActiveInteractions;

    // Landscape references
    UPROPERTY()
    TArray<TWeakObjectPtr<ALandscapeProxy>> ManagedLandscapes;

    // Physical materials for different terrain types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TMap<ECore_TerrainType, TSoftObjectPtr<UPhysicalMaterial>> TerrainPhysicalMaterials;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveInteractions = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float InteractionCleanupInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TerrainUpdateRadius = 5000.0f;

private:
    // Internal helper functions
    void InitializeDefaultTerrainProperties();
    void SetupDefaultBiomeTerrainMapping();
    void CleanupExpiredInteractions();
    void UpdateTerrainPhysicsLOD(float DeltaTime);
    
    // Biome architecture reference
    UPROPERTY()
    TWeakObjectPtr<UEng_BiomeArchitecture> BiomeArchitecture;

    // Performance tracking
    UPROPERTY()
    float LastCleanupTime;

    UPROPERTY()
    float LastLODUpdateTime;

    UPROPERTY()
    int32 TerrainPhysicsUpdateCount;
};