#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment,     // Fog, wind, rain, ambient particles
    Creature,        // Dinosaur breath, footsteps, presence indicators
    Combat,          // Blood, impacts, destruction
    Atmosphere,      // Tension builders, subtle environmental cues
    Interaction,     // Tool usage, fire, crafting
    Narrative        // Story moments, gema effects
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,          // Background atmospheric effects
    Moderate,        // Noticeable but not distracting
    Dramatic,        // Key story/gameplay moments
    Critical         // Life/death situations
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeSpan = 10.0f;

    // Performance scaling based on distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD0_Distance = 1000.0f;  // Full quality

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD1_Distance = 2500.0f;  // Reduced particles

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD2_Distance = 5000.0f;  // Minimal particles

    FVFXDefinition()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFXCategory::Environment;
        Intensity = EVFXIntensity::Subtle;
    }
};

/**
 * Central manager for all VFX systems in the game
 * Handles spawning, pooling, LOD management and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Library - all available effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<FString, FVFXDefinition> VFXLibrary;

    // Active effect tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentEffects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateFrequency = 0.5f; // Update LOD twice per second

    // Atmosphere control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float GlobalTensionLevel = 0.5f; // 0.0 = calm, 1.0 = maximum tension

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableAtmosphericEffects = true;

private:
    float LODUpdateTimer = 0.0f;
    TArray<UNiagaraComponent*> EffectPool;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(const FString& EffectName, AActor* AttachToActor, const FName& SocketName = NAME_None, const FVector& LocationOffset = FVector::ZeroVector);

    // Atmosphere and tension control
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGlobalTensionLevel(float NewTensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TriggerTensionSpike(float Duration = 3.0f, float Intensity = 1.0f);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateEffectLODs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEffects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    // Effect library management
    UFUNCTION(BlueprintCallable, Category = "VFX Library")
    void RegisterVFXEffect(const FString& EffectName, const FVFXDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category = "VFX Library")
    bool GetVFXDefinition(const FString& EffectName, FVFXDefinition& OutDefinition) const;

    // Cleanup
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffectsByCategory(EVFXCategory Category);

private:
    void InitializeVFXLibrary();
    void CleanupFinishedEffects();
    UNiagaraComponent* GetPooledEffect();
    void ReturnEffectToPool(UNiagaraComponent* Effect);
    float CalculateLODLevel(const FVector& EffectLocation, const FVFXDefinition& Definition) const;
    void ApplyLODToEffect(UNiagaraComponent* Effect, float LODLevel);
};