#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "VFX_BloodSystem.generated.h"

// Enum para tipos de sangue
UENUM(BlueprintType)
enum class EVFX_BloodType : uint8
{
    Human       UMETA(DisplayName = "Human Blood"),
    Dinosaur    UMETA(DisplayName = "Dinosaur Blood"),
    SmallAnimal UMETA(DisplayName = "Small Animal Blood")
};

// Struct para configuração de sangue
USTRUCT(BlueprintType)
struct FVFX_BloodConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    EVFX_BloodType BloodType = EVFX_BloodType::Human;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float BloodIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector BloodDirection = FVector(0, 0, -1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    bool bShouldStainSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float StainDuration = 30.0f;

    FVFX_BloodConfig()
    {
        BloodType = EVFX_BloodType::Human;
        BloodIntensity = 1.0f;
        BloodDirection = FVector(0, 0, -1);
        bShouldStainSurface = true;
        StainDuration = 30.0f;
    }
};

/**
 * Sistema de VFX de sangue para combate realista pré-histórico
 * Gere efeitos de sangue, manchas e sons de impacto
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_BloodSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_BloodSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Sistema principal de sangue
    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void SpawnBloodEffect(const FVector& Location, const FVFX_BloodConfig& BloodConfig);

    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void SpawnBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void CreateBloodStain(const FVector& Location, float StainSize = 1.0f, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void PlayBloodSound(const FVector& Location, EVFX_BloodType BloodType);

    // Gestão de manchas temporárias
    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void CleanupOldBloodStains();

    UFUNCTION(BlueprintCallable, Category = "VFX Blood")
    void SetBloodSystemEnabled(bool bEnabled);

protected:
    // Assets Niagara para diferentes tipos de sangue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> HumanBloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> DinosaurBloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> SmallAnimalBloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodStainEffect;

    // Sons de sangue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TSoftObjectPtr<USoundBase> BloodSplatterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TSoftObjectPtr<USoundBase> BloodImpactSound;

    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood Settings")
    bool bBloodSystemEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood Settings")
    float MaxBloodStains = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood Settings")
    float BloodCleanupInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood Settings")
    float DefaultStainDuration = 30.0f;

    // Gestão interna
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveBloodStains;

    UPROPERTY()
    TArray<float> BloodStainTimers;

    float BloodCleanupTimer = 0.0f;

private:
    // Métodos internos
    UNiagaraSystem* GetBloodEffectForType(EVFX_BloodType BloodType);
    void ValidateBloodLocation(const FVector& Location);
    void RemoveOldestBloodStain();
};