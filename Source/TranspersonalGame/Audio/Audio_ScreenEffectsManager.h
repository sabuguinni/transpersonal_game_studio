#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_ScreenEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FalloffRadius = 2000.0f;

    FAudio_ScreenShakeData()
    {
        Intensity = 1.0f;
        Duration = 2.0f;
        Frequency = 10.0f;
        FalloffRadius = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    FAudio_DamageFlashData()
    {
        FlashIntensity = 0.8f;
        FlashDuration = 0.3f;
        FlashColor = FLinearColor::Red;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenEffectsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Screen shake system
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerScreenShake(const FVector& EpicenterLocation, const FAudio_ScreenShakeData& ShakeData);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerTRexFootstepShake(const FVector& FootstepLocation);

    // Damage flash system
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerDamageFlash(const FAudio_DamageFlashData& FlashData);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerPlayerDamageFlash(float DamageAmount);

    // Footstep particle system
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void SpawnFootstepParticles(const FVector& Location, float ParticleScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void SpawnDinosaurFootstepParticles(const FVector& Location, const FString& DinosaurType);

protected:
    // Screen shake properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ScreenShakeData DefaultTRexShakeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ScreenShakeData DefaultPlayerShakeData;

    // Damage flash properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash", meta = (AllowPrivateAccess = "true"))
    FAudio_DamageFlashData DefaultDamageFlashData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Flash", meta = (AllowPrivateAccess = "true"))
    UMaterialInstanceDynamic* DamageFlashMaterial;

    // Particle system references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles", meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* FootstepDustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles", meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* DinosaurFootstepParticles;

    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal", meta = (AllowPrivateAccess = "true"))
    bool bDamageFlashActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal", meta = (AllowPrivateAccess = "true"))
    float CurrentFlashAlpha;

    FTimerHandle DamageFlashTimerHandle;

private:
    // Internal methods
    void UpdateDamageFlash();
    void EndDamageFlash();
    float CalculateShakeIntensityByDistance(const FVector& EpicenterLocation, float MaxRadius) const;
    void ApplyScreenShakeToPlayer(float Intensity, float Duration, float Frequency);
};