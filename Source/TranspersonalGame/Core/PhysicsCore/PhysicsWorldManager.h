#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "PhysicsWorldManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsWorldEvent, AActor*, Actor, const FString&, EventType);

/**
 * Physics World Manager - Centralized physics world management and coordination
 * Handles world-level physics settings, simulation control, and cross-system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsWorldManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsWorldManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World Physics Control
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetWorldGravity(const FVector& NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    FVector GetWorldGravity() const;

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    bool IsPhysicsSimulationEnabled() const;

    // Physics Performance Management
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsTickRate(float TickRate);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsTickRate() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetMaxPhysicsObjects(int32 MaxObjects);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetCurrentPhysicsObjectCount() const;

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void RegisterPhysicsMaterial(const FString& MaterialName, class UPhysicalMaterial* Material);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetPhysicsMaterial(const FString& MaterialName) const;

    // Physics Collision Management
    UFUNCTION(BlueprintCallable, Category = "Physics Collision")
    void SetCollisionProfile(AActor* Actor, const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Physics Collision")
    void EnableActorPhysics(AActor* Actor, bool bSimulatePhysics = true);

    UFUNCTION(BlueprintCallable, Category = "Physics Collision")
    void DisableActorPhysics(AActor* Actor);

    // Physics Debugging
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void EnablePhysicsDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ShowPhysicsStats(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DumpPhysicsWorldInfo();

    // Physics Events
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsWorldEvent OnPhysicsWorldEvent;

    // Physics Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void CullDistantPhysicsObjects(const FVector& ViewLocation, float CullDistance);

protected:
    // Internal physics management
    void InitializePhysicsWorld();
    void UpdatePhysicsSettings();
    void MonitorPhysicsPerformance();

    // Physics object tracking
    void RegisterPhysicsActor(AActor* Actor);
    void UnregisterPhysicsActor(AActor* Actor);

private:
    // Physics world settings
    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    FVector WorldGravity;

    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSimulationEnabled;

    UPROPERTY(EditAnywhere, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bDebugVisualizationEnabled;

    // Physics material registry
    UPROPERTY()
    TMap<FString, class UPhysicalMaterial*> PhysicsMaterials;

    // Active physics actors
    UPROPERTY()
    TArray<AActor*> ActivePhysicsActors;

    // Performance monitoring
    UPROPERTY()
    float LastPerformanceCheckTime;

    UPROPERTY()
    int32 PhysicsObjectCount;

    UPROPERTY()
    float AveragePhysicsFrameTime;
};