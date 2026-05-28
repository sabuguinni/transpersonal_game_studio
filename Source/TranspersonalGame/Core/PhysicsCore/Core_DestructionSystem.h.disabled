#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float HealthPoints = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TArray<class UStaticMesh*> DestroyedMeshPieces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    FCore_DestructionData()
    {
        HealthPoints = 100.0f;
        ImpactThreshold = 50.0f;
        bCanBeDestroyed = true;
        FragmentLifetime = 10.0f;
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

    // Destruction properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bDebugDestruction = false;

    // Destruction methods
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void DestroyObject();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionEnabled(bool bEnabled);

    // Impact detection
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
    // Internal state
    float CurrentHealth;
    bool bIsDestroyed;
    TArray<AActor*> SpawnedFragments;

    // Helper methods
    void SpawnDestructionFragments(const FVector& ImpactLocation);
    void CleanupFragments();
    void ApplyImpactForce(const FVector& ImpactLocation, const FVector& ImpactDirection, float Force);
};