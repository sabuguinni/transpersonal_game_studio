#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsSystemManager.generated.h"

// Forward declarations
class UEng_ArchitectureManager;

UENUM(BlueprintType)
enum class ECore_PhysicsSystemState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Running         UMETA(DisplayName = "Running"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    bool bChaosEnabled;

    FCore_PhysicsMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        CollisionChecks = 0;
        MemoryUsageMB = 0.0f;
        bChaosEnabled = false;
    }
};

USTRUCT(BlueprintType)
struct FCore_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> ObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bBlockAll;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bIgnoreAll;

    FCore_CollisionProfile()
    {
        ProfileName = NAME_None;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECC_WorldStatic;
        bBlockAll = false;
        bIgnoreAll = false;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics-related systems including collision, rigid body dynamics,
 * destruction, and integration with the Engine Architecture Manager.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // === SYSTEM STATE MANAGEMENT ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    ECore_PhysicsSystemState SystemState;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    FCore_PhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    bool bEnablePhysicsDebugging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    bool bEnableCollisionDebugging;

    // === SYSTEM INITIALIZATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool ValidatePhysicsConfiguration();

    // === COLLISION MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionProfile(const FCore_CollisionProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool ApplyCollisionProfileToActor(AActor* TargetActor, const FName& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupDinosaurCollision(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupEnvironmentCollision(AActor* EnvironmentActor);

    // === PHYSICS METRICS ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics Metrics")
    FCore_PhysicsMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Metrics")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Metrics")
    bool IsPhysicsPerformanceAcceptable() const;

    // === RIGID BODY MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Rigid Body")
    void EnableRigidBodyPhysics(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Rigid Body")
    void DisableRigidBodyPhysics(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Rigid Body")
    void SetRigidBodyMass(UPrimitiveComponent* Component, float Mass);

    // === DESTRUCTION SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void EnableDestructionForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(AActor* TargetActor, const FVector& ImpactPoint, float Force);

    // === ARCHITECTURE INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Architecture Integration")
    void RegisterWithArchitectureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture Integration")
    void ReportSystemStatus();

    // === DEBUG AND VALIDATION ===
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPhysicsSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateAllPhysicsActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ShowCollisionDebugInfo();

protected:
    // === INTERNAL SYSTEMS ===
    
    UPROPERTY()
    TArray<FCore_CollisionProfile> RegisteredCollisionProfiles;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedPhysicsActors;

    UPROPERTY()
    TWeakObjectPtr<UEng_ArchitectureManager> ArchitectureManager;

    // === INTERNAL METHODS ===
    
    void InitializeCollisionProfiles();
    void SetupDefaultPhysicsSettings();
    void RegisterPhysicsCallbacks();
    void UnregisterPhysicsCallbacks();
    
    // Metrics calculation
    void CalculatePhysicsFrameTime();
    void CountActiveRigidBodies();
    void CountCollisionChecks();
    void CalculateMemoryUsage();

    // Validation helpers
    bool ValidateChaosPhysics() const;
    bool ValidateCollisionSettings() const;
    bool ValidatePhysicsWorldSettings() const;

private:
    // === PERFORMANCE TRACKING ===
    
    float LastPhysicsUpdateTime;
    int32 PhysicsFrameCounter;
    float AccumulatedPhysicsTime;
    
    // === ERROR HANDLING ===
    
    FString LastErrorMessage;
    float ErrorReportCooldown;
    static constexpr float ERROR_REPORT_INTERVAL = 5.0f;
};