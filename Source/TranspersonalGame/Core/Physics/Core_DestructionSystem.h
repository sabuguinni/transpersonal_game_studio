#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentCount = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TArray<UStaticMesh*> FragmentMeshes;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    UStaticMeshComponent* MeshComponent;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool IsDestroyed() const;

private:
    bool bIsDestroyed = false;
    float CurrentHealth = 100.0f;

    void SpawnFragments();
    void CreateFragment(const FVector& Position, const FVector& Velocity);
};

UCLASS()
class TRANSPERSONALGAME_API UCore_DestructionSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    void RegisterDestructibleActor(AActor* Actor, const FCore_DestructionData& DestructionData);

    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    void UnregisterDestructibleActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    void ProcessAreaDestruction(const FVector& Center, float Radius, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    TArray<AActor*> GetDestructibleActorsInRadius(const FVector& Center, float Radius);

protected:
    UPROPERTY()
    TMap<AActor*, FCore_DestructionData> RegisteredActors;

    UPROPERTY()
    TArray<AActor*> FragmentActors;

    void CleanupFragments();
    void UpdateDestructionStates();

private:
    FTimerHandle CleanupTimer;
    static constexpr float CleanupInterval = 30.0f;
    static constexpr int32 MaxFragments = 500;
};