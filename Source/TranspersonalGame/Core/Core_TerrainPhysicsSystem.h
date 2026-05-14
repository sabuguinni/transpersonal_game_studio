#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Core_TerrainPhysicsSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainSurfaceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Ice         UMETA(DisplayName = "Ice"),
    Water       UMETA(DisplayName = "Water"),
    Lava        UMETA(DisplayName = "Lava"),
    Wood        UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainSurfaceProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties")
    ECore_TerrainSurfaceType SurfaceType = ECore_TerrainSurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float Density = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hardness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties")
    bool bCanDeform = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float DeformationThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float RecoveryRate = 1.0f;

    FCore_TerrainSurfaceProperties()
    {
        SurfaceType = ECore_TerrainSurfaceType::Dirt;
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1500.0f;
        Hardness = 0.5f;
        bCanDeform = true;
        DeformationThreshold = 1000.0f;
        RecoveryRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainDeformationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Depth = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Force = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bIsRecovering = false;

    FCore_TerrainDeformationData()
    {
        Location = FVector::ZeroVector;
        Radius = 100.0f;
        Depth = 10.0f;
        Force = 1000.0f;
        Timestamp = 0.0f;
        bIsRecovering = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnTerrainDeformation, FVector, Location, float, Force);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCore_OnSurfaceImpact, FVector, Location, ECore_TerrainSurfaceType, SurfaceType, float, ImpactForce);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Surface Material System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainSurfaceType, FCore_TerrainSurfaceProperties> SurfaceProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<UPhysicalMaterial*, ECore_TerrainSurfaceType> PhysicalMaterialMapping;

    // Deformation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float GlobalDeformationThreshold = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MaxDeformationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxDeformationDepth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    TArray<FCore_TerrainDeformationData> ActiveDeformations;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 MaxActiveDeformations = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float DeformationUpdateFrequency = 0.1f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnTerrainDeformation OnTerrainDeformation;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnSurfaceImpact OnSurfaceImpact;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainSurfaceType GetSurfaceTypeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainSurfaceProperties GetSurfacePropertiesAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ApplyTerrainDeformation(const FVector& Location, float Force, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ProcessSurfaceImpact(const FVector& Location, float ImpactForce, UPrimitiveComponent* HitComponent = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ClearAllDeformations();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    int32 GetActiveDeformationCount() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetSurfaceProperties(ECore_TerrainSurfaceType SurfaceType, const FCore_TerrainSurfaceProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationDeformed(const FVector& Location, float Tolerance = 50.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics", CallInEditor)
    void InitializeDefaultSurfaceProperties();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics", CallInEditor)
    void ValidatePhysicalMaterialMappings();

protected:
    // Internal Methods
    void UpdateDeformations(float DeltaTime);
    void ProcessDeformationRecovery(FCore_TerrainDeformationData& Deformation, float DeltaTime);
    UPhysicalMaterial* GetPhysicalMaterialAtLocation(const FVector& Location);
    ALandscape* FindLandscapeAtLocation(const FVector& Location);
    void ApplyPhysicalMaterialProperties(UPrimitiveComponent* Component, const FCore_TerrainSurfaceProperties& Properties);
    void CleanupExpiredDeformations();

private:
    float LastDeformationUpdate = 0.0f;
    TWeakObjectPtr<ALandscape> CachedLandscape;
    
    // Performance tracking
    int32 DeformationsProcessedThisFrame = 0;
    float FrameStartTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API ACore_TerrainPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCore_TerrainPhysicsSystem* TerrainPhysicsSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
    bool bAutoRegisterWithGameMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
    bool bProcessGlobalTerrainEvents = true;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Manager")
    static ACore_TerrainPhysicsManager* GetTerrainPhysicsManager(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Manager")
    void RegisterTerrainActor(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Manager")
    void UnregisterTerrainActor(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Manager")
    void ProcessGlobalTerrainImpact(const FVector& Location, float Force, AActor* Instigator = nullptr);

protected:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredTerrainActors;

    void OnTerrainDeformationReceived(FVector Location, float Force);
    void OnSurfaceImpactReceived(FVector Location, ECore_TerrainSurfaceType SurfaceType, float ImpactForce);
};