#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXManager.generated.h"

/**
 * VFX Manager - Sistema central de controlo de efeitos visuais
 * Gere performance, LOD chain e activação contextual de efeitos
 * Baseado na filosofia: "O melhor VFX é aquele que o jogador nem sabe que viu"
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

    // === CORE VFX SYSTEMS ===
    
    /** Sistema de LOD automático para VFX baseado em distância e performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float VFXLODDistance_High = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float VFXLODDistance_Medium = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float VFXLODDistance_Low = 3000.0f;

    /** Limite máximo de sistemas VFX activos simultaneamente */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveVFXSystems = 50;

    /** Pool de componentes Niagara reutilizáveis para performance */
    UPROPERTY()
    TArray<UNiagaraComponent*> VFXPool;

    /** Sistemas VFX activos no mundo */
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXSystems;

public:
    // === VFX SPAWNING FUNCTIONS ===
    
    /** Spawn VFX com LOD automático e gestão de performance */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(
        UNiagaraSystem* VFXSystem,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector::OneVector,
        bool bAutoDestroy = true,
        float LifeTime = 5.0f
    );

    /** Spawn VFX attachado a um actor */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(
        UNiagaraSystem* VFXSystem,
        USceneComponent* AttachToComponent,
        FName AttachPointName = NAME_None,
        FVector Location = FVector::ZeroVector,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector::OneVector,
        bool bAutoDestroy = true,
        float LifeTime = 5.0f
    );

    // === ENVIRONMENTAL VFX ===
    
    /** Sistema de chuva adaptativo baseado no clima */
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetRainIntensity(float Intensity);
    
    /** Efeitos de vento nas folhas e vegetação */
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWindStrength(float WindStrength, FVector WindDirection);

    /** Partículas de poeira atmosférica para profundidade */
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetAtmosphericDust(float Density, FLinearColor Color);

    // === DINOSAUR SPECIFIC VFX ===
    
    /** Efeitos de respiração para dinossauros */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    UNiagaraComponent* SpawnDinosaurBreath(
        AActor* DinosaurActor,
        float BreathIntensity,
        FLinearColor BreathColor
    );

    /** Efeitos de pegadas no solo */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SpawnFootstepVFX(
        FVector FootLocation,
        float DinosaurWeight,
        int32 SurfaceType
    );

    /** Efeitos de domesticação (partículas calmantes) */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    UNiagaraComponent* SpawnTamingVFX(AActor* DinosaurActor, float TamingProgress);

    // === SURVIVAL VFX ===
    
    /** Sistema de fogo para fogueiras e tochas */
    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    UNiagaraComponent* SpawnFireVFX(
        FVector Location,
        float FireIntensity,
        bool bIsTorch = false
    );

    /** Efeitos de impacto de ferramentas */
    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    void SpawnToolImpactVFX(
        FVector ImpactLocation,
        FVector ImpactNormal,
        int32 MaterialType,
        float ImpactForce
    );

    /** Efeitos de água (gotejamento, respingos) */
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnWaterVFX(
        FVector Location,
        int32 WaterType, // 0=gotejamento, 1=respingo, 2=corrente
        float Intensity
    );

private:
    // === PERFORMANCE MANAGEMENT ===
    
    /** Actualiza LOD de todos os VFX activos */
    void UpdateVFXLOD();
    
    /** Remove VFX expirados da pool */
    void CleanupExpiredVFX();
    
    /** Calcula LOD baseado na distância ao jogador */
    int32 CalculateVFXLOD(FVector VFXLocation);
    
    /** Obtém componente da pool ou cria novo */
    UNiagaraComponent* GetPooledVFXComponent();
    
    /** Retorna componente à pool */
    void ReturnToPool(UNiagaraComponent* Component);

    // === INTERNAL SYSTEMS ===
    
    /** Referência ao jogador para cálculos de LOD */
    UPROPERTY()
    APawn* PlayerPawn;
    
    /** Timer para limpeza periódica */
    FTimerHandle CleanupTimer;
    
    /** Contador de performance para debug */
    UPROPERTY(VisibleAnywhere, Category = "Debug")
    int32 ActiveVFXCount;
    
    UPROPERTY(VisibleAnywhere, Category = "Debug")
    int32 PooledVFXCount;
};