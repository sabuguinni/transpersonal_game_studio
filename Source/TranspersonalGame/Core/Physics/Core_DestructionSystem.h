#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fragmentation   UMETA(DisplayName = "Fragmentation"),
    Collapse        UMETA(DisplayName = "Collapse"),
    Explosion       UMETA(DisplayName = "Explosion"),
    Erosion         UMETA(DisplayName = "Erosion")
};

USTRUCT(BlueprintType)
struct FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DamageThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fragmentation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bApplyPhysicsToFragments = true;

    FCore_DestructionData()
    {
        Health = 100.0f;
        DamageThreshold = 50.0f;
        DestructionType = ECore_DestructionType::Fragmentation;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        bApplyPhysicsToFragments = true;
    }
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bDestroyOnZeroHealth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactForceMultiplier = 1.0f;

    // Destruction Methods
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, FVector ImpactLocation = FVector::ZeroVector, FVector ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(FVector ImpactLocation = FVector::ZeroVector, FVector ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(FVector ImpactLocation, FVector ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleCollapse(FVector ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleExplosion(FVector ImpactLocation, float ExplosionRadius = 500.0f);

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool IsDestroyed() const { return DestructionData.Health <= 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RepairDamage(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionType(ECore_DestructionType NewType);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDestruction, AActor*, DestroyedActor, FVector, ImpactLocation, ECore_DestructionType, DestructionType);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDestruction OnDestruction;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageReceived, float, DamageAmount, float, RemainingHealth);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDamageReceived OnDamageReceived;

private:
    // Internal state
    bool bIsDestroyed = false;
    TArray<AActor*> SpawnedFragments;

    // Helper methods
    UStaticMeshComponent* GetOwnerMeshComponent() const;
    void CleanupFragments();
    FVector CalculateFragmentVelocity(FVector ImpactLocation, FVector FragmentLocation, FVector ImpactForce) const;
};