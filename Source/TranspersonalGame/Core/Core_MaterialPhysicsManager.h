#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SharedTypes.h"
#include "Core_MaterialPhysicsManager.generated.h"

/**
 * Core_MaterialPhysicsManager
 * Manages the physical properties of materials in the prehistoric world.
 * Handles material-based physics interactions, surface properties, and visual material application.
 * Implements realistic material behaviors for different biome environments.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_MaterialPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_MaterialPhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Material Application System
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyTerrainMaterials();

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyVegetationMaterials();

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyRockMaterials();

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyDinosaurMaterials();

    // Biome-specific material application
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyBiomeMaterials(ECore_BiomeType BiomeType, const TArray<AActor*>& Actors);

    // Physical material properties
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    UPhysicalMaterial* GetPhysicalMaterialForSurface(ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void SetupPhysicalMaterialProperties();

    // Material interaction system
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    float GetFrictionForSurface(ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    float GetRestitutionForSurface(ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    bool IsSurfaceDestructible(ECore_SurfaceType SurfaceType);

protected:
    // Material references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<ECore_SurfaceType, TSoftObjectPtr<UMaterialInterface>> SurfaceMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<ECore_BiomeType, TSoftObjectPtr<UMaterialInterface>> BiomeMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<ECore_SurfaceType, TSoftObjectPtr<UPhysicalMaterial>> PhysicalMaterials;

    // Material properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    TMap<ECore_SurfaceType, float> SurfaceFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    TMap<ECore_SurfaceType, float> SurfaceRestitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    TMap<ECore_SurfaceType, bool> SurfaceDestructibility;

    // Color palettes for different elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor TerrainGreenColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor TreeBrownColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor RockGreyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor TRexDarkGreenColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor RaptorOrangeBrownColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palettes")
    FLinearColor BrachiosaurusBlueGreyColor;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaterialUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMaterialsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicMaterialUpdates;

private:
    // Internal state
    float LastMaterialUpdate;
    int32 CurrentMaterialIndex;
    TArray<AActor*> CachedActors;

    // Helper functions
    void InitializeDefaultMaterials();
    void InitializePhysicalProperties();
    void CacheWorldActors();
    UMaterialInterface* CreateBasicColorMaterial(const FLinearColor& Color, const FString& MaterialName);
    void ApplyMaterialToActor(AActor* Actor, UMaterialInterface* Material);
    ECore_SurfaceType DetermineSurfaceType(AActor* Actor);
    ECore_BiomeType DetermineBiomeType(const FVector& Location);
};