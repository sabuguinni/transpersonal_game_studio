#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

class UCore_TerrainPhysicsManager;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Quality")
    ECore_PhysicsQuality PhysicsQuality = ECore_PhysicsQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "30", ClampMax = "120"))
    int32 MaxPhysicsObjects = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PhysicsTimeStep = 0.016f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bEnableTerrainPhysics = true;
};

/**
 * Core Physics System Manager - Central coordinator for all physics subsystems
 * Manages terrain physics, ragdoll systems, and destruction mechanics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics System Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FCore_PhysicsSettings GetCurrentPhysicsSettings() const { return CurrentSettings; }

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQuality(ECore_PhysicsQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    // System Access
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UCore_TerrainPhysicsManager* GetTerrainPhysicsManager() const { return TerrainPhysicsManager; }

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UCore_RagdollSystem* GetRagdollSystem() const { return RagdollSystem; }

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UCore_DestructionSystem* GetDestructionSystem() const { return DestructionSystem; }

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsStatistics();

protected:
    // Core subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UCore_TerrainPhysicsManager> TerrainPhysicsManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCore_PhysicsSettings CurrentSettings;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFramePhysicsTime;

private:
    void CreateSubsystems();
    void ConfigurePhysicsEngine();
    void UpdatePerformanceMetrics();

    bool bSystemsInitialized;
    FTimerHandle PerformanceUpdateTimer;
};

/**
 * Component for actors that need advanced physics integration
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void EnableAdvancedPhysics(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterWithPhysicsManager();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UnregisterFromPhysicsManager();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bUseTerrainPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bUseRagdollPhysics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bUseDestruction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float PhysicsUpdateRate = 1.0f;

protected:
    UPROPERTY()
    TObjectPtr<UCore_PhysicsSystemManager> PhysicsManager;

    bool bRegisteredWithManager;
    float LastUpdateTime;
};