#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Audio_DamageFlashEffect.generated.h"

UENUM(BlueprintType)
enum class EAudio_DamageType : uint8
{
    Physical UMETA(DisplayName = "Physical Damage"),
    Fire UMETA(DisplayName = "Fire Damage"),
    Poison UMETA(DisplayName = "Poison Damage"),
    Bleeding UMETA(DisplayName = "Bleeding Damage"),
    Critical UMETA(DisplayName = "Critical Hit")
};

USTRUCT(BlueprintType)
struct FAudio_FlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float MaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    float FadeSpeed = 3.0f;

    FAudio_FlashSettings()
    {
        FlashColor = FLinearColor::Red;
        FlashDuration = 0.3f;
        MaxOpacity = 0.6f;
        FadeSpeed = 3.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DamageFlashEffect : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DamageFlashEffect();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* FlashMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Settings")
    TMap<EAudio_DamageType, FAudio_FlashSettings> DamageFlashPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseDamageFlashMaterial;

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicFlashMaterial;

    // Flash state variables
    UPROPERTY()
    bool bIsFlashing;

    UPROPERTY()
    float CurrentFlashTime;

    UPROPERTY()
    float FlashDuration;

    UPROPERTY()
    float MaxFlashOpacity;

    UPROPERTY()
    float FadeSpeed;

    UPROPERTY()
    FLinearColor CurrentFlashColor;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerDamageFlash(EAudio_DamageType DamageType, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void TriggerCustomFlash(FLinearColor FlashColor, float Duration, float Opacity);

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    void StopFlash();

    UFUNCTION(BlueprintCallable, Category = "Damage Flash")
    bool IsCurrentlyFlashing() const { return bIsFlashing; }

private:
    void InitializeFlashPresets();
    void UpdateFlashEffect(float DeltaTime);
    void SetFlashOpacity(float Opacity);
    float CalculateDamageIntensity(float DamageAmount);
};