#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Classes/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "TerrainDeformationSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTerrainDeformation, Log, All);

/** Terrain deformation data for a specific location */
USTRUCT(BlueprintType)
struct FTerrainDeformationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Depth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CauseActorName;

    FTerrainDeformationData()
    {
        Location = FVector::ZeroVector;
        Depth = 0.0f;
        Radius = 100.0f;
        Timestamp = 0.0f;
        CauseActorName = TEXT("");
    }
};

/** Terrain material properties */
USTRUCT(BlueprintType)
struct FTerrainMaterialProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elasticity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDeformation = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoveryRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPermanentDeformation = false;

    FTerrainMaterialProperties()
    {
        Hardness = 1.0f;
        Elasticity = 0.5f;
        MaxDeformation = 50.0f;
        RecoveryRate = 0.1f;
        bPermanentDeformation = false;
    }
};

/**
 * Terrain Deformation System for realistic ground interaction
 * Handles footprints, tire tracks, and environmental deformation
 * Supports different terrain materials (mud, sand, grass, rock)
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTerrainDeformationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UTerrainDeformationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize terrain deformation system */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void InitializeTerrainDeformation();

    /** Create deformation at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void CreateDeformation(const FVector& Location, float Force, float Radius, AActor* CauseActor = nullptr);

    /** Register actor for automatic deformation tracking */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void RegisterActorForDeformation(AActor* Actor, float DeformationForce = 1000.0f);

    /** Unregister actor from deformation tracking */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void UnregisterActorFromDeformation(AActor* Actor);

    /** Set terrain material properties for area */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void SetTerrainMaterialProperties(const FVector& Location, float Radius, const FTerrainMaterialProperties& Properties);

    /** Get terrain hardness at location */
    UFUNCTION(BlueprintPure, Category = "Terrain Deformation")
    float GetTerrainHardnessAtLocation(const FVector& Location) const;

    /** Check if location can be deformed */
    UFUNCTION(BlueprintPure, Category = "Terrain Deformation")
    bool CanLocationBeDeformed(const FVector& Location) const;

    /** Get deformation depth at location */
    UFUNCTION(BlueprintPure, Category = "Terrain Deformation")
    float GetDeformationDepthAtLocation(const FVector& Location) const;

    /** Clear all deformations */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ClearAllDeformations();

    /** Clear deformations in radius */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void ClearDeformationsInRadius(const FVector& Location, float Radius);

    /** Enable/disable terrain deformation */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void SetTerrainDeformationEnabled(bool bEnabled);

    /** Set maximum number of deformations */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void SetMaxDeformations(int32 MaxCount);

protected:
    /** Active terrain deformations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Deformation")
    TArray<FTerrainDeformationData> ActiveDeformations;

    /** Actors registered for automatic deformation */
    UPROPERTY()
    TMap<TObjectPtr<AActor>, float> TrackedActors;

    /** Terrain material properties by location */
    UPROPERTY()
    TMap<FVector, FTerrainMaterialProperties> TerrainMaterials;

    /** Deformation system enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    bool bDeformationEnabled = true;

    /** Maximum number of active deformations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveDeformations = 500;

    /** Minimum force required for deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float MinDeformationForce = 500.0f;

    /** Default terrain material properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    FTerrainMaterialProperties DefaultTerrainProperties;

    /** Deformation update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DeformationUpdateFrequency = 10.0f;

    /** Deformation recovery enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    bool bEnableDeformationRecovery = true;

    /** Debug visualization enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDeformationDebug = false;

private:
    void UpdateTrackedActors(float DeltaTime);
    void UpdateDeformationRecovery(float DeltaTime);
    void ProcessActorDeformation(AActor* Actor, float DeformationForce, float DeltaTime);
    FTerrainMaterialProperties GetTerrainPropertiesAtLocation(const FVector& Location) const;
    void ApplyDeformationToLandscape(const FTerrainDeformationData& Deformation);
    void DrawDeformationDebug() const;
    void CleanupOldDeformations();
    
    float LastDeformationUpdate = 0.0f;
    TArray<FVector> PreviousActorLocations;
};