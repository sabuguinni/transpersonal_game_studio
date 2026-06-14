#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_DamageFlashController.generated.h"

UENUM(BlueprintType)
enum class EAudio_DamageType : uint8
{
    Light       UMETA(DisplayName = "Light Damage"),
    Medium      UMETA(DisplayName = "Medium Damage"),
    Heavy       UMETA(DisplayName = "Heavy Damage"),
    Critical    UMETA(DisplayName = "Critical Damage")
};

USTRUCT(BlueprintType)
struct FAudio_DamageFlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float FlashIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float FadeOutTime = 0.2f;

    FAudio_DamageFlashSettings()
    {
        FlashIntensity = 0.5f;
        FlashDuration = 0.3f;
        FlashColor = FLinearColor::Red;
        FadeOutTime = 0.2f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DamageFlashController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DamageFlashController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main flash trigger function
    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerDamageFlash(EAudio_DamageType DamageType, float CustomIntensity = -1.0f);

    // Direct flash with custom settings
    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerCustomFlash(const FAudio_DamageFlashSettings& FlashSettings);

    // Stop current flash immediately
    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void StopFlash();

    // Check if flash is currently active
    UFUNCTION(BlueprintPure, Category = "Damage Flash")
    bool IsFlashing() const { return bIsFlashing; }

protected:
    // Flash settings for different damage types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Configuration")
    FAudio_DamageFlashSettings LightDamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Configuration")
    FAudio_DamageFlashSettings MediumDamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Configuration")
    FAudio_DamageFlashSettings HeavyDamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Configuration")
    FAudio_DamageFlashSettings CriticalDamageSettings;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Flash State")
    bool bIsFlashing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Flash State")
    float CurrentFlashAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Flash State")
    float FlashTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Flash State")
    FAudio_DamageFlashSettings ActiveFlashSettings;

private:
    // Internal flash management
    void UpdateFlash(float DeltaTime);
    void EndFlash();
    FAudio_DamageFlashSettings GetFlashSettingsForDamageType(EAudio_DamageType DamageType) const;

    // Timer handle for flash duration
    FTimerHandle FlashTimerHandle;
};

#include "Audio_DamageFlashController.generated.h"