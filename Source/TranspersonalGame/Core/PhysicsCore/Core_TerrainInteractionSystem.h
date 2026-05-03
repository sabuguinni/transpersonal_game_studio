#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_TerrainInteractionSystem.generated.h"

// Forward declarations
class ALandscape;
class ULandscapeComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTerrainInteraction, AActor*, Actor, FVector, Location, ECore_TerrainType, TerrainType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootstepGenerated, FVector, Location, ECore_SurfaceMaterial, SurfaceType);

/**
 * Core terrain interaction system that handles all physics interactions between actors and terrain
 * Manages footstep detection, surface material identification, and terrain deformation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainInteractionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN INTERACTION CORE ===
    
    /** Check what type of terrain the actor is standing on */
    UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation) const;
    
    /** Get surface material properties at a specific location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
    ECore_SurfaceMaterial GetSurfaceMaterialAtLocation(const FVector& WorldLocation) const;
    
    /** Generate footstep interaction at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
    void GenerateFootstepInteraction(const FVector& FootLocation, float ImpactForce);
    
    /** Check if terrain can be deformed at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
    bool CanDeformTerrainAtLocation(const FVector& WorldLocation) const;
    
    /** Apply terrain deformation (for heavy impacts) */
    UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
    void ApplyTerrainDeformation(const FVector& ImpactLocation, float DeformationRadius, float DeformationDepth);

    // === SURFACE DETECTION ===
    
    /** Perform detailed surface trace to identify material */
    UFUNCTION(BlueprintCallable, Category = "Surface Detection")
    bool TraceSurfaceAtLocation(const FVector& StartLocation, const FVector& EndLocation, FHitResult& OutHit) const;
    
    /** Get movement modifier based on surface type */
    UFUNCTION(BlueprintCallable, Category = "Surface Detection")
    float GetMovementModifierForSurface(ECore_SurfaceMaterial SurfaceType) const;
    
    /** Check if surface is slippery */
    UFUNCTION(BlueprintCallable, Category = "Surface Detection")
    bool IsSurfaceSlippery(ECore_SurfaceMaterial SurfaceType) const;

    // === ENVIRONMENTAL PHYSICS ===
    
    /** Apply environmental effects based on terrain type */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyEnvironmentalEffects(AActor* TargetActor, ECore_TerrainType TerrainType, float DeltaTime);
    
    /** Calculate stability factor for actor on current terrain */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    float CalculateTerrainStability(AActor* Actor) const;

protected:
    // === CONFIGURATION ===
    
    /** Maximum distance for terrain interaction detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float MaxInteractionDistance = 200.0f;
    
    /** Frequency of terrain checks per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float TerrainCheckFrequency = 10.0f;
    
    /** Enable terrain deformation system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableTerrainDeformation = true;
    
    /** Enable detailed surface material detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDetailedSurfaceDetection = true;

    // === SURFACE MATERIAL PROPERTIES ===
    
    /** Movement speed multipliers for different surface types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties")
    TMap<ECore_SurfaceMaterial, float> SurfaceMovementModifiers;
    
    /** Friction coefficients for different surface types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties")
    TMap<ECore_SurfaceMaterial, float> SurfaceFrictionValues;
    
    /** Stability factors for different terrain types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties")
    TMap<ECore_TerrainType, float> TerrainStabilityFactors;

    // === EVENTS ===
    
    /** Called when terrain interaction occurs */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTerrainInteraction OnTerrainInteraction;
    
    /** Called when footstep is generated */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFootstepGenerated OnFootstepGenerated;

private:
    // === INTERNAL STATE ===
    
    /** Timer for terrain checks */
    float TerrainCheckTimer = 0.0f;
    
    /** Cache of recent terrain queries for performance */
    TMap<FVector, ECore_TerrainType> TerrainTypeCache;
    
    /** Cache of recent surface material queries */
    TMap<FVector, ECore_SurfaceMaterial> SurfaceMaterialCache;
    
    /** Maximum cache size to prevent memory bloat */
    static constexpr int32 MaxCacheSize = 100;

    // === INTERNAL METHODS ===
    
    /** Initialize surface material properties */
    void InitializeSurfaceProperties();
    
    /** Clean old entries from cache */
    void CleanCache();
    
    /** Get landscape component at location */
    ULandscapeComponent* GetLandscapeComponentAtLocation(const FVector& WorldLocation) const;
    
    /** Analyze static mesh for surface material */
    ECore_SurfaceMaterial AnalyzeStaticMeshSurface(UStaticMeshComponent* MeshComponent) const;
    
    /** Calculate terrain type from landscape data */
    ECore_TerrainType CalculateTerrainTypeFromLandscape(const FVector& WorldLocation) const;
};