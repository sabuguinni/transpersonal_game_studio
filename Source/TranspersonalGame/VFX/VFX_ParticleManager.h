#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

// Enum para tipos de efeitos VFX
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DinosaurFootstep,
    DinosaurBreath,
    BloodSplatter,
    DustCloud,
    CampfireFire,
    CampfireSmoke,
    RainDrops,
    WindParticles,
    ImpactSparks,
    WaterSplash
};

// Struct para configuração de efeitos VFX
USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float SpawnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FLinearColor TintColor = FLinearColor::White;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::None;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 3.0f;
        bAutoDestroy = true;
        SpawnRate = 1.0f;
        TintColor = FLinearColor::White;
    }
};

// Struct para efeitos activos
USTRUCT(BlueprintType)
struct FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effect")
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effect")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effect")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effect")
    float TimeRemaining = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effect")
    bool bIsActive = false;

    FVFX_ActiveEffect()
    {
        NiagaraComponent = nullptr;
        EffectType = EVFX_EffectType::None;
        Location = FVector::ZeroVector;
        TimeRemaining = 0.0f;
        bIsActive = false;
    }
};

/**
 * Sistema de gestão de efeitos visuais usando Niagara
 * Responsável por spawnar, gerir e limpar efeitos VFX no mundo
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Spawnar efeito VFX numa localização específica
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, const FVFX_EffectConfig& Config = FVFX_EffectConfig());

    // Spawnar efeito de pegada de dinossauro
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool SpawnDinosaurFootstepVFX(FVector Location, EDinosaurSpecies DinosaurType, float FootSize = 1.0f);

    // Spawnar efeito de fogueira
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool SpawnCampfireVFX(FVector Location, bool bIncludeSmoke = true);

    // Spawnar efeito de impacto/sangue
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool SpawnBloodSplatterVFX(FVector Location, FVector ImpactDirection, float Intensity = 1.0f);

    // Spawnar efeito de poeira
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool SpawnDustCloudVFX(FVector Location, float Size = 1.0f, FLinearColor DustColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f));

    // Parar todos os efeitos de um tipo específico
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffectsByType(EVFX_EffectType EffectType);

    // Parar todos os efeitos VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Obter número de efeitos activos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectsCount() const;

    // Verificar se um tipo de efeito está activo
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectTypeActive(EVFX_EffectType EffectType) const;

protected:
    // Array de efeitos activos
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    TArray<FVFX_ActiveEffect> ActiveEffects;

    // Configurações por defeito para cada tipo de efeito
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFX_EffectType, FVFX_EffectConfig> DefaultEffectConfigs;

    // Limite máximo de efeitos simultâneos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxActiveEffects = 50;

    // Distância máxima do jogador para spawnar efeitos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance", meta = (ClampMin = "100", ClampMax = "10000"))
    float MaxSpawnDistance = 5000.0f;

    // Timer para limpeza de efeitos
    FTimerHandle CleanupTimerHandle;

private:
    // Função interna para criar componente Niagara
    UNiagaraComponent* CreateNiagaraComponent(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, const FVFX_EffectConfig& Config);

    // Limpar efeitos expirados
    UFUNCTION()
    void CleanupExpiredEffects();

    // Validar se a localização está dentro dos limites do mapa
    bool IsLocationValid(const FVector& Location) const;

    // Obter configuração por defeito para um tipo de efeito
    FVFX_EffectConfig GetDefaultConfigForEffect(EVFX_EffectType EffectType) const;

    // Inicializar configurações por defeito
    void InitializeDefaultConfigs();

    // Verificar se está próximo do jogador
    bool IsNearPlayer(const FVector& Location) const;
};