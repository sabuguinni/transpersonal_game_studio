#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "VFX_BreathSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_BreathType : uint8
{
    None UMETA(DisplayName = "None"),
    ColdVapor UMETA(DisplayName = "Cold Vapor"),
    HotSteam UMETA(DisplayName = "Hot Steam"),
    Exhaustion UMETA(DisplayName = "Exhaustion"),
    Roar UMETA(DisplayName = "Roar Blast")
};

USTRUCT(BlueprintType)
struct FVFX_BreathSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
    EVFX_BreathType BreathType = EVFX_BreathType::ColdVapor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
    FLinearColor TintColor = FLinearColor::White;

    FVFX_BreathSettings()
    {
        BreathType = EVFX_BreathType::ColdVapor;
        Intensity = 1.0f;
        Duration = 2.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        TintColor = FLinearColor::White;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_BreathSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_BreathSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    FVFX_BreathSettings BreathSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    TSoftObjectPtr<UNiagaraSystem> ColdVaporEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    TSoftObjectPtr<UNiagaraSystem> HotSteamEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    TSoftObjectPtr<UNiagaraSystem> ExhaustionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    TSoftObjectPtr<UNiagaraSystem> RoarBlastEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    FVector BreathOffset = FVector(200.0f, 0.0f, 150.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    float BreathInterval = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    float TemperatureThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    bool bAutoBreathing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath System")
    bool bTemperatureBasedBreath = true;

    UFUNCTION(BlueprintCallable, Category = "Breath System")
    void TriggerBreathEffect(EVFX_BreathType BreathType, float Intensity = 1.0f, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Breath System")
    void StartBreathing();

    UFUNCTION(BlueprintCallable, Category = "Breath System")
    void StopBreathing();

    UFUNCTION(BlueprintCallable, Category = "Breath System")
    void SetBreathSettings(const FVFX_BreathSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Breath System")
    UNiagaraComponent* GetActiveBreathComponent() const { return ActiveBreathComponent; }

private:
    UPROPERTY()
    UNiagaraComponent* ActiveBreathComponent;

    float BreathTimer;
    bool bIsBreathing;

    void CreateBreathEffect(EVFX_BreathType BreathType);
    void UpdateBreathEffect(float DeltaTime);
    UNiagaraSystem* GetBreathEffectAsset(EVFX_BreathType BreathType);
    float GetCurrentTemperature();
};