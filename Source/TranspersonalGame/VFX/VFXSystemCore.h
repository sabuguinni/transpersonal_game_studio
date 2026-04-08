#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemCore.generated.h"

/**
 * Core VFX System for Transpersonal Game Studio
 * Manages all visual effects with 3-tier LOD system for performance
 * Designed for prehistoric survival atmosphere with dynamic scaling
 */

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality - Close Range"),
    Medium      UMETA(DisplayName = "Medium Quality - Mid Range"), 
    Low         UMETA(DisplayName = "Low Quality - Far Range"),
    Disabled    UMETA(DisplayName = "Disabled - Performance Mode")
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment     UMETA(DisplayName = "Environmental Effects"),
    Creature        UMETA(DisplayName = "Creature Effects"),
    Combat          UMETA(DisplayName = "Combat Effects"),
    Atmosphere      UMETA(DisplayName = "Atmospheric Effects"),
    Player          UMETA(DisplayName = "Player Effects"),
    Destruction     UMETA(DisplayName = "Destruction Effects")
};

USTRUCT(BlueprintType)
struct FVFXLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HighQualityDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MediumQualityDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowQualityDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateFrequency = 0.1f;
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Core")
    class UNiagaraComponent* SpawnVFX(UNiagaraSystem* VFXSystem, FVector Location, FRotator Rotation = FRotator::ZeroRotator, EVFXCategory Category = EVFXCategory::Environment);

    UFUNCTION(BlueprintCallable, Category = "VFX Core")
    void SetVFXLOD(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Core")
    EVFXLODLevel CalculateLODLevel(FVector VFXLocation, FVector ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Core")
    void UpdateAllVFXLOD();

    // Atmospheric VFX
    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetAtmosphericTension(float TensionLevel); // 0.0 = calm, 1.0 = maximum tension

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void TriggerCreaturePresenceEffect(FVector Location, float IntensityRadius);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(bool bEnablePerformanceMode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveVFXCount() const { return ActiveVFXComponents.Num(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Settings")
    FVFXLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Settings")
    bool bEnableAtmosphericEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Settings")
    float AtmosphericTensionLevel = 0.3f;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TMap<EVFXCategory, int32> CategoryCounts;

    float LastLODUpdateTime = 0.0f;
    
    void CleanupInactiveVFX();
    void ApplyLODToComponent(UNiagaraComponent* Component, EVFXLODLevel LODLevel);
};