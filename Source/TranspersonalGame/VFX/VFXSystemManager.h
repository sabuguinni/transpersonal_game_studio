#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Chuva, vento, folhas caindo
    Survival,          // Fogo de acampamento, fumaça, vapor
    DinosaurInteraction, // Pegadas, respiração, saliva
    Combat,            // Sangue, impactos, ferimentos
    Atmospheric,       // Neblina, raios de sol, partículas de ar
    Destruction,       // Madeira quebrando, pedras caindo
    Magic             // Efeito da gema temporal
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,    // Quase imperceptível, atmosférico
    Medium,    // Visível mas não dominante
    High,      // Chamativo, momentos importantes
    Cinematic  // Máxima qualidade, momentos únicos
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
    FVector DefaultScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAttachToActor = false;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LODLevel = 0; // 0=High, 1=Medium, 2=Low

    FVFXDefinition()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Medium;
    }
};

/**
 * Sistema central de gerenciamento de efeitos visuais
 * Controla spawning, pooling e performance de todos os VFX
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

    // Registry de todos os efeitos disponíveis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TArray<FVFXDefinition> VFXRegistry;

    // Pool de componentes reutilizáveis
    UPROPERTY()
    TArray<UNiagaraComponent*> AvailableComponents;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveComponents;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGPUTime = 2.0f; // milliseconds

public:
    // Spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(const FString& EffectName, USceneComponent* AttachComponent, const FVector& LocationOffset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfCategory(EVFXCategory Category);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 Level); // 0=Ultra, 1=High, 2=Medium, 3=Low

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentGPUTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectCount() const { return ActiveComponents.Num(); }

private:
    UNiagaraComponent* GetPooledComponent();
    void ReturnComponentToPool(UNiagaraComponent* Component);
    void PerformanceCheck();
    void OptimizeActiveEffects();

    FTimerHandle PerformanceCheckTimer;
    float LastGPUTime = 0.0f;
    int32 CurrentPerformanceLevel = 1; // Default: High
};