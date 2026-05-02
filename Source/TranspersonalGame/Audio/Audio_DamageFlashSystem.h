#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/PostProcessComponent.h"
#include "Audio_DamageFlashSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_DamageType : uint8
{
    Light      UMETA(DisplayName = "Light Damage"),
    Medium     UMETA(DisplayName = "Medium Damage"),
    Heavy      UMETA(DisplayName = "Heavy Damage"),
    Critical   UMETA(DisplayName = "Critical Damage"),
    Death      UMETA(DisplayName = "Death")
};

USTRUCT(BlueprintType)
struct FAudio_DamageFlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
    float FlashIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
    float FadeOutTime = 0.2f;

    FAudio_DamageFlashSettings()
    {
        FlashDuration = 0.3f;
        FlashIntensity = 0.5f;
        FlashColor = FLinearColor::Red;
        FadeOutTime = 0.2f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DamageFlashSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DamageFlashSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Trigger damage flash effect
    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void TriggerDamageFlash(EAudio_DamageType DamageType);

    // Trigger custom damage flash
    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void TriggerCustomFlash(FAudio_DamageFlashSettings FlashSettings);

    // Stop current flash effect
    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void StopFlash();

    // Check if flash is currently active
    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    bool IsFlashActive() const { return bIsFlashActive; }

protected:
    // Flash settings for different damage types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    TMap<EAudio_DamageType, FAudio_DamageFlashSettings> FlashPresets;

    // Current flash state
    UPROPERTY()
    bool bIsFlashActive = false;

    UPROPERTY()
    float CurrentFlashTime = 0.0f;

    UPROPERTY()
    float CurrentFlashDuration = 0.0f;

    UPROPERTY()
    FAudio_DamageFlashSettings CurrentFlashSettings;

    // Cache player controller
    UPROPERTY()
    APlayerController* CachedPlayerController;

    // Initialize default flash presets
    void InitializeFlashPresets();

    // Update flash effect
    void UpdateFlashEffect(float DeltaTime);

    // Apply flash to screen
    void ApplyFlashEffect(float FlashAlpha);
};