// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "PhysicsOptimizer.generated.h"

USTRUCT(BlueprintType)
struct FPhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MidDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FarDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxNearObjects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMidObjects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxFarObjects = 20;
};

USTRUCT(BlueprintType)
struct FPhysicsObjectInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UPrimitiveComponent> Component;

    UPROPERTY()
    float DistanceToPlayer = 0.0f;

    UPROPERTY()
    bool bIsActive = true;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    int32 LODLevel = 0; // 0=Near, 1=Mid, 2=Far, 3=Disabled
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsOptimizer : public UObject
{
    GENERATED_BODY()

public:
    UPhysicsOptimizer();

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void InitializeOptimizer(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdateOptimization(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void RegisterPhysicsObject(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UnregisterPhysicsObject(UPrimitiveComponent* Component);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLODLevel(UPrimitiveComponent* Component, int32 LODLevel);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    float GetCurrentPhysicsLoad() const { return CurrentPhysicsLoad; }

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    int32 GetActivePhysicsObjects() const { return ActivePhysicsObjects.Num(); }

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPhysicsLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxPhysicsLoad = 0.8f; // 80% max physics load

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnablePhysicsCulling = true;

protected:
    // Internal state
    UPROPERTY()
    TWeakObjectPtr<UWorld> WorldContext;

    UPROPERTY()
    TMap<TWeakObjectPtr<UPrimitiveComponent>, FPhysicsObjectInfo> TrackedObjects;

    UPROPERTY()
    TArray<TWeakObjectPtr<UPrimitiveComponent>> ActivePhysicsObjects;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    float CurrentPhysicsLoad = 0.0f;

    UPROPERTY()
    int32 FrameCounter = 0;

    // Performance tracking
    UPROPERTY()
    TArray<float> RecentFrameTimes;

    UPROPERTY()
    float AverageFrameTime = 0.0f;

    // Threading
    FThreadSafeBool bOptimizationActive;
    TQueue<TWeakObjectPtr<UPrimitiveComponent>> PendingRegistrations;
    TQueue<TWeakObjectPtr<UPrimitiveComponent>> PendingUnregistrations;

private:
    // Internal optimization methods
    void ProcessPendingRegistrations();
    void ProcessPendingUnregistrations();
    void CalculatePhysicsLoad();
    void UpdateFrameTimeTracking(float DeltaTime);
    void ApplyAdaptiveLOD();
    void CullDistantObjects();
    FVector GetPlayerLocation() const;
    void OptimizeCollisionSettings(UPrimitiveComponent* Component, int32 LODLevel);
    void UpdatePhysicsSubsteps();
};