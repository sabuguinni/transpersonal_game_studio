#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Manages all physics interactions in the prehistoric world
 * Handles collision, ragdoll, destruction and environmental physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics configuration
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetWorldPhysicsGravity(float NewGravity = -980.0f);

    // Collision management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UnregisterPhysicsActor(AActor* Actor);

    // Ragdoll system integration
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdollForActor(AActor* Actor);

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyDestructionForce(AActor* Actor, FVector ForceLocation, float ForceStrength);

    // Environmental physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetEnvironmentalPhysics(ECore_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsForWeather(ECore_WeatherType WeatherType);

    // Collision detection
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool CheckActorCollision(AActor* ActorA, AActor* ActorB);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    TArray<AActor*> GetActorsInRadius(FVector Location, float Radius);

    // Physics debugging
    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void DebugPhysicsState();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsDebugDraw(bool bEnable);

protected:
    // Registered physics actors
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    TArray<AActor*> RegisteredActors;

    // Physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float WorldGravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float DefaultAngularDamping;

    // Environmental modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float WaterDensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float MudFrictionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float RockHardnessMultiplier;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    float PhysicsUpdateTime;

private:
    // Internal physics management
    void UpdatePhysicsActors();
    void CleanupInvalidActors();
    void ApplyEnvironmentalModifiers();

    // Collision callbacks
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    // Performance monitoring
    void TrackPhysicsPerformance();
};