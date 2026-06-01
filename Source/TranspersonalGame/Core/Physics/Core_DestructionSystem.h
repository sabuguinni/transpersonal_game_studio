#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None UMETA(DisplayName = "None"),
    Fracture UMETA(DisplayName = "Fracture"),
    Shatter UMETA(DisplayName = "Shatter"),
    Crumble UMETA(DisplayName = "Crumble"),
    Explode UMETA(DisplayName = "Explode")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<UParticleSystem> DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<USoundCue> DestructionSound;

    FCore_DestructionData()
    {
        DestructionType = ECore_DestructionType::Fracture;
        Health = 100.0f;
        ImpactThreshold = 50.0f;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        bCreateDebris = true;
    }
};

/**
 * Core Destruction System Component
 * Handles object destruction, fracturing, and debris creation for realistic physics destruction
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

    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionData DestructionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bIsDestroyed = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<UStaticMesh*> FragmentMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> SpawnedFragments;

    // Destruction Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SpawnDestructionEffects(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetHealthPercentage() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, FVector, ImpactLocation);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDestructionTriggered OnDestructionTriggered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, AActor*, DamagedActor, float, DamageAmount, float, RemainingHealth);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDamageTaken OnDamageTaken;

private:
    // Internal fragment cleanup timer
    FTimerHandle FragmentCleanupTimer;

    // Helper functions
    void InitializeFragmentMeshes();
    AActor* CreateFragmentActor(UStaticMesh* FragmentMesh, const FVector& Location, const FVector& Velocity);
    void SetupFragmentPhysics(AActor* Fragment, const FVector& Velocity);
    FVector CalculateFragmentVelocity(const FVector& ImpactLocation, const FVector& FragmentLocation, const FVector& ImpactDirection);
};