#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysicsIntegrator.generated.h"

/**
 * Core Terrain Physics Integrator
 * Manages physics integration between terrain systems and physics simulation
 * Handles collision detection, surface materials, and terrain deformation physics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core terrain physics integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainCollision(ALandscape* Landscape);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainMaterialPhysics(const FVector& Location, ECore_TerrainType TerrainType);

    // Surface physics properties
    UFUNCTION(BlueprintCallable, Category = "Surface Physics")
    float GetTerrainFriction(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Surface Physics")
    float GetTerrainBounciness(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Surface Physics")
    FVector GetTerrainNormal(const FVector& Location) const;

    // Deformation physics
    UFUNCTION(BlueprintCallable, Category = "Deformation")
    void ApplyTerrainDeformation(const FVector& Location, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Deformation")
    bool CanDeformTerrain(const FVector& Location) const;

    // Physics material management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void SetupTerrainPhysicsMaterials();

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetPhysicsMaterialForTerrain(ECore_TerrainType TerrainType) const;

    // Collision optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeTerrainCollision(float ViewDistance);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void UpdateCollisionLOD(const FVector& PlayerLocation);

protected:
    // Terrain physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, float> TerrainFrictionValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, float> TerrainBouncinessValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, class UPhysicalMaterial*> TerrainPhysicsMaterials;

    // Deformation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float MaxDeformationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float DeformationStrengthMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bEnableTerrainDeformation;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CollisionUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CollisionCullingDistance;

private:
    // Internal state
    UPROPERTY()
    ALandscape* CachedLandscape;

    UPROPERTY()
    TArray<AActor*> PhysicsEnabledActors;

    // Performance tracking
    float LastCollisionUpdate;
    int32 CurrentLODLevel;

    // Helper functions
    void CacheLandscapeReference();
    void UpdatePhysicsActorsList();
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& Location) const;
    void ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial);
};