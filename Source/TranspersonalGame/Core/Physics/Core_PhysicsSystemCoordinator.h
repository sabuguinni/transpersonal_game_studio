#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Core_PhysicsSystemCoordinator.generated.h"

// Forward declarations for cross-module types
class UCore_PhysicsIntegrationMaster;
class UCore_PhysicsOptimizer;
class UCore_CharacterPhysics;
class UCore_TerrainPhysics;
class UCore_CollisionManager;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

UENUM(BlueprintType)
enum class ECore_PhysicsSystemState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Paused          UMETA(DisplayName = "Paused"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    Adaptive    UMETA(DisplayName = "Adaptive Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RagdollFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCollisionShapes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsSubsteps = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quality")
    ECore_PhysicsQuality CurrentQuality = ECore_PhysicsQuality::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsOptimizationActive = false;

    FCore_PhysicsSystemMetrics()
    {
        PhysicsFrameTime = 0.0f;
        CollisionFrameTime = 0.0f;
        RagdollFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        ActiveCollisionShapes = 0;
        PhysicsSubsteps = 1;
        CurrentQuality = ECore_PhysicsQuality::Medium;
        bIsOptimizationActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSystemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float FixedDeltaTime = 0.016667f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxSubstepDeltaTime = 0.016667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableComplexCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionTolerance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRigidBodies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdollPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxDestructionFragments = 100;

    FCore_PhysicsSystemConfig()
    {
        FixedDeltaTime = 0.016667f;
        MaxSubsteps = 6;
        MaxSubstepDeltaTime = 0.016667f;
        bEnableComplexCollision = true;
        CollisionTolerance = 0.1f;
        bEnableAdaptiveQuality = true;
        TargetFrameTime = 16.67f;
        MaxActiveRigidBodies = 1000;
        bEnableRagdollPhysics = true;
        RagdollBlendTime = 0.2f;
        bEnableDestruction = true;
        MaxDestructionFragments = 100;
    }
};

/**
 * Core Physics System Coordinator
 * Master coordinator for all physics systems in the prehistoric survival game.
 * Manages initialization, updates, performance monitoring, and system integration.
 * Designed for the "Walk Around" milestone - ensures smooth character movement and physics.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core system management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsSystems(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    ECore_PhysicsSystemState GetSystemState() const { return CurrentState; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PhysicsSystemMetrics GetPhysicsMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQuality(ECore_PhysicsQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    // Configuration management
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void ApplyPhysicsConfig(const FCore_PhysicsSystemConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FCore_PhysicsSystemConfig GetPhysicsConfig() const { return SystemConfig; }

    // Character physics integration
    UFUNCTION(BlueprintCallable, Category = "Character")
    bool RegisterCharacterPhysics(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UnregisterCharacterPhysics(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool IsCharacterPhysicsRegistered(ACharacter* Character) const;

    // System validation and diagnostics
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    bool ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    TArray<FString> GetSystemDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void LogPhysicsState();

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsSystemStateChanged, ECore_PhysicsSystemState, NewState);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsQualityChanged, ECore_PhysicsQuality, NewQuality);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsPerformanceWarning, float, FrameTime);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsSystemStateChanged OnSystemStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsQualityChanged OnQualityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsPerformanceWarning OnPerformanceWarning;

protected:
    // Core system state
    UPROPERTY(BlueprintReadOnly, Category = "System State")
    ECore_PhysicsSystemState CurrentState = ECore_PhysicsSystemState::Uninitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FCore_PhysicsSystemConfig SystemConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsSystemMetrics CurrentMetrics;

    // System references (using raw pointers for cross-module compatibility)
    UCore_PhysicsIntegrationMaster* PhysicsIntegrationMaster = nullptr;
    UCore_PhysicsOptimizer* PhysicsOptimizer = nullptr;
    UCore_CharacterPhysics* CharacterPhysics = nullptr;
    UCore_TerrainPhysics* TerrainPhysics = nullptr;
    UCore_CollisionManager* CollisionManager = nullptr;
    UCore_RagdollSystem* RagdollSystem = nullptr;
    UCore_DestructionSystem* DestructionSystem = nullptr;

    // Registered characters
    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacter>> RegisteredCharacters;

    // Performance tracking
    UPROPERTY()
    float LastFrameTime = 0.0f;

    UPROPERTY()
    float AverageFrameTime = 16.67f;

    UPROPERTY()
    int32 FrameSampleCount = 0;

    UPROPERTY()
    bool bPerformanceWarningActive = false;

    // Internal methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void CheckPerformanceThresholds();
    void OptimizePhysicsQuality();
    bool CreatePhysicsSubsystems();
    void CleanupPhysicsSubsystems();
    void SetSystemState(ECore_PhysicsSystemState NewState);

    // Timer handles
    FTimerHandle PhysicsUpdateTimer;
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle ValidationTimer;
};