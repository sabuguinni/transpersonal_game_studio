#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_TerrainPhysics.generated.h"

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SurfaceFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Bounciness = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableComplexCollision = true;

    FCore_TerrainPhysicsData()
    {
        SurfaceFriction = 0.7f;
        Bounciness = 0.1f;
        Density = 1.0f;
        bEnableComplexCollision = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain physics configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_TerrainPhysicsData TerrainData;

    // Landscape reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ALandscape* LandscapeActor;

    // Physics material for terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    class UPhysicalMaterial* TerrainPhysicsMaterial;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainForces(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationOnTerrain(const FVector& WorldLocation);

private:
    // Internal terrain physics calculations
    void CalculateTerrainInteraction(AActor* Actor);
    void UpdatePhysicsMaterial();
    void SetupLandscapeCollision();
};