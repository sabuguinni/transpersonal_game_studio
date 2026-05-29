#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Audio_DamageFlashSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    bool bPlayDamageSound = true;

    FAudio_DamageFlashData()
    {
        FlashDuration = 0.3f;
        FlashIntensity = 0.8f;
        FlashColor = FLinearColor::Red;
        bPlayDamageSound = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DamageFlashSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DamageFlashSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ScreenOverlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* CriticalDamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* FlashMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FAudio_DamageFlashData DefaultFlashData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FAudio_DamageFlashData CriticalFlashData;

private:
    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicFlashMaterial;

    float CurrentFlashTime;
    float FlashDuration;
    bool bIsFlashing;
    FLinearColor FlashColor;
    float FlashIntensity;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerDamageFlash(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerCriticalDamageFlash();

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerCustomFlash(const FAudio_DamageFlashData& FlashData);

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void StopFlash();

private:
    void StartFlash(const FAudio_DamageFlashData& FlashData);
    void UpdateFlash(float DeltaTime);
    void PlayDamageAudio(bool bIsCritical);
};