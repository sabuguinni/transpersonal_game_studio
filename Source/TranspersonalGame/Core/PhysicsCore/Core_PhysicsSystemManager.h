#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCorePhysics, Log, All);

/**
 * Core Physics System Manager
 * Manages all physics simulation, ragdoll physics, and destructible objects
 * Provides centralized control over physics settings and performance monitoring
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GlobalPhysicsTimeDilation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableAdvancedPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableRagdollPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableDestructiblePhysics;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxPhysicsSubsteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.008", ClampMax = "0.033"))
    float MaxSubstepDeltaTime;

private:
    // Physics tracking
    UPROPERTY()
    TArray<TObjectPtr<UPrimitiveComponent>> ActiveRigidBodies;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> RagdollActors;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> DestructibleActors;

    // System management
    void InitializePhysicsSystem();
    void CleanupPhysicsSystem();
    void RegisterPhysicsEvents();
    void UnregisterPhysicsEvents();

    // Update functions
    void UpdatePhysicsSimulation(float DeltaTime);
    void MonitorPhysicsPerformance(float DeltaTime);
    void ScanForPhysicsActors();

    // Ragdoll physics management
    void UpdateRagdollPhysics(float DeltaTime);
    void UpdateDestructiblePhysics(float DeltaTime);

public:
    // Rigid body management
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void RegisterRigidBody(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void UnregisterRigidBody(UPrimitiveComponent* Component);

    // Ragdoll management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void RegisterRagdollActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void UnregisterRagdollActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void EnableRagdollPhysics(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DisableRagdollPhysics(AActor* Actor);

    // Global physics control
    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetGlobalGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetPhysicsTimeDilation(float NewTimeDilation);

    // Status queries
    UFUNCTION(BlueprintPure, Category = "Physics Status")
    int32 GetActiveRigidBodyCount() const;

    UFUNCTION(BlueprintPure, Category = "Physics Status")
    int32 GetActiveRagdollCount() const;

    UFUNCTION(BlueprintPure, Category = "Physics Status")
    bool IsPhysicsSystemEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetPhysicsSystemEnabled(bool bEnabled);
};