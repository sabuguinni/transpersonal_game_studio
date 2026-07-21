#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode"),
    Dissolve        UMETA(DisplayName = "Dissolve")
};

USTRUCT(BlueprintType)
struct FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float DestructionThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1", ClampMax = "50"))
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float FragmentLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bPlaySound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bSpawnParticles = true;

    FCore_DestructionSettings()
    {
        DestructionType = ECore_DestructionType::Fracture;
        DestructionThreshold = 100.0f;
        FragmentCount = 8;
        FragmentLifetime = 5.0f;
        bCreateDebris = true;
        bPlaySound = true;
        bSpawnParticles = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, FVector, ImpactLocation);

/**
 * Core Destruction System - Handles realistic object destruction and fragmentation
 * Manages fracturing, debris creation, and destruction effects for environmental objects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionSettings DestructionSettings;

    // Material-specific destruction thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    TMap<FString, float> MaterialThresholds;

    // Destruction Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* DestructionParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    USoundCue* DestructionSound;

    // Fragment Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragments")
    UStaticMesh* FragmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragments", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float FragmentScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragments", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float FragmentImpulse = 500.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDestructionTriggered OnDestructionTriggered;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(FVector ImpactLocation, float ImpactForce, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionType(ECore_DestructionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionThreshold(float NewThreshold);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed(float AppliedForce) const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(FVector ImpactLocation, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PlayDestructionEffects(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    // Material-based destruction
    UFUNCTION(BlueprintCallable, Category = "Material Destruction")
    void SetMaterialThreshold(const FString& MaterialName, float Threshold);

    UFUNCTION(BlueprintCallable, Category = "Material Destruction")
    float GetMaterialThreshold(const FString& MaterialName) const;

    // Advanced destruction methods
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void CreateRadialDestruction(FVector Center, float Radius, float Force);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void CreateDirectionalDestruction(FVector Start, FVector End, float Width, float Force);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ScheduleDelayedDestruction(float Delay, FVector ImpactLocation, float ImpactForce);

private:
    // Internal state
    bool bIsDestroyed;
    float AccumulatedDamage;
    TArray<AActor*> CreatedFragments;
    FTimerHandle CleanupTimer;
    FTimerHandle DelayedDestructionTimer;

    // Internal methods
    void InitializeMaterialThresholds();
    void CreateFractureFragments(FVector ImpactLocation, FVector ImpactDirection);
    void CreateShatterFragments(FVector ImpactLocation);
    void CreateCrumbleFragments(FVector ImpactLocation);
    void CreateExplosionFragments(FVector ImpactLocation);
    void ApplyFragmentPhysics(AActor* Fragment, FVector ImpactDirection, float Force);
    void HandleDelayedDestruction();
    FString GetOwnerMaterialName() const;
};