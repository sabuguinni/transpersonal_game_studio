#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

/**
 * VFX System Manager - Centralized control for all visual effects
 * Handles LOD chains, performance scaling, and contextual VFX triggers
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> EnvironmentalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature VFX")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> CreatureEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival VFX")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> SurvivalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric VFX")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> AtmosphericEffects;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighQualityDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumQualityDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowQualityDistance = 10000.0f;

    // Performance Scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance = 15000.0f;

    // VFX Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnEnvironmentalEffect(const FString& EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnCreatureEffect(const FString& EffectName, AActor* TargetActor, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnSurvivalEffect(const FString& EffectName, FVector Location, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetAtmosphericIntensity(float Intensity);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVFXLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEffects(FVector PlayerLocation);

private:
    // Active effect tracking
    TArray<UNiagaraComponent*> ActiveEffects;
    
    // Performance monitoring
    float LastLODUpdateTime = 0.0f;
    float LODUpdateInterval = 0.5f; // Update LOD twice per second

    // Helper functions
    ENiagaraQualityLevel GetQualityLevelForDistance(float Distance);
    void CleanupFinishedEffects();
};