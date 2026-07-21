#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "EngineArchitecturalFramework.h"
#include "Core_RagdollPhysicsSystem.generated.h"

// Ragdoll state for prehistoric creatures and characters
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive     UMETA(DisplayName = "Inactive"),
    Activating   UMETA(DisplayName = "Activating"),
    Active       UMETA(DisplayName = "Active"),
    Blending     UMETA(DisplayName = "Blending"),
    Recovering   UMETA(DisplayName = "Recovering")
};

// Ragdoll trigger types for realistic prehistoric scenarios
UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    Death           UMETA(DisplayName = "Death"),
    KnockOut        UMETA(DisplayName = "Knock Out"),
    HeavyImpact     UMETA(DisplayName = "Heavy Impact"),
    Falling         UMETA(DisplayName = "Falling"),
    DinosaurAttack  UMETA(DisplayName = "Dinosaur Attack"),
    Manual          UMETA(DisplayName = "Manual")
};

// Ragdoll configuration for different creature types
USTRUCT(BlueprintType)
struct FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float ActivationForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RecoveryTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float DamageThreshold = 50.0f;

    FCore_RagdollConfig()
    {
        ActivationForce = 500.0f;
        BlendTime = 0.2f;
        RecoveryTime = 2.0f;
        bAutoRecover = true;
        MaxRagdollTime = 10.0f;
        DamageThreshold = 50.0f;
    }
};

// Ragdoll performance metrics
USTRUCT(BlueprintType)
struct FCore_RagdollMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageBlendTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActivations = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime = 0.0f;

    FCore_RagdollMetrics()
    {
        ActiveRagdolls = 0;
        AverageBlendTime = 0.0f;
        TotalActivations = 0;
        LastUpdateTime = 0.0f;
    }
};

/**
 * Core Ragdoll Physics System for Transpersonal Game
 * Manages realistic ragdoll physics for characters and dinosaurs in prehistoric environments
 * Integrates with EngineArchitecturalFramework for performance monitoring
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core ragdoll functionality
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ActivateRagdoll(ECore_RagdollTrigger TriggerType, FVector ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    // Prehistoric-specific ragdoll features
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyDinosaurAttackRagdoll(FVector AttackDirection, float AttackForce);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyFallingRagdoll(float FallHeight, FVector ImpactPoint);

    UFUNCTION(BlueprintCallable, Category = "Combat Physics")
    void ApplyWeaponImpactRagdoll(FVector ImpactPoint, FVector ImpactDirection, float ImpactForce);

    // Performance and architectural integration
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RegisterWithArchitecturalFramework();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FCore_RagdollMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawRagdollState();

protected:
    // Core ragdoll state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Config")
    FCore_RagdollConfig RagdollConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FCore_RagdollMetrics PerformanceMetrics;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UEngineArchitecturalFramework* ArchitecturalFramework;

    // Internal state tracking
    UPROPERTY()
    float StateTimer;

    UPROPERTY()
    float BlendAlpha;

    UPROPERTY()
    ECore_RagdollTrigger LastTriggerType;

    UPROPERTY()
    FVector LastImpactForce;

    // Performance tracking
    UPROPERTY()
    float LastPerformanceUpdate;

    UPROPERTY()
    TArray<float> BlendTimeHistory;

private:
    // Internal ragdoll management
    void UpdateRagdollState(float DeltaTime);
    void BlendToRagdoll(float DeltaTime);
    void BlendFromRagdoll(float DeltaTime);
    void UpdatePerformanceMetrics(float DeltaTime);
    
    // Physics integration
    void SetupPhysicsAsset();
    void ApplyImpactForce(FVector Force, FVector ImpactPoint);
    void ConfigureRagdollConstraints();
    
    // Architectural framework integration
    void ReportPerformanceToFramework();
    bool ValidateArchitecturalCompliance();
};