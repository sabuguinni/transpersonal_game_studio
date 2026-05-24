#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "Core_TerrainDeformation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTerrainDeformed, FVector, Location, float, Radius, float, Intensity);

/**
 * Terrain Deformation Data Structure
 * Stores information about terrain modifications for physics simulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainDeformationData
{
    GENERATED_BODY()

    /** Location of the deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    FVector Location = FVector::ZeroVector;

    /** Radius of the deformation effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float Radius = 100.0f;

    /** Intensity of the deformation (-1.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float Intensity = 0.5f;

    /** Type of deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    ECore_TerrainType TerrainType = ECore_TerrainType::Dirt;

    /** Timestamp when deformation occurred */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float Timestamp = 0.0f;

    /** Whether this deformation is permanent */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    bool bIsPermanent = false;

    FCore_TerrainDeformationData()
    {
        Location = FVector::ZeroVector;
        Radius = 100.0f;
        Intensity = 0.5f;
        TerrainType = ECore_TerrainType::Dirt;
        Timestamp = 0.0f;
        bIsPermanent = false;
    }
};

/**
 * Terrain Deformation Component
 * Handles dynamic terrain modification for physics interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainDeformationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainDeformationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Apply deformation to terrain at specified location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyDeformation(const FVector& Location, float Radius, float Intensity, ECore_TerrainType TerrainType = ECore_TerrainType::Dirt);

    /** Remove deformation from terrain */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void RemoveDeformation(const FVector& Location, float Radius);

    /** Get terrain hardness at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    float GetTerrainHardness(const FVector& Location) const;

    /** Check if location can be deformed */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    bool CanDeformTerrain(const FVector& Location, ECore_TerrainType TerrainType) const;

    /** Event called when terrain is deformed */
    UPROPERTY(BlueprintAssignable, Category = "Terrain Deformation")
    FOnTerrainDeformed OnTerrainDeformed;

protected:
    /** Maximum deformation radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    float MaxDeformationRadius = 500.0f;

    /** Maximum deformation intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    float MaxDeformationIntensity = 2.0f;

    /** Time for temporary deformations to fade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    float DeformationFadeTime = 30.0f;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    bool bEnableDebugVisualization = false;

private:
    /** Active deformations */
    UPROPERTY()
    TArray<FCore_TerrainDeformationData> ActiveDeformations;

    /** Cached landscape reference */
    UPROPERTY()
    ALandscape* CachedLandscape = nullptr;

    /** Update deformation fade */
    void UpdateDeformationFade(float DeltaTime);

    /** Find landscape in world */
    ALandscape* FindLandscape() const;

    /** Apply deformation to landscape */
    void ApplyLandscapeDeformation(const FCore_TerrainDeformationData& DeformationData);
};

/**
 * Terrain Deformation Manager
 * Global system for managing terrain deformation across the world
 */
UCLASS()
class TRANSPERSONALGAME_API UCore_TerrainDeformationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get the terrain deformation manager instance */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation", CallInEditor = true)
    static UCore_TerrainDeformationManager* GetTerrainDeformationManager(const UObject* WorldContext);

    /** Register terrain deformation component */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void RegisterDeformationComponent(UCore_TerrainDeformationComponent* Component);

    /** Unregister terrain deformation component */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void UnregisterDeformationComponent(UCore_TerrainDeformationComponent* Component);

    /** Apply global terrain deformation */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyGlobalDeformation(const FVector& Location, float Radius, float Intensity, ECore_TerrainType TerrainType = ECore_TerrainType::Dirt);

    /** Get terrain data at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    FCore_TerrainDeformationData GetTerrainDataAtLocation(const FVector& Location) const;

    /** Enable/disable terrain deformation system */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void SetTerrainDeformationEnabled(bool bEnabled);

protected:
    /** Registered deformation components */
    UPROPERTY()
    TArray<UCore_TerrainDeformationComponent*> RegisteredComponents;

    /** Whether terrain deformation is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    bool bTerrainDeformationEnabled = true;

    /** Global deformation multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    float GlobalDeformationMultiplier = 1.0f;

    /** Maximum number of active deformations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveDeformations = 100;

private:
    /** Clean up invalid component references */
    void CleanupInvalidComponents();
};

#include "Core_TerrainDeformation.generated.h"