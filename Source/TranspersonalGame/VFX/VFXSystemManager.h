#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment,     // Atmospheric effects, weather, ambient
    Creature,        // Dinosaur-specific effects
    Combat,          // Impact, damage, destruction
    Interaction,     // Player actions, tool use
    Emotional,       // Trust, fear, domestication indicators
    Survival         // Crafting, building, resource gathering
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,          // Barely noticeable, atmospheric
    Moderate,        // Clear but not overwhelming
    Dramatic,        // High impact, story moments
    Critical         // Emergency, danger, climax
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
    float BaseDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectsGameplay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EmotionalIntent;
};

/**
 * Central manager for all VFX systems in the game
 * Handles LOD, performance scaling, and emotional context
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXLODLevel(int32 LODLevel);

    // Emotional Context System
    UFUNCTION(BlueprintCallable, Category = "VFX|Emotion")
    void SetEmotionalContext(const FString& Context, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Emotion")
    void ModifyVFXForEmotion(UNiagaraComponent* VFXComponent, const FString& EmotionalState);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void UpdatePerformanceMetrics(float FrameTime, int32 ActiveParticleCount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    bool ShouldCullVFX(const FVector& VFXLocation, float VFXImportance) const;

protected:
    // VFX Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<FString, FVFXDefinition> VFXRegistry;

    // Active VFX tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 CurrentLODLevel = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceNear = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceFar = 2000.0f;

    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveParticles = 50000;

    UPROPERTY()
    float CurrentFrameTime = 16.67f;

    UPROPERTY()
    int32 CurrentActiveParticles = 0;

    // Emotional Context
    UPROPERTY()
    FString CurrentEmotionalContext = "Neutral";

    UPROPERTY()
    float CurrentEmotionalIntensity = 0.5f;

private:
    void InitializeVFXRegistry();
    void UpdateLODBasedOnPerformance();
    void CleanupInactiveVFX();
    int32 CalculateOptimalLOD(const FVector& VFXLocation) const;
};