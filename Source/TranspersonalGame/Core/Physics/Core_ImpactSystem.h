#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Core_ImpactSystem.generated.h"

class UPrimitiveComponent;
class USoundCue;
class UParticleSystem;
class UNiagaraSystem;

/**
 * Settings for material-specific impact behavior
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_ImpactSettings
{
    GENERATED_BODY()

    /** Multiplier for damage calculation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float DamageMultiplier = 1.0f;

    /** Volume multiplier for impact sounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float SoundVolume = 1.0f;

    /** Scale multiplier for impact particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float ParticleScale = 1.0f;

    /** Whether this material can cause damage on impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bCanCauseDamage = true;

    /** Whether to play sound effects on impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bPlaySound = true;

    /** Whether to spawn particle effects on impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bSpawnParticles = true;

    FCore_ImpactSettings()
    {
        DamageMultiplier = 1.0f;
        SoundVolume = 1.0f;
        ParticleScale = 1.0f;
        bCanCauseDamage = true;
        bPlaySound = true;
        bSpawnParticles = true;
    }
};

/**
 * Core Impact System Component
 * 
 * Handles collision impacts, material-based responses, damage calculation,
 * and audio/visual effect spawning for realistic impact feedback.
 * 
 * Features:
 * - Material-based impact processing (Stone, Wood, Flesh, etc.)
 * - Velocity-based damage calculation
 * - Dynamic sound and particle effect spawning
 * - Configurable impact thresholds and material properties
 * - Integration with UE5 physics and collision systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_ImpactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_ImpactSystem();

protected:
    virtual void BeginPlay() override;

    /** Initialize the impact system */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void InitializeImpactSystem();

    /** Setup default material impact mappings */
    void SetupDefaultMaterialImpacts();

    /** Register collision event callbacks */
    void RegisterCollisionCallbacks();

public:
    /** Collision event handler */
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    /** Process an impact event */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ProcessImpact(const FHitResult& HitResult, float ImpactVelocity, UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent);

    /** Determine material type from hit result */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    FString DetermineMaterialType(const FHitResult& HitResult, UPrimitiveComponent* Component);

    /** Play impact sound based on material and strength */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void PlayImpactSound(const FVector& Location, const FString& MaterialType, float ImpactStrength, float VolumeMultiplier);

    /** Spawn impact particles based on material and strength */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SpawnImpactParticles(const FVector& Location, const FVector& Normal, const FString& MaterialType, float ImpactStrength, float ScaleMultiplier);

    /** Apply damage based on impact velocity and material */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ApplyImpactDamage(AActor* TargetActor, float ImpactVelocity, float DamageMultiplier);

    /** Set impact settings for a specific material type */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SetMaterialImpactSettings(const FString& MaterialType, const FCore_ImpactSettings& Settings);

    /** Get impact settings for a specific material type */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    FCore_ImpactSettings GetMaterialImpactSettings(const FString& MaterialType) const;

    /** Set the velocity range for impact detection */
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SetImpactVelocityRange(float MinVelocity, float MaxVelocity);

protected:
    /** Minimum impact velocity to trigger effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float MinImpactVelocity = 300.0f;

    /** Maximum impact velocity for scaling calculations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float MaxImpactVelocity = 2000.0f;

    /** Default impact settings for unknown materials */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    FCore_ImpactSettings DefaultImpactSettings;

    /** Map of material types to their impact settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    TMap<FString, FCore_ImpactSettings> MaterialImpactMap;

    /** Impact sound assets for different materials */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Assets")
    TMap<FString, TSoftObjectPtr<USoundCue>> ImpactSounds;

    /** Impact particle systems for different materials */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Assets")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> ImpactParticles;
};