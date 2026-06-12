#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Standard        UMETA(DisplayName = "Standard Physics"),
    HighPrecision   UMETA(DisplayName = "High Precision"),
    Performance     UMETA(DisplayName = "Performance Optimized"),
    Cinematic       UMETA(DisplayName = "Cinematic Quality")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    bool bEnableCCD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    int32 SolverIterations = 8;

    FCore_PhysicsProfile()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        MaxAngularVelocity = 3600.0f;
        bEnableCCD = false;
        SolverIterations = 8;
    }
};

/**
 * Core Physics System Manager - Handles global physics configuration and optimization
 * Manages physics profiles, collision layers, and performance settings
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
    // Physics Mode Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    ECore_PhysicsMode GetCurrentPhysicsMode() const { return CurrentPhysicsMode; }

    // Physics Profile Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsProfile(const FCore_PhysicsProfile& Profile);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    FCore_PhysicsProfile GetCurrentPhysicsProfile() const { return CurrentProfile; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsProfiler(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    float GetPhysicsFrameTime() const { return PhysicsFrameTime; }

    UFUNCTION(BlueprintPure, Category = "Physics System")
    int32 GetActiveRigidBodies() const { return ActiveRigidBodies; }

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterCollisionProfile(const FName& ProfileName, const FCollisionResponseContainer& Responses);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void OptimizeCollisionForPerformance();

    // Gravity and World Physics
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetWorldGravity(const FVector& NewGravity);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    FVector GetWorldGravity() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor = true)
    void DebugDrawPhysicsStats();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TogglePhysicsVisualization(bool bShowCollision, bool bShowConstraints, bool bShowMass);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    ECore_PhysicsMode CurrentPhysicsMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    FCore_PhysicsProfile CurrentProfile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    bool bPhysicsProfilerEnabled;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    float PhysicsFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    int32 ActiveRigidBodies;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    float ProfilerUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    TMap<FName, FCollisionResponseContainer> CustomCollisionProfiles;

private:
    void UpdatePhysicsProfiler();
    void ApplyPhysicsModeSettings();
    void CountActiveRigidBodies();

    float ProfilerTimer;
    UWorld* CachedWorld;
};