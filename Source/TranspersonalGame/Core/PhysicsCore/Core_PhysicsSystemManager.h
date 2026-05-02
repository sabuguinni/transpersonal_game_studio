#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsSimulationMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Basic           UMETA(DisplayName = "Basic Physics"),
    Advanced        UMETA(DisplayName = "Advanced Physics"),
    Ragdoll         UMETA(DisplayName = "Ragdoll Physics"),
    Destruction     UMETA(DisplayName = "Destruction Physics")
};

UENUM(BlueprintType)
enum class ECore_CollisionComplexity : uint8
{
    Simple          UMETA(DisplayName = "Simple Collision"),
    Complex         UMETA(DisplayName = "Complex Collision"),
    UseComplexAsSimple UMETA(DisplayName = "Use Complex as Simple")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsSimulationMode SimulationMode = ECore_PhysicsSimulationMode::Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Gravity = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_CollisionComplexity CollisionComplexity = ECore_CollisionComplexity::Simple;

    FCore_PhysicsSettings()
    {
        SimulationMode = ECore_PhysicsSimulationMode::Basic;
        Gravity = -980.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        MaxAngularVelocity = 3600.0f;
        bEnablePhysicsSimulation = true;
        CollisionComplexity = ECore_CollisionComplexity::Simple;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneLinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneAngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneMassScale = 1.0f;

    FCore_RagdollSettings()
    {
        bEnableRagdoll = false;
        RagdollBlendTime = 0.2f;
        BoneLinearDamping = 0.1f;
        BoneAngularDamping = 0.1f;
        BoneMassScale = 1.0f;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics simulation, collision detection, and ragdoll systems
 * for the Transpersonal Game prehistoric survival environment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Core physics management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    // Object physics control
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsForActor(AActor* Actor, bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetActorPhysicsSettings(AActor* Actor, const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyImpulseToActor(AActor* Actor, const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyForceToActor(AActor* Actor, const FVector& Force);

    // Ragdoll physics
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void EnableRagdollForCharacter(ACharacter* Character, const FCore_RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DisableRagdollForCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool IsCharacterInRagdoll(ACharacter* Character) const;

    // Destruction physics
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void CreateDestructionEffect(AActor* Actor, const FVector& ImpactPoint, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void SimulateObjectBreaking(AActor* Actor, int32 FragmentCount = 5);

    // Physics queries
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool LineTracePhysics(const FVector& Start, const FVector& End, FHitResult& HitResult) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool SphereTracePhysics(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    TArray<AActor*> GetPhysicsActorsInRadius(const FVector& Center, float Radius) const;

    // System validation
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidatePhysicsSystem() const;

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void RunPhysicsSystemDiagnostics();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_PhysicsSettings DefaultPhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_RagdollSettings DefaultRagdollSettings;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPhysicsSystemInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<TWeakObjectPtr<AActor>> ManagedPhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<TWeakObjectPtr<ACharacter>> RagdollCharacters;

private:
    // Internal physics management
    void SetupPhysicsWorld();
    void ConfigurePhysicsSettings();
    void RegisterPhysicsCallbacks();
    void CleanupInvalidActors();

    // Ragdoll implementation
    void SetupRagdollPhysics(ACharacter* Character, const FCore_RagdollSettings& Settings);
    void BlendToRagdoll(ACharacter* Character, float BlendTime);
    void BlendFromRagdoll(ACharacter* Character, float BlendTime);

    // Performance tracking
    float LastPhysicsFrameTime = 0.0f;
    int32 LastActiveActorCount = 0;
    
    // System state
    bool bSystemDiagnosticsEnabled = false;
};