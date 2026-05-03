#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Arcade          UMETA(DisplayName = "Arcade Physics"), 
    Cinematic       UMETA(DisplayName = "Cinematic Physics"),
    Survival        UMETA(DisplayName = "Survival Physics")
};

UENUM(BlueprintType)
enum class ECore_CollisionPreset : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Debris          UMETA(DisplayName = "Debris")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableDestruction = true;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        Restitution = 0.3f;
        Friction = 0.7f;
        bEnableRagdoll = true;
        bEnableDestruction = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    FCore_RagdollSettings()
    {
        BlendTime = 0.2f;
        RecoveryTime = 2.0f;
        ImpactThreshold = 500.0f;
        bAutoRecover = true;
    }
};

/**
 * Core Physics System Manager - Handles all physics simulation, collision, ragdoll, and destruction
 * This is the central authority for physics behavior in the prehistoric survival world
 */
UCLASS(BlueprintType, Blueprintable)
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

    // Physics Settings
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsSettings(const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    FCore_PhysicsSettings GetPhysicsSettings() const { return PhysicsSettings; }

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetCollisionPreset(AActor* Actor, ECore_CollisionPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable = true);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableRagdoll(AActor* Character, const FCore_RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DisableRagdoll(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsRagdollActive(AActor* Character) const;

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TriggerDestruction(AActor* Actor, FVector ImpactLocation, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetDestructible(AActor* Actor, bool bDestructible = true);

    // Force Application
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyForceAtLocation(AActor* Actor, FVector Force, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyImpulse(AActor* Actor, FVector Impulse);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bVelChange = false);

    // Physics Queries
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool LineTrace(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool SphereTrace(FVector Start, FVector End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    TArray<AActor*> GetActorsInRadius(FVector Center, float Radius);

    // System Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics System")
    void ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics System")
    void RunPhysicsTests();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    ECore_PhysicsMode CurrentPhysicsMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System")
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    TArray<AActor*> RagdollActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    TArray<AActor*> DestructibleActors;

private:
    void InitializePhysicsSettings();
    void UpdatePhysicsSimulation(float DeltaTime);
    void ProcessRagdollActors(float DeltaTime);
    void ProcessDestructibleActors(float DeltaTime);
    
    // Physics mode implementations
    void ApplyRealisticPhysics();
    void ApplyArcadePhysics();
    void ApplyCinematicPhysics();
    void ApplySurvivalPhysics();
};