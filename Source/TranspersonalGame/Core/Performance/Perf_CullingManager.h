#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Perf_CullingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float OcclusionCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float SmallObjectCullDistance;

    FPerf_CullingSettings()
    {
        FrustumCullingDistance = 15000.0f;
        OcclusionCullingDistance = 8000.0f;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        bEnableDistanceCulling = true;
        SmallObjectCullDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullableObject
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    UPrimitiveComponent* Component;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float BoundsRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsCurrentlyVisible;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float LastVisibilityCheck;

    FPerf_CullableObject()
    {
        Component = nullptr;
        BoundsRadius = 100.0f;
        LastKnownLocation = FVector::ZeroVector;
        bIsCurrentlyVisible = true;
        LastVisibilityCheck = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_CullingManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_CullingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxObjectsPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_CullableObject> CullableObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ObjectsCulledThisFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalObjectsManaged;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CullingEfficiency;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterCullableComponent(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterCullableComponent(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCullingForObject(FPerf_CullableObject& CullableObject);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAllCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsObjectInFrustum(const FVector& ObjectLocation, float ObjectRadius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsObjectOccluded(const FVector& ObjectLocation, float ObjectRadius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldCullByDistance(const FVector& ObjectLocation, float ObjectRadius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetDistanceToPlayer(const FVector& ObjectLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableCullingSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceVisibilityUpdate();

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetVisibleObjectCount();

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetCulledObjectCount();

private:
    float TimeSinceLastUpdate;
    APawn* CachedPlayerPawn;
    int32 CurrentUpdateIndex;
    bool bCullingEnabled;

    void CachePlayerPawn();
    void ProcessCullingBatch(int32 StartIndex, int32 EndIndex);
    void UpdateCullingStatistics();
    FVector GetPlayerCameraLocation();
    FVector GetPlayerCameraForward();
};