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
    Environment,    // Chuva, vento, folhas
    Combat,         // Sangue, impactos, destruição
    Survival,       // Fogo, fumo, faíscas
    Mystical,       // Gema, energia sobrenatural
    Creature        // Efeitos específicos de dinossauros
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,         // Efeitos ambientes constantes
    Moderate,       // Efeitos de gameplay normal
    Intense,        // Momentos de alta tensão
    Cinematic       // Cutscenes e momentos épicos
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
    float MaxViewDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoDestroyDelay = 5.0f;
};

/**
 * Sistema central de gerenciamento de VFX
 * Controla spawning, pooling, LOD e performance de todos os efeitos visuais
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
    // Sistema de spawning de VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXAtLocation(EVFXCategory Category, const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXAttached(EVFXCategory Category, const FString& EffectName, AActor* AttachToActor, const FName& SocketName = NAME_None);

    // Sistema de pooling para performance
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ReturnVFXToPool(UNiagaraComponent* VFXComponent);

    // Sistema de LOD baseado em distância
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLOD();

    // Controle de qualidade global
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXQuality(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    // Sistema de batch para múltiplos efeitos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXBatch(const TArray<FString>& EffectNames, const TArray<FVector>& Locations);

protected:
    // Registry de todos os VFX disponíveis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TMap<FString, FVFXDefinition> VFXRegistry;

    // Pool de componentes reutilizáveis
    UPROPERTY()
    TMap<FString, TArray<UNiagaraComponent*>> VFXPool;

    // VFX ativos no mundo
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFX;

    // Configurações de performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VFXCullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel = 2; // High por default

    // Referência ao player para cálculos de distância
    UPROPERTY()
    APawn* PlayerPawn;

private:
    void InitializeVFXRegistry();
    void CleanupExpiredVFX();
    UNiagaraComponent* GetPooledVFX(const FString& EffectName);
    void CreatePooledVFX(const FString& EffectName, int32 PoolSize = 5);
    float GetDistanceToPlayer(const FVector& Location);
    int32 CalculateLODLevel(float Distance);
};