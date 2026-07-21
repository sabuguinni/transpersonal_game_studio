#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialInterface.h"
#include "Core_TerrainPhysicsEnhancer.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Rocky       UMETA(DisplayName = "Rocky Terrain"),
    Muddy       UMETA(DisplayName = "Muddy Terrain"),
    Sandy       UMETA(DisplayName = "Sandy Terrain"),
    Grassy      UMETA(DisplayName = "Grassy Terrain"),
    Icy         UMETA(DisplayName = "Icy Terrain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableComplexCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grassy;

    FCore_TerrainPhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        bEnableComplexCollision = true;
        TerrainType = ECore_TerrainType::Grassy;
    }
};

/**
 * Core Systems Programmer - Advanced Terrain Physics Enhancement System
 * Provides realistic terrain physics with material-based collision response
 * Integrates with UE5 Landscape system for seamless physics simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_TerrainPhysicsEnhancer : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysicsEnhancer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties PhysicsProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TArray<class ALandscape*> ManagedLandscapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float PhysicsUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableAdvancedPhysics = true;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain physics methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysicsProperties();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainTypePhysics(ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainPhysicsAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterLandscapeActor(class ALandscape* LandscapeActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UnregisterLandscapeActor(class ALandscape* LandscapeActor);

    // Advanced physics integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void EnableComplexCollisionForTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetupTerrainMaterialPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreatePhysicsZones();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ValidateTerrainPhysicsSetup();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsBasedOnPlayerDistance(FVector PlayerLocation);

protected:
    // Internal physics management
    void UpdatePhysicsProperties();
    void ProcessTerrainCollision();
    void HandleTerrainDeformation();
    
    // Terrain type specific physics
    FCore_TerrainPhysicsProperties GetRockyTerrainPhysics();
    FCore_TerrainPhysicsProperties GetMuddyTerrainPhysics();
    FCore_TerrainPhysicsProperties GetSandyTerrainPhysics();
    FCore_TerrainPhysicsProperties GetGrassyTerrainPhysics();
    FCore_TerrainPhysicsProperties GetIcyTerrainPhysics();

private:
    FTimerHandle PhysicsUpdateTimer;
    float LastPhysicsUpdate = 0.0f;
    bool bPhysicsInitialized = false;
};