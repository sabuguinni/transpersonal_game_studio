#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_PhysicsWorldManager.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsWorldManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics world configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float WorldGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float GlobalPhysicsTimeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    int32 MaxPhysicsSubSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    bool bEnableAsyncPhysics;

    // Environmental physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    float AirDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    float WaterDensity;

    // Physics optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveRigidBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsCulling;

    // Physics world management functions
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void InitializePhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void UpdatePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGlobalGravity(float NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetPhysicsTimeStep(float NewTimeStep);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyWindForce(AActor* TargetActor, float ForceMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void SetEnvironmentalConditions(float Wind, FVector WindDir, float AirDens, float WaterDens);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    int32 GetActivePhysicsBodiesCount();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPhysicsWorld();

private:
    // Internal physics management
    void UpdatePhysicsLOD();
    void CullDistantPhysicsBodies();
    void ManagePhysicsMemory();

    // Physics world state
    bool bPhysicsWorldInitialized;
    float LastPhysicsUpdateTime;
    TArray<TWeakObjectPtr<AActor>> ActivePhysicsActors;
    TArray<TWeakObjectPtr<AActor>> CulledPhysicsActors;
};