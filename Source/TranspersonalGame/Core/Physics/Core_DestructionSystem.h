#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode"),
    Burn            UMETA(DisplayName = "Burn")
};

USTRUCT(BlueprintType)
struct FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float DestructionThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1", ClampMax = "50"))
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bApplyImpulseToFragments = true;

    FCore_DestructionSettings()
    {
        DestructionType = ECore_DestructionType::Fracture;
        DestructionThreshold = 500.0f;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        bCreateDebris = true;
        bApplyImpulseToFragments = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnDestruction, AActor*, DestroyedActor, const FVector&, ImpactLocation);

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
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

    // Health and Damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float MaxHealth = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bCanBeDestroyed = true;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnDestruction OnDestruction;

    // Main Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(const FCore_DestructionSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool IsDestroyed() const;

    // Collision Response
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
    UPROPERTY()
    bool bIsDestroyed = false;

    UPROPERTY()
    TArray<AActor*> CreatedFragments;

    FTimerHandle CleanupTimerHandle;

    void InitializeDestruction();
    void CleanupFragments();
    AActor* CreateFragment(const FVector& Location, const FVector& Velocity, float Scale);
    void SetupCollisionResponse();
};