#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/DataTable.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Chuva, vento, folhas
    Creature,          // Sangue, respiração, pegadas
    Combat,            // Impactos, explosões, ferimentos
    Interaction,       // Domesticação, coleta, construção
    Atmospheric,       // Névoa, raios de sol, partículas de ar
    Destruction        // Quebra de objetos, desmoronamento
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,     // Quase imperceptível, atmosférico
    Moderate,   // Visível mas não dominante
    Prominent,  // Claro e intencional
    Dramatic    // Momentos climáticos apenas
};

USTRUCT(BlueprintType)
struct FVFXDefinition : public FTableRowBase
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
    float LODDistance_High = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance_Medium = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance_Low = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsVariation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VariationParameters;

    FVFXDefinition()
    {
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Moderate;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    UDataTable* VFXDatabase;

    // Sistema de LOD automático baseado em distância
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudget_GPU = 8.0f; // ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveLOD = true;

public:
    // Spawn de efeitos com LOD automático
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, 
                                         const FVector& Location, 
                                         const FRotator& Rotation = FRotator::ZeroRotator,
                                         const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(const FString& EffectName,
                                       USceneComponent* AttachComponent,
                                       const FName& AttachPointName = NAME_None,
                                       const FVector& Location = FVector::ZeroVector,
                                       const FRotator& Rotation = FRotator::ZeroRotator,
                                       const FVector& Scale = FVector::OneVector);

    // Sistema de variação procedural para criaturas únicas
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ApplyCreatureVariation(UNiagaraComponent* VFXComponent, int32 CreatureID);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentGPUBudgetUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceUpdateLOD();

private:
    TArray<UNiagaraComponent*> ActiveEffects;
    
    void UpdateLODSystem();
    int32 CalculateEffectLOD(const FVector& EffectLocation) const;
    void CleanupInactiveEffects();

    FTimerHandle LODUpdateTimer;
};