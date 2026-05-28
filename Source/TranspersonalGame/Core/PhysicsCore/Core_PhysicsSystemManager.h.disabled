#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Arcade         UMETA(DisplayName = "Arcade Physics"),
    Cinematic      UMETA(DisplayName = "Cinematic Physics"),
    Disabled       UMETA(DisplayName = "Physics Disabled")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float AngularDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxPhysicsStepDeltaTime = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableCCD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableAsyncScene = true;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.01f;
        MaxPhysicsStepDeltaTime = 0.05f;
        MaxSubsteps = 6;
        bEnableCCD = true;
        bEnableAsyncScene = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsStepTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SleepingRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveConstraints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    FCore_PhysicsPerformanceMetrics()
    {
        PhysicsStepTime = 0.0f;
        ActiveRigidBodies = 0;
        SleepingRigidBodies = 0;
        ActiveConstraints = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics systems in the game including collision, ragdoll, destruction
 * Provides centralized physics configuration and performance monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Physics Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Realistic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    bool bEnablePhysicsDebugDraw = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    bool bEnablePerformanceMonitoring = true;

    // Biome-specific physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Physics")
    TMap<EBiomeType, FCore_PhysicsSettings> BiomePhysicsSettings;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 1.0f;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCore_CollisionManagerComponent* CollisionManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCore_RagdollManagerComponent* RagdollManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCore_DestructionManagerComponent* DestructionManager;

private:
    // Internal state
    float MetricsTimer = 0.0f;
    bool bIsInitialized = false;

    // Physics world references
    UPROPERTY()
    UWorld* CachedWorld;

public:
    // Physics System Interface
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetBiomePhysicsSettings(EBiomeType BiomeType, const FCore_PhysicsSettings& Settings);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void TogglePhysicsDebugDraw();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogPhysicsStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidatePhysicsSetup();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsSettings GetBiomePhysicsSettings(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterPhysicsActor(AActor* Actor);

    // Static Access
    UFUNCTION(BlueprintCallable, Category = "Physics System", meta = (CallInEditor = "true"))
    static ACore_PhysicsSystemManager* GetPhysicsSystemManager(UWorld* World);

private:
    // Internal implementation
    void InitializeComponents();
    void ConfigureWorldPhysics();
    void UpdatePhysicsSettings();
    void CollectPerformanceData();
    void ValidateConfiguration();

    // Registered actors for physics management
    UPROPERTY()
    TArray<AActor*> RegisteredPhysicsActors;
};