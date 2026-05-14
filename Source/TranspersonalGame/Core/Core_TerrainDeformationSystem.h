#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Landscape/LandscapeProxy.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeDataAccess.h"
#include "SharedTypes.h"
#include "Core_TerrainDeformationSystem.generated.h"

/**
 * Core Terrain Deformation System
 * Handles realistic terrain deformation from impacts, explosions, and heavy objects
 * Integrates with UE5 Landscape system for real-time height modifications
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DeformationEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Force = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Depth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    ECore_TerrainType TerrainType = ECore_TerrainType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float TimeStamp = 0.0f;

    FCore_DeformationEvent()
    {
        ImpactLocation = FVector::ZeroVector;
        Force = 0.0f;
        Radius = 100.0f;
        Depth = 50.0f;
        TerrainType = ECore_TerrainType::Dirt;
        TimeStamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    ECore_TerrainType MaterialType = ECore_TerrainType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float Hardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float DeformationResistance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float RecoveryRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float MaxDeformation = 200.0f;

    FCore_TerrainMaterial()
    {
        MaterialType = ECore_TerrainType::Dirt;
        Hardness = 1.0f;
        DeformationResistance = 1.0f;
        RecoveryRate = 0.1f;
        MaxDeformation = 200.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainDeformationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainDeformationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core deformation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyDeformation(const FCore_DeformationEvent& DeformationEvent);

    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyExplosionDeformation(FVector Location, float ExplosionForce, float BlastRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyFootstepDeformation(FVector Location, float Weight, float FootSize);

    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ApplyVehicleDeformation(FVector Location, float VehicleWeight, float TireWidth);

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    ECore_TerrainType GetTerrainTypeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetTerrainHardnessAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    bool CanDeformAtLocation(FVector Location, float RequiredForce);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Terrain Recovery")
    void ProcessTerrainRecovery(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Recovery")
    void SetRecoveryEnabled(bool bEnabled);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawDeformationZones();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateTerrainSystem();

protected:
    // Landscape integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class ALandscapeProxy* CachedLandscape;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class ULandscapeInfo* LandscapeInfo;

    // Deformation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
    float MaxDeformationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
    float MinDeformationForce = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
    float DeformationFalloffExponent = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation Settings")
    int32 MaxConcurrentDeformations = 50;

    // Material properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    TMap<ECore_TerrainType, FCore_TerrainMaterial> TerrainMaterials;

    // Recovery system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bEnableTerrainRecovery = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float GlobalRecoveryRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryTickInterval = 1.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHeightmapUpdatesPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinDeformationThreshold = 1.0f;

private:
    // Internal deformation tracking
    TArray<FCore_DeformationEvent> ActiveDeformations;
    TArray<FCore_DeformationEvent> PendingDeformations;
    
    // Landscape modification helpers
    void ModifyLandscapeHeightmap(const FCore_DeformationEvent& Event);
    void CalculateDeformationShape(const FCore_DeformationEvent& Event, TArray<FVector2D>& OutPoints, TArray<float>& OutHeights);
    
    // Material detection
    void InitializeTerrainMaterials();
    FCore_TerrainMaterial GetMaterialProperties(ECore_TerrainType TerrainType);
    
    // Performance optimization
    void OptimizeDeformationQueue();
    bool ShouldProcessDeformation(const FCore_DeformationEvent& Event);
    
    // Recovery processing
    float LastRecoveryTime = 0.0f;
    void ProcessSingleDeformationRecovery(FCore_DeformationEvent& Event, float DeltaTime);
    
    // Debug helpers
    void DrawDeformationDebugInfo(const FCore_DeformationEvent& Event);
    void LogDeformationStats();
};