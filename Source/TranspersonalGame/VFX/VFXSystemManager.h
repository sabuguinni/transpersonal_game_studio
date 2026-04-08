#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Fog, dust, ambient particles
    Creature,          // Dinosaur breath, footsteps, presence
    Combat,            // Blood, impacts, destruction
    Interaction,       // Domestication feedback, tool use
    Atmospheric,       // Weather, lighting effects
    Narrative          // Story moments, gema effects
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,            // Background ambience
    Moderate,          // Normal gameplay
    Dramatic,          // Key moments
    Cinematic          // Story beats
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
    float BaseCost = 1.0f; // Performance cost multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxViewDistance = 5000.0f;
};

/**
 * Central manager for all VFX systems in the Jurassic survival game
 * Handles LOD, performance scaling, and narrative-driven effects
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

    // VFX Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TArray<FVFXDefinition> RegisteredEffects;

    // Performance Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudget = 100.0f; // Total cost allowed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentPerformanceCost = 0.0f;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance_High = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance_Medium = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance_Low = 5000.0f;

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TMap<FString, int32> EffectCounts;

public:
    // Main Interface Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(const FString& EffectName, const FVector& Location, 
                                     const FRotator& Rotation = FRotator::ZeroRotator, 
                                     AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffectsOfCategory(EVFXCategory Category);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CanSpawnEffect(const FString& EffectName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceProfile(int32 ProfileLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    // Narrative Integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeVFX(const FString& MomentName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetAtmosphericIntensity(float Intensity); // 0.0 to 1.0

    // Creature-Specific VFX
    UFUNCTION(BlueprintCallable, Category = "Creatures")
    void PlayCreaturePresenceVFX(AActor* Creature, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Creatures")
    void PlayDomesticationFeedbackVFX(AActor* Creature, float TrustLevel);

protected:
    // Internal Management
    void CleanupFinishedEffects();
    void ApplyLODToEffect(UNiagaraComponent* Effect, float Distance);
    FVFXDefinition* FindEffectDefinition(const FString& EffectName);
    float CalculateEffectCost(const FVFXDefinition& Definition, float Distance) const;
};