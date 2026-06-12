#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Core/Engine/BiomeManager.h"
#include "SharedTypes.h"
#include "Core_BiomePhysicsAdapter.generated.h"

/**
 * Core_BiomePhysicsAdapter - Integrates BiomeManager with physics systems
 * Provides biome-specific physics materials, terrain friction, and environmental physics
 * Bridges Engine Architect's biome system with Core Systems physics implementation
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_BiomePhysicsAdapter : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_BiomePhysicsAdapter();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Physics Materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Physics")
    TMap<EBiomeType, UPhysicalMaterial*> BiomePhysicsMaterials;

    // Terrain Friction Values per Biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    TMap<EBiomeType, float> BiomeFrictionValues;

    // Movement Speed Multipliers per Biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    TMap<EBiomeType, float> BiomeMovementMultipliers;

    // Environmental Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Physics")
    TMap<EBiomeType, float> BiomeGravityMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Physics")
    TMap<EBiomeType, float> BiomeWindResistance;

    // Cached References
    UPROPERTY()
    UBiomeManager* BiomeManager;

    UPROPERTY()
    EBiomeType CurrentBiome;

    UPROPERTY()
    EBiomeType PreviousBiome;

public:
    // Physics Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    UPhysicalMaterial* GetCurrentBiomePhysicsMaterial() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    float GetCurrentBiomeFriction() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    float GetCurrentBiomeMovementMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    float GetCurrentBiomeGravityMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void ApplyBiomePhysicsToActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void UpdateBiomePhysics(EBiomeType NewBiome);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    FVector CalculateBiomeWindForce(const FVector& ActorVelocity) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    float CalculateBiomeMovementPenalty(float BaseSpeed) const;

    // Terrain Interaction
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysics(class UPrimitiveComponent* Component, const FVector& ImpactPoint);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsTerrainSuitableForMovement(const FVector& Location, float Radius = 50.0f) const;

protected:
    // Internal Methods
    void InitializeBiomePhysicsMaterials();
    void InitializeBiomePhysicsValues();
    void OnBiomeChanged(EBiomeType OldBiome, EBiomeType NewBiome);
    void CacheBiomeManager();

    // Physics Material Creation
    UPhysicalMaterial* CreateBiomePhysicsMaterial(EBiomeType BiomeType);
    void ConfigurePhysicsMaterialForBiome(UPhysicalMaterial* Material, EBiomeType BiomeType);

    // Validation
    bool ValidateBiomePhysicsSetup() const;
};