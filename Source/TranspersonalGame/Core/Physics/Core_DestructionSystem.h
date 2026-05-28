#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "No Destruction"),
    Fracture        UMETA(DisplayName = "Fracture Break"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explosion"),
    Slice           UMETA(DisplayName = "Clean Slice")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.1", ClampMax = "1000.0"))
    float HealthPoints = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float DestructionThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1", ClampMax = "50"))
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bUsePhysicsFragments = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float ExplosionForce = 1000.0f;

    FCore_DestructionSettings()
    {
        DestructionType = ECore_DestructionType::Fracture;
        HealthPoints = 100.0f;
        DestructionThreshold = 500.0f;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        bUsePhysicsFragments = true;
        bCreateDebris = true;
        ExplosionForce = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionFragment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Fragment")
    class UStaticMeshComponent* FragmentMesh = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Fragment")
    FVector InitialVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Fragment")
    float LifetimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Fragment")
    bool bIsPhysicsEnabled = true;

    FCore_DestructionFragment()
    {
        FragmentMesh = nullptr;
        InitialVelocity = FVector::ZeroVector;
        LifetimeRemaining = 0.0f;
        bIsPhysicsEnabled = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, const FVector&, ImpactLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFragmentCreated, UStaticMeshComponent*, Fragment, const FVector&, Velocity, float, Lifetime);

/**
 * Core_DestructionSystem - Handles realistic destruction mechanics for prehistoric world objects
 * Supports fracturing, shattering, and physics-based debris for immersive dinosaur survival gameplay
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core destruction settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionSettings DestructionSettings;

    // Material-specific destruction properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float WoodDestructionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float StoneDestructionMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float MetalDestructionMultiplier = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float BoneDestructionMultiplier = 1.5f;

    // Current destruction state
    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    bool bIsDestroyed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    TArray<FCore_DestructionFragment> ActiveFragments;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDestructionTriggered OnDestructionTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnFragmentCreated OnFragmentCreated;

    // Main destruction functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ImpactLocation, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetDestructionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ResetDestruction();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(const FCore_DestructionSettings& NewSettings);

protected:
    // Internal destruction logic
    void ProcessFragmentLifetime(float DeltaTime);
    void CreateSingleFragment(const FVector& Position, const FVector& Velocity, float Scale);
    UStaticMeshComponent* GenerateFragmentMesh(const FVector& Position, float Scale);
    FVector CalculateFragmentVelocity(const FVector& ImpactLocation, const FVector& FragmentPosition, const FVector& ImpactForce);
    
    // Material detection
    float GetMaterialDestructionMultiplier(UPrimitiveComponent* Component);
    
    // Fragment cleanup timer
    FTimerHandle FragmentCleanupTimer;
    void StartFragmentCleanupTimer();
};