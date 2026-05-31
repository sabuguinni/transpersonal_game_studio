#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Standard        UMETA(DisplayName = "Standard Physics"),
    HighPrecision   UMETA(DisplayName = "High Precision"),
    Performance     UMETA(DisplayName = "Performance Mode"),
    Cinematic       UMETA(DisplayName = "Cinematic Mode")
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
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Standard;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        PhysicsMode = ECore_PhysicsMode::Standard;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Settings Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    bool bAutoApplySettings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateRate = 60.0f;

    // Collision Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System", meta = (AllowPrivateAccess = "true"))
    bool bEnableCollisionOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System", meta = (AllowPrivateAccess = "true"))
    float CollisionCheckDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System", meta = (AllowPrivateAccess = "true"))
    int32 MaxCollisionChecksPerFrame = 100;

    // Ragdoll System Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "true"))
    bool bEnableRagdollSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "true"))
    float RagdollActivationThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "true"))
    float RagdollDeactivationDelay = 5.0f;

    // Destruction System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction System", meta = (AllowPrivateAccess = "true"))
    bool bEnableDestructionSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction System", meta = (AllowPrivateAccess = "true"))
    float DestructionForceThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction System", meta = (AllowPrivateAccess = "true"))
    int32 MaxDestructibleObjects = 50;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentPhysicsFrameTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActiveCollisionChecks = 0;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableRagdollForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DisableRagdollForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TriggerDestruction(AActor* TargetActor, FVector ImpactPoint, float Force);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintPure, Category = "Physics System")
    bool IsPhysicsObjectActive(AActor* TargetActor) const;

    UFUNCTION(BlueprintPure, Category = "Physics System")
    float GetPhysicsPerformanceScore() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollActivated, AActor*, Actor, float, ActivationForce);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionTriggered, AActor*, DestroyedActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsPerformanceWarning, float, PerformanceScore);

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnRagdollActivated OnRagdollActivated;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnDestructionTriggered OnDestructionTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsPerformanceWarning OnPhysicsPerformanceWarning;

private:
    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<AActor>> ActiveRagdollActors;
    TArray<TWeakObjectPtr<AActor>> DestructibleActors;

    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 1.0f;

    // Internal functions
    void UpdatePhysicsTracking();
    void CheckPerformance();
    void CleanupInvalidActors();
    void ProcessCollisionOptimization();
    void UpdateRagdollStates();
    void ProcessDestructionQueue();
};