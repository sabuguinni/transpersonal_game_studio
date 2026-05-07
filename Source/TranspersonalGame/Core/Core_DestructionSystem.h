#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
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
    Burn            UMETA(DisplayName = "Burn"),
    Dissolve        UMETA(DisplayName = "Dissolve")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float HealthPoints = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DamageThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpulseStrength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bPlayEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<UParticleSystem> DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<USoundCue> DestructionSound;

    FCore_DestructionData()
    {
        DestructionType = ECore_DestructionType::Fracture;
        HealthPoints = 100.0f;
        DamageThreshold = 50.0f;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        ImpulseStrength = 500.0f;
        bCreateDebris = true;
        bPlayEffects = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_DestructionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bIsDestructible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bIsDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    // Destruction Methods
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PlayDestructionEffects(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetHealthPercentage() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDestructionTriggered, AActor*, DestroyedActor, FVector, ImpactPoint, FVector, ImpactDirection);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDestructionTriggered OnDestructionTriggered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, float, RemainingHealth);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDamageTaken OnDamageTaken;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedFragments;

    void CleanupFragments();
    AActor* CreateFragment(const FVector& Location, const FVector& Velocity, int32 FragmentIndex);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_DestructibleActor : public AActor
{
    GENERATED_BODY()

public:
    ACore_DestructibleActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCore_DestructionComponent* DestructionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UParticleSystemComponent* EffectsComponent;

    // Destruction Interface
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TakeDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionType(ECore_DestructionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    ECore_DestructionType GetDestructionType() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool IsDestroyed() const;

protected:
    UFUNCTION()
    void OnDestructionTriggered(AActor* DestroyedActor, FVector ImpactPoint, FVector ImpactDirection);

    UFUNCTION()
    void OnDamageTaken(float DamageAmount, float RemainingHealth);

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_DestructionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Global Destruction Management
    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void RegisterDestructibleActor(ACore_DestructibleActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void UnregisterDestructibleActor(ACore_DestructibleActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void TriggerAreaDestruction(const FVector& Center, float Radius, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void CleanupDestroyedActors();

    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    int32 GetActiveDestructibleCount() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    TArray<ACore_DestructibleActor*> GetDestructibleActorsInRadius(const FVector& Center, float Radius);

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveFragments = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FragmentCleanupInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDestructionLOD = true;

private:
    UPROPERTY()
    TArray<ACore_DestructibleActor*> RegisteredActors;

    UPROPERTY()
    TArray<AActor*> ActiveFragments;

    FTimerHandle CleanupTimerHandle;

    void PerformCleanup();
    bool ShouldCreateFragment(const FVector& Location) const;
};