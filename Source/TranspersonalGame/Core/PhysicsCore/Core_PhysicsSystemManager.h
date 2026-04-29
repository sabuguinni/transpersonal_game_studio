#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager for Transpersonal Game
 * Handles realistic physics simulation for prehistoric survival gameplay
 * Manages collision detection, ragdoll physics, and environmental interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void UpdatePhysicsSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void ShutdownPhysicsSystem();

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    bool PerformLineTrace(FVector Start, FVector End, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    bool PerformSphereTrace(FVector Center, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    TArray<FHitResult> PerformMultiLineTrace(FVector Start, FVector End);

    // Ragdoll Physics
    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void EnableRagdollPhysics(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void DisableRagdollPhysics(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void ApplyImpulseToRagdoll(AActor* Actor, FVector Impulse, FVector Location);

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void SimulateWindEffect(AActor* Actor, FVector WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void ApplyGravityEffect(AActor* Actor, float GravityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void HandleWaterPhysics(AActor* Actor, bool bIsInWater);

    // Destruction Physics
    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void CreateDestructibleMesh(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Physics Core")
    void TriggerDestruction(AActor* Actor, FVector ImpactPoint, float Force);

    // Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultAngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableRealisticPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsSimulationRate;

    // Debug Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowPhysicsDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawCollisionShapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bLogPhysicsEvents;

protected:
    // Internal physics state
    UPROPERTY()
    bool bPhysicsSystemInitialized;

    UPROPERTY()
    TArray<AActor*> RagdollActors;

    UPROPERTY()
    TArray<AActor*> DestructibleActors;

    // Internal methods
    void ConfigurePhysicsSettings();
    void SetupCollisionChannels();
    void InitializePhysicsConstraints();
    void UpdateRagdollSimulation(float DeltaTime);
    void UpdateDestructionSimulation(float DeltaTime);

private:
    // Physics simulation data
    float LastUpdateTime;
    int32 PhysicsFrameCounter;
    TMap<AActor*, float> ActorPhysicsData;
};