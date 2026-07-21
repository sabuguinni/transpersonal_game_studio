#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_WaterSystemManager.generated.h"

/**
 * Water System Manager - Handles dynamic water bodies, rivers, and water flow
 * Creates and manages lakes, rivers, waterfalls, and water-based ecosystems
 * Supports seasonal water level changes and dynamic flow systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WaterSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WaterSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Water body components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water System")
    class USceneComponent* RootSceneComponent;

    // Water configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    float BaseWaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    float SeasonalVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    bool bDynamicWaterLevels;

    // Water materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* LakeWaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* RiverWaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* WaterfallMaterial;

    // Water body arrays
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Bodies")
    TArray<class AStaticMeshActor*> Lakes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Bodies")
    TArray<class AStaticMeshActor*> Rivers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Bodies")
    TArray<class AStaticMeshActor*> Waterfalls;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Bodies")
    TArray<class AStaticMeshActor*> Ponds;

    // Flow system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flow System")
    TArray<FVector> FlowPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow System")
    float FlowParticleIntensity;

public:
    // Water creation functions
    UFUNCTION(BlueprintCallable, Category = "Water Creation")
    void CreateMainLake(FVector Location, float Scale = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Water Creation")
    void CreateRiverSystem(const TArray<FVector>& RiverPoints);

    UFUNCTION(BlueprintCallable, Category = "Water Creation")
    void CreateWaterfall(FVector Location, FRotator Rotation, float Height = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Water Creation")
    void CreatePond(FVector Location, float Scale = 5.0f, const FString& BiomeType = TEXT("Generic"));

    // Water management functions
    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void UpdateWaterLevels(float SeasonalFactor);

    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void UpdateFlowEffects();

    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void SetWaterMaterials();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Water Utility")
    bool IsLocationNearWater(FVector Location, float Radius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Water Utility")
    FVector GetNearestWaterSource(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water Utility")
    float GetWaterDepthAtLocation(FVector Location) const;

    // Ecosystem functions
    UFUNCTION(BlueprintCallable, Category = "Water Ecosystem")
    void SpawnWaterVegetation();

    UFUNCTION(BlueprintCallable, Category = "Water Ecosystem")
    void CreateFishingSpots();

    UFUNCTION(BlueprintCallable, Category = "Water Ecosystem")
    void UpdateWaterQuality(float PollutionLevel);

private:
    // Internal water creation helpers
    AStaticMeshActor* CreateWaterBody(FVector Location, FVector Scale, FRotator Rotation, const FString& Label);
    void SetupWaterMesh(AStaticMeshActor* WaterActor, class UStaticMesh* Mesh);
    void ApplyWaterMaterial(AStaticMeshActor* WaterActor, class UMaterialInterface* Material);

    // Flow system helpers
    void CreateFlowMarkers();
    void UpdateFlowDirection();
    void CalculateWaterFlow();

    // Current water state
    float CurrentWaterLevel;
    float CurrentFlowRate;
    bool bWaterSystemActive;
};