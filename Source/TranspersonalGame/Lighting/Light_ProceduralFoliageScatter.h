#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_ProceduralFoliageScatter.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ScatterPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Pattern")
    int32 ObjectCount = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Pattern")
    float MinScale = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Pattern")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Pattern")
    float ScatterRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Pattern")
    FString ObjectPrefix = TEXT("Scattered");

    FLight_ScatterPattern()
    {
        ObjectCount = 30;
        MinScale = 0.3f;
        MaxScale = 1.2f;
        ScatterRadius = 2000.0f;
        ObjectPrefix = TEXT("Scattered");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FoliageVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Variation")
    TArray<float> TreeScaleFactors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Variation")
    int32 GrassPatchCount = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Variation")
    float GrassMinScale = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Variation")
    float GrassMaxScale = 4.0f;

    FLight_FoliageVariation()
    {
        TreeScaleFactors = {0.7f, 1.0f, 1.3f, 1.6f}; // small, medium, large, extra large
        GrassPatchCount = 20;
        GrassMinScale = 2.0f;
        GrassMaxScale = 4.0f;
    }
};

/**
 * Procedural foliage scatter system for creating natural environment variety
 * Manages scattered rocks, bushes, grass patches, and tree size variations
 * Integrates with lighting system for realistic environmental distribution
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_ProceduralFoliageScatter : public AActor
{
    GENERATED_BODY()

public:
    ALight_ProceduralFoliageScatter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Scatter configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Configuration")
    FLight_ScatterPattern RockScatterPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Configuration")
    FLight_ScatterPattern BushScatterPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Configuration")
    FLight_FoliageVariation FoliageVariation;

    // Terrain bounds for scattering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D TerrainMinBounds = FVector2D(-2000.0f, -2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D TerrainMaxBounds = FVector2D(2000.0f, 2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float DefaultSpawnHeight = 100.0f;

    // Mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* RockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* BushMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* GrassMesh;

    // Tracking arrays
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> ScatteredRocks;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> ScatteredBushes;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> GrassPatches;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> VariedTrees;

public:
    // Main scatter functions
    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void ScatterRocks();

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void ScatterBushes();

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void CreateGrassPatches();

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void VaryExistingTrees();

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void ExecuteFullScatter();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    FVector GetRandomScatterLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    float GetRandomScale(float MinScale, float MaxScale) const;

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    FRotator GetRandomRotation() const;

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void ClearAllScatteredObjects();

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    int32 GetTotalScatteredObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Foliage Scatter")
    void LogScatterStatistics() const;

protected:
    // Helper functions
    AActor* SpawnScatteredObject(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FString& LabelPrefix, int32 Index);
    
    void LoadDefaultMeshes();
    
    TArray<AActor*> FindActorsWithLabelContaining(const FString& LabelSubstring) const;
};