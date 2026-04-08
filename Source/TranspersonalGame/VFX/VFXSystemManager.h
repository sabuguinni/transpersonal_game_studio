#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXSystemManager.generated.h"

/**
 * VFX System Manager - Centraliza todos os efeitos visuais do jogo
 * Responsável por LOD chain, pooling e performance dos VFX
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

    // === SISTEMAS CORE ===
    
    /** Sistema de LOD para VFX baseado em distância e performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance_Close = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance_Medium = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance_Far = 3000.0f;

    /** Pool de componentes Niagara para reutilização */
    UPROPERTY()
    TArray<UNiagaraComponent*> VFXPool_Environmental;
    
    UPROPERTY()
    TArray<UNiagaraComponent*> VFXPool_Combat;
    
    UPROPERTY()
    TArray<UNiagaraComponent*> VFXPool_Atmospheric;

    // === EFEITOS AMBIENTAIS ===
    
    /** Efeito de folhas caindo - sugere movimento de dinossauros */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> FallingLeaves_System;
    
    /** Poeira levantada por passos pesados */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> HeavyFootstep_Dust;
    
    /** Galhos quebrando - áudio visual */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> BreakingBranches_Debris;
    
    /** Pássaros fugindo em pânico */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> FlockPanic_Birds;

    // === EFEITOS ATMOSFÉRICOS ===
    
    /** Névoa densa para reduzir visibilidade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric VFX")
    TSoftObjectPtr<UNiagaraSystem> DenseFog_System;
    
    /** Raios de luz filtrados pela vegetação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric VFX")
    TSoftObjectPtr<UNiagaraSystem> VolumetricLightRays;
    
    /** Chuva torrencial para criar tensão */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric VFX")
    TSoftObjectPtr<UNiagaraSystem> TorrentialRain_System;

    // === EFEITOS DE SOBREVIVÊNCIA ===
    
    /** Fumo de fogueira - essencial para sobrevivência */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival VFX")
    TSoftObjectPtr<UNiagaraSystem> CampfireSmoke_System;
    
    /** Faíscas de ferramentas sendo criadas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival VFX")
    TSoftObjectPtr<UNiagaraSystem> ToolCrafting_Sparks;
    
    /** Sangue de ferimentos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival VFX")
    TSoftObjectPtr<UNiagaraSystem> BloodWounds_System;

    // === EFEITOS DE DINOSSAUROS ===
    
    /** Respiração visível de grandes predadores */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    TSoftObjectPtr<UNiagaraSystem> PredatorBreath_Steam;
    
    /** Pegadas na lama com água acumulando */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    TSoftObjectPtr<UNiagaraSystem> MudFootprints_Water;
    
    /** Saliva caindo de grandes carnívoros */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    TSoftObjectPtr<UNiagaraSystem> CarnivoreDrool_System;

public:
    // === FUNÇÕES PÚBLICAS ===
    
    /** Spawn de efeito com LOD automático */
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    UNiagaraComponent* SpawnVFXWithLOD(UNiagaraSystem* System, FVector Location, FRotator Rotation, float LifeTime = 5.0f);
    
    /** Sistema de pooling para performance */
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    UNiagaraComponent* GetPooledVFXComponent(EVFXPoolType PoolType);
    
    /** Retorna componente ao pool */
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void ReturnVFXToPool(UNiagaraComponent* Component, EVFXPoolType PoolType);
    
    /** Trigger de efeitos ambientais baseado em eventos */
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void TriggerEnvironmentalTension(FVector Location, float Intensity = 1.0f);
    
    /** Sistema de performance - ajusta qualidade baseado no framerate */
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void AdjustVFXQualityForPerformance();

private:
    /** Calcula LOD baseado na distância ao jogador */
    int32 CalculateLODLevel(FVector VFXLocation);
    
    /** Sistema de culling para VFX fora da tela */
    bool ShouldCullVFX(FVector VFXLocation);
    
    /** Monitoramento de performance */
    float CurrentFrameTime;
    int32 ActiveVFXCount;
    
    /** Referência ao jogador para cálculos de distância */
    UPROPERTY()
    APawn* PlayerPawn;
};

/** Enum para tipos de pool de VFX */
UENUM(BlueprintType)
enum class EVFXPoolType : uint8
{
    Environmental   UMETA(DisplayName = "Environmental"),
    Combat         UMETA(DisplayName = "Combat"),
    Atmospheric    UMETA(DisplayName = "Atmospheric"),
    Survival       UMETA(DisplayName = "Survival")
};