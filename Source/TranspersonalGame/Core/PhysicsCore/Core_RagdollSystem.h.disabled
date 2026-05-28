#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinVelocityForRagdoll = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bUsePhysicsBlending = true;

    FCore_RagdollSettings()
    {
        BlendInTime = 0.2f;
        BlendOutTime = 0.5f;
        MaxRagdollTime = 10.0f;
        MinVelocityForRagdoll = 500.0f;
        RecoveryThreshold = 50.0f;
        bAutoRecover = true;
        bUsePhysicsBlending = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState = ECore_RagdollState::Disabled;

    // Ragdoll settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollSettings RagdollSettings;

    // Target skeletal mesh component
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    USkeletalMeshComponent* TargetMesh = nullptr;

    // Physics asset override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    UPhysicsAsset* CustomPhysicsAsset = nullptr;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRagdollDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDistanceCulling = true;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(bool bForceActivation = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollState(ECore_RagdollState NewState);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetPhysicsBlendWeight(float BlendWeight);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    FVector GetRagdollVelocity() const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Ragdoll")
    void TestRagdollActivation();

    UFUNCTION(CallInEditor, Category = "Ragdoll")
    void ValidatePhysicsAsset();

private:
    void InitializeRagdollSystem();
    void UpdateRagdollState(float DeltaTime);
    void BlendToRagdoll(float DeltaTime);
    void BlendFromRagdoll(float DeltaTime);
    void CheckRecoveryConditions();
    bool ShouldCullRagdoll() const;

    // Internal state
    float StateTime = 0.0f;
    float BlendWeight = 0.0f;
    FVector LastPosition = FVector::ZeroVector;
    bool bWasSimulatingPhysics = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_RagdollManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_RagdollManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Global ragdoll settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Manager")
    FCore_RagdollSettings GlobalRagdollSettings;

    // Performance limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RagdollUpdateFrequency = 30.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentActiveRagdolls = 0;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void RegisterRagdollComponent(UCore_RagdollComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void UnregisterRagdollComponent(UCore_RagdollComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void UpdateAllRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    TArray<UCore_RagdollComponent*> GetActiveRagdolls() const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Ragdoll Manager")
    void TestAllRagdolls();

    UFUNCTION(CallInEditor, Category = "Ragdoll Manager")
    void ResetAllRagdolls();

private:
    void ManageRagdollLimits();
    void CullDistantRagdolls();
    void UpdatePerformanceMetrics();

    // Registered ragdoll components
    UPROPERTY()
    TArray<UCore_RagdollComponent*> RegisteredRagdolls;

    // Performance tracking
    float LastUpdateTime = 0.0f;
    float UpdateInterval = 0.033f; // ~30 FPS
};