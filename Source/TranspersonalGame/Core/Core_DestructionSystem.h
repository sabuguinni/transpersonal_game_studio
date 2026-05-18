#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float HealthPoints = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DamageThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    FCore_DestructionData()
    {
        HealthPoints = 100.0f;
        DamageThreshold = 25.0f;
        FragmentCount = 8;
        ExplosionForce = 1000.0f;
        bCanBeDestroyed = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    TArray<UStaticMesh*> FragmentMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    bool bUsePhysicsFragments = true;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    bool bIsDestroyed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    TArray<AActor*> SpawnedFragments;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ExplosionCenter);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ExplosionCenter);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetHealthPercentage() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, FVector, ExplosionCenter);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageApplied, AActor*, DamagedActor, float, DamageAmount, float, RemainingHealth);

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDestructionTriggered OnDestructionTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDamageApplied OnDamageApplied;

private:
    // Internal helpers
    void InitializeFragmentMeshes();
    AActor* CreateFragment(UStaticMesh* FragmentMesh, const FVector& Location, const FVector& Velocity);
    void ApplyFragmentPhysics(AActor* Fragment, const FVector& ExplosionCenter);

    // Cleanup timer
    FTimerHandle FragmentCleanupTimer;
};