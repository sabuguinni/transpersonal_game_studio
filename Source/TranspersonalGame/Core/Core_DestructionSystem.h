#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None = 0,
    Shatter = 1,
    Crumble = 2,
    Explode = 3,
    Dissolve = 4,
    Burn = 5
};

UENUM(BlueprintType)
enum class ECore_MaterialType : uint8
{
    Stone = 0,
    Wood = 1,
    Bone = 2,
    Metal = 3,
    Flesh = 4,
    Plant = 5,
    Crystal = 6
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_MaterialType MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bPlaySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateParticles;

    FCore_DestructionSettings()
    {
        DestructionType = ECore_DestructionType::Shatter;
        MaterialType = ECore_MaterialType::Stone;
        Health = 100.0f;
        MaxHealth = 100.0f;
        FragmentCount = 10.0f;
        ExplosionForce = 1000.0f;
        FragmentLifetime = 30.0f;
        bCreateDebris = true;
        bPlaySound = true;
        bCreateParticles = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float DamageAmount;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FVector ImpactLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FVector ImpactDirection;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float ImpactForce;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource;

    FCore_DamageInfo()
    {
        DamageAmount = 0.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactDirection = FVector::ZeroVector;
        ImpactForce = 0.0f;
        DamageSource = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnDestruction, AActor*, DestroyedActor, FCore_DamageInfo, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCore_OnDamaged, AActor*, DamagedActor, float, DamageAmount, float, RemainingHealth);

/**
 * Core Destruction System Component
 * Handles realistic destruction physics for environmental objects
 * Supports multiple destruction types and material-based responses
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Destruction Interface
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(const FCore_DestructionSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    FCore_DestructionSettings GetDestructionSettings() const { return DestructionSettings; }

    // Health Management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetHealth() const { return DestructionSettings.Health; }

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool IsDestroyed() const { return bIsDestroyed; }

    // Destruction Types
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ShatterDestruction(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CrumbleDestruction(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ExplodeDestruction(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void DissolveDestruction(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void BurnDestruction(const FCore_DamageInfo& DamageInfo);

    // Fragment Management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    // Effects
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PlayDestructionEffects(const FCore_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SpawnParticles(const FVector& Location, ECore_MaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PlayDestructionSound(const FVector& Location, ECore_MaterialType MaterialType);

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RepairObject();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Destruction")
    void TestDestruction();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnDestruction OnDestruction;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnDamaged OnDamaged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionSettings DestructionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<ECore_MaterialType, USoundCue*> DestructionSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TMap<ECore_MaterialType, UParticleSystem*> DestructionParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragments")
    UStaticMesh* FragmentMesh;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsDestroyed;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<AActor*> SpawnedFragments;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastDamageTime;

private:
    FTimerHandle FragmentCleanupTimer;
    UStaticMeshComponent* OriginalMesh;
    USkeletalMeshComponent* OriginalSkeletalMesh;

    void InitializeComponent();
    void CacheOriginalMesh();
    void ApplyMaterialBasedDamage(float BaseDamage, ECore_MaterialType MaterialType);
    float GetMaterialResistance(ECore_MaterialType MaterialType) const;
    FVector CalculateFragmentVelocity(const FVector& ImpactLocation, const FVector& FragmentLocation, float Force) const;
    void HideOriginalMesh();
    void ShowOriginalMesh();
};

/**
 * Destructible Actor Base Class
 * Pre-configured actor with destruction system component
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_DestructibleActor : public AActor
{
    GENERATED_BODY()

public:
    ACore_DestructibleActor();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    UCore_DestructionSystem* GetDestructionSystem() const { return DestructionSystem; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCore_DestructionSystem* DestructionSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FCore_DestructionSettings DefaultDestructionSettings;

private:
    void SetupDefaultSettings();
};