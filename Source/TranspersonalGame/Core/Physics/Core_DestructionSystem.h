#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode")
};

USTRUCT(BlueprintType)
struct FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentCount = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanRegenerate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float RegenerationTime = 30.0f;

    FCore_DestructionData()
    {
        DestructionType = ECore_DestructionType::None;
        Health = 100.0f;
        FragmentCount = 8.0f;
        ExplosionForce = 500.0f;
        bCanRegenerate = false;
        RegenerationTime = 30.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    TArray<UStaticMesh*> FragmentMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    UMaterialInterface* FragmentMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    bool bIsDestroyed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    float RegenerationTimer = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void StartRegeneration();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ResetDestruction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Destruction Events")
    void OnDestructionTriggered(const FVector& ImpactPoint);

    UFUNCTION(BlueprintImplementableEvent, Category = "Destruction Events")
    void OnRegenerationComplete();

private:
    void CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection);
    void ProcessRegeneration(float DeltaTime);

    TArray<AActor*> SpawnedFragments;
    UStaticMeshComponent* OriginalMeshComponent;
    bool bRegenerationInProgress = false;
};