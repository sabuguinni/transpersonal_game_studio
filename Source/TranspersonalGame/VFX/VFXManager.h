#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    // Environmental Effects
    AtmosphericDust,
    LeafFall,
    SporeParticles,
    MistLayers,
    
    // Dinosaur Effects
    Breathing,
    Footsteps,
    TailSwish,
    Roar,
    
    // Player Interaction
    CraftingSuccess,
    TamingProgress,
    DangerWarning,
    
    // Combat Effects
    Impact,
    Blood,
    Destruction,
    
    // Discovery Effects
    GemGlow,
    ArtifactShimmer
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,
    Moderate,
    Intense,
    Cinematic
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXType VFXType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LODLevel = 0; // 0=High, 1=Medium, 2=Low
};

/**
 * VFX Manager - Central hub for all visual effects in the game
 * Handles LOD management, performance optimization, and effect pooling
 */
UCLASS()
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Library
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Library")
    TMap<EVFXType, FVFXDefinition> VFXLibrary;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LODDistance1 = 1000.0f; // Switch to Medium LOD

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LODDistance2 = 2000.0f; // Switch to Low LOD

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullDistance = 3000.0f; // Disable effects beyond this

private:
    // Active effects tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Effect pooling
    UPROPERTY()
    TMap<EVFXType, TArray<UNiagaraComponent*>> EffectPool;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType VFXType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* AttachVFX(EVFXType VFXType, USceneComponent* AttachTo, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfType(EVFXType VFXType);

    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Atmosphere")
    void SetAtmosphericIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Atmosphere")
    void TriggerEnvironmentalEvent(EVFXType EventType, FVector Location, float Radius = 500.0f);

    // Dinosaur-specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurBreathing(AActor* Dinosaur, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void UpdateLODLevels(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

private:
    void InitializeVFXLibrary();
    void CleanupFinishedEffects();
    UNiagaraComponent* GetPooledEffect(EVFXType VFXType);
    void ReturnToPool(UNiagaraComponent* Effect, EVFXType VFXType);
    int32 CalculateLODLevel(float Distance) const;
};