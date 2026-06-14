#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "VFX_FireSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_FireIntensity : uint8
{
    Ember       UMETA(DisplayName = "Ember"),
    Small       UMETA(DisplayName = "Small Fire"),
    Medium      UMETA(DisplayName = "Medium Fire"),
    Large       UMETA(DisplayName = "Large Fire"),
    Inferno     UMETA(DisplayName = "Inferno")
};

USTRUCT(BlueprintType)
struct FVFX_FireParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    EVFX_FireIntensity Intensity = EVFX_FireIntensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float FlameHeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float FlameWidth = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SmokeAmount = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmberCount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    FLinearColor FlameColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_FireSystem : public AActor
{
    GENERATED_BODY()

public:
    AVFX_FireSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Fire system components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FireBaseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* FlameParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* SmokeParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* EmberParticles;

    // Fire parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    FVFX_FireParams FireParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    bool bAutoStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    bool bFlickerEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float FlickerSpeed = 2.0f;

    // Fire control functions
    UFUNCTION(BlueprintCallable, Category = "Fire VFX")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Fire VFX")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Fire VFX")
    void SetFireIntensity(EVFX_FireIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Fire VFX")
    void UpdateFireParameters(const FVFX_FireParams& NewParams);

    UFUNCTION(BlueprintCallable, Category = "Fire VFX")
    bool IsFireActive() const;

private:
    void InitializeFireComponents();
    void UpdateFlickerEffect(float DeltaTime);
    void ApplyFireParameters();

    // Internal state
    bool bFireActive = false;
    float FlickerTimer = 0.0f;
    float BaseFlameHeight = 1.0f;
};