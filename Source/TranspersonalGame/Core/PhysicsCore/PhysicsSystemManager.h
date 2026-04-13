#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsSystemManager.generated.h"

class UCore_RagdollComponent;
class UCore_DestructionComponent;
class UCore_CollisionComponent;

UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float FixedTimeStep = 0.016667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsQuality QualityLevel = ECore_PhysicsQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulatedBodies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 5000.0f;
};

USTRUCT(BlueprintType)
struct FCore_PhysicsStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 SleepingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CollisionPairs = 0;
};

/**
 * Core physics system manager that orchestrates all physics subsystems
 * Manages ragdoll, destruction, collision, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core physics management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantPhysicsBodies(const FVector& ViewerLocation);

    // Component registration
    UFUNCTION(BlueprintCallable, Category = "Component Management")
    void RegisterRagdollComponent(UCore_RagdollComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Component Management")
    void RegisterDestructionComponent(UCore_DestructionComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Component Management")
    void RegisterCollisionComponent(UCore_CollisionComponent* Component);

    // Stats and monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
    FCore_PhysicsStats GetPhysicsStats() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TogglePhysicsDebugDraw();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidatePhysicsSetup();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FCore_PhysicsStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TArray<UCore_RagdollComponent*> RegisteredRagdollComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TArray<UCore_DestructionComponent*> RegisteredDestructionComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TArray<UCore_CollisionComponent*> RegisteredCollisionComponents;

private:
    void UpdatePhysicsStats();
    void ApplyQualitySettings();
    void ManagePhysicsLOD();

    bool bIsInitialized = false;
    bool bDebugDrawEnabled = false;
    float StatsUpdateTimer = 0.0f;
    static constexpr float STATS_UPDATE_FREQUENCY = 0.1f; // 10 times per second
};