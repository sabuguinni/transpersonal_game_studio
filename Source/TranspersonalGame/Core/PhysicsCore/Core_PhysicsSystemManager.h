#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "../../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

class UCore_CollisionManager;
class UCore_DestructionSystem;
class UCore_RagdollManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GlobalGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DefaultMass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DefaultFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DefaultRestitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAdvancedPhysics = true;

    FCore_PhysicsSettings()
    {
        GlobalGravityScale = 1.0f;
        DefaultMass = 100.0f;
        DefaultFriction = 0.7f;
        DefaultRestitution = 0.3f;
        bEnableAdvancedPhysics = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsObjectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    AActor* PhysicsActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    UPrimitiveComponent* PhysicsComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_BiomeType BiomeLocation = ECore_BiomeType::Savanna;

    FCore_PhysicsObjectData()
    {
        PhysicsActor = nullptr;
        PhysicsComponent = nullptr;
        Mass = 100.0f;
        bSimulatePhysics = true;
        BiomeLocation = ECore_BiomeType::Savanna;
    }
};

/**
 * Core Physics System Manager - Coordinates all physics systems in the prehistoric survival game
 * Manages collision detection, destruction, ragdoll physics, and environmental interactions
 * Integrates with BiomeManager for location-based physics properties
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool EnablePhysicsOnActor(AActor* Actor, float Mass = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool DisablePhysicsOnActor(AActor* Actor);

    // Physics Object Registration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RegisterPhysicsObject(AActor* Actor, const FCore_PhysicsObjectData& PhysicsData);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<FCore_PhysicsObjectData> GetAllPhysicsObjects() const;

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyBiomePhysicsProperties(AActor* Actor, ECore_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyEnvironmentalForces(AActor* Actor, const FVector& Force, const FVector& Location);

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetupCollisionForActor(AActor* Actor, ECore_CollisionType CollisionType);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB);

    // Destruction System Integration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void CreateDestructionFragments(AActor* Actor, int32 FragmentCount = 5);

    // Ragdoll System Integration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnableRagdollPhysics(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DisableRagdollPhysics(AActor* Character);

    // Physics Simulation Control
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalPhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FCore_PhysicsSettings GetGlobalPhysicsSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void PausePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ResumePhysicsSimulation();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetPhysicsPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics")
    void ValidatePhysicsSystem();

protected:
    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsSettings PhysicsSettings;

    // Subsystem References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    UCore_CollisionManager* CollisionManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    UCore_DestructionSystem* DestructionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    UCore_RagdollManager* RagdollManager;

    // Physics Object Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsObjectData> RegisteredPhysicsObjects;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    int32 ActivePhysicsObjectCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    float LastFramePhysicsTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemInitialized = false;

private:
    // Internal helper functions
    void InitializeSubsystems();
    void CleanupSubsystems();
    UPrimitiveComponent* GetPrimaryPhysicsComponent(AActor* Actor);
    void UpdatePhysicsPerformanceMetrics(float DeltaTime);
};