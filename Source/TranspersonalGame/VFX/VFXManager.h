#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0,    // Sempre renderizado (morte, descobertas importantes)
    High = 1,        // Alta prioridade (combate, predadores próximos)
    Medium = 2,      // Prioridade média (ambiente, herbívoros)
    Low = 3,         // Baixa prioridade (detalhes distantes)
    Ambient = 4      // Ambiente geral (pode ser desligado para performance)
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Combat,          // Efeitos de combate e violência
    Environmental,   // Ambiente, clima, vegetação
    Creature,        // Efeitos específicos de dinossauros
    Discovery,       // Descobertas, itens importantes
    Atmospheric,     // Névoa, partículas de ar, luz
    Interactive      // Efeitos de interação do jogador
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    FVFXDefinition()
    {
        Priority = EVFXPriority::Medium;
        Category = EVFXCategory::Environmental;
    }
};

/**
 * Sistema central de gerenciamento de VFX
 * Controla LOD, pooling e performance de todos os efeitos visuais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Sistema de LOD baseado em distância e performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance_High = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance_Medium = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance_Low = 5000.0f;

    // Limites de performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    // Catálogo de efeitos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<FString, FVFXDefinition> VFXLibrary;

public:
    // Interface principal para spawnar efeitos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFX(const FString& EffectName, 
                                     const FVector& Location, 
                                     const FRotator& Rotation = FRotator::ZeroRotator,
                                     const FVector& Scale = FVector::OneVector,
                                     AActor* AttachToActor = nullptr);

    // Sistema de LOD automático
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLOD();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentVFXLoad() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(bool bLowPerformanceMode);

    // Efeitos específicos do jogo
    UFUNCTION(BlueprintCallable, Category = "Game VFX")
    void PlayDinosaurFootstep(const FVector& Location, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "Game VFX")
    void PlayBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Game VFX")
    void PlayEnvironmentalDisturbance(const FVector& Location, float Radius);

private:
    // Pool de componentes para reutilização
    UPROPERTY()
    TArray<UNiagaraComponent*> AvailableComponents;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveComponents;

    // Referência ao jogador para cálculos de distância
    UPROPERTY()
    APawn* PlayerPawn;

    // Performance tracking
    float CurrentFrameTime;
    int32 ActiveEffectCount;
    bool bLowPerformanceMode;

    // Métodos internos
    UNiagaraComponent* GetPooledComponent();
    void ReturnComponentToPool(UNiagaraComponent* Component);
    int32 GetLODLevel(float Distance) const;
    void CullDistantEffects();
};