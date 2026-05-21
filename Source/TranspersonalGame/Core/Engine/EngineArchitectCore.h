#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "CollisionQueryParams.h"
#include "Engine/CollisionProfile.h"
#include "../../SharedTypes.h"
#include "EngineArchitectCore.generated.h"

/**
 * Engine Architect Core System
 * Defines the fundamental technical architecture for the Transpersonal Game Studio
 * Establishes physics, collision, performance, and system integration rules
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    FEng_PerformanceMetrics()
    {
        FrameRate = 60.0f;
        ActorCount = 0;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsSimulation;

    FEng_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.01f;
        bEnablePhysicsSimulation = true;
    }
};

UENUM(BlueprintType)
enum class EEng_CollisionPreset : uint8
{
    DinosaurBody        UMETA(DisplayName = "Dinosaur Body"),
    DinosaurTrigger     UMETA(DisplayName = "Dinosaur Trigger Zone"),
    PlayerCharacter     UMETA(DisplayName = "Player Character"),
    Environment         UMETA(DisplayName = "Environment"),
    Projectile          UMETA(DisplayName = "Projectile"),
    Interactable        UMETA(DisplayName = "Interactable Object")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical            UMETA(DisplayName = "Critical"),
    High                UMETA(DisplayName = "High"),
    Normal              UMETA(DisplayName = "Normal"),
    Low                 UMETA(DisplayName = "Low")
};

/**
 * Core Engine Architecture Manager
 * Manages performance, physics, and system integration across all game modules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectCore : public UObject
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceThresholds();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void OptimizeWorldActors();

    // Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetupDinosaurPhysics(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ConfigureCollisionPreset(UPrimitiveComponent* Component, EEng_CollisionPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePhysicsIntegrity();

    // System Architecture
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckSystemIntegrity();

    // World State Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateWorldState();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetActiveActorCount();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    FEng_PhysicsSettings DefaultPhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    float MaxMemoryUsageMB;

private:
    void UpdatePerformanceMetrics();
    void ApplyCollisionSettings(UPrimitiveComponent* Component, EEng_CollisionPreset Preset);
    bool ValidateActorPhysics(AActor* Actor);
};

/**
 * Engine Architect Subsystem
 * World subsystem that manages engine architecture across the entire game world
 */
UCLASS()
class TRANSPERSONALGAME_API UEngineArchitectSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UEngineArchitectCore* GetEngineArchitect();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy();

protected:
    UPROPERTY()
    UEngineArchitectCore* EngineArchitect;

private:
    void InitializePhysicsSettings();
    void SetupCollisionProfiles();
    void ValidateModuleIntegration();
};