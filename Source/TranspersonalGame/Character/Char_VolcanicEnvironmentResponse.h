#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Char_VolcanicEnvironmentResponse.generated.h"

UENUM(BlueprintType)
enum class EChar_HeatExposureLevel : uint8
{
    None        UMETA(DisplayName = "No Heat"),
    Mild        UMETA(DisplayName = "Mild Heat"),
    Moderate    UMETA(DisplayName = "Moderate Heat"),
    Severe      UMETA(DisplayName = "Severe Heat"),
    Extreme     UMETA(DisplayName = "Extreme Heat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VolcanicExposureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Response")
    float HeatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Response")
    float ExposureDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Response")
    EChar_HeatExposureLevel ExposureLevel = EChar_HeatExposureLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Response")
    bool bIsInVolcanicZone = false;

    FChar_VolcanicExposureData()
    {
        HeatIntensity = 0.0f;
        ExposureDuration = 0.0f;
        ExposureLevel = EChar_HeatExposureLevel::None;
        bIsInVolcanicZone = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_VolcanicEnvironmentResponse : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_VolcanicEnvironmentResponse();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Heat exposure tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Response")
    FChar_VolcanicExposureData CurrentExposure;

    // Heat damage thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Damage")
    float MildHeatThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Damage")
    float ModerateHeatThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Damage")
    float SevereHeatThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Damage")
    float ExtremeHeatThreshold = 100.0f;

    // Character material response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Response")
    float SweatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Response")
    float SkinGlowIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Response")
    bool bShowHeatDistortion = false;

    // Movement penalties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Response")
    float HeatMovementPenalty = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Response")
    float MaxHeatMovementPenalty = 0.5f;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Response")
    void UpdateHeatExposure(float HeatIntensity, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Response")
    void SetVolcanicZoneStatus(bool bInZone);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Response")
    EChar_HeatExposureLevel GetCurrentHeatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Response")
    float GetHeatDamagePerSecond() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Response")
    void ApplyHeatDamage(float DeltaTime);

private:
    void UpdateVisualEffects();
    void UpdateMovementPenalties();
    void CalculateHeatLevel();

    // Internal timers
    float HeatDamageTimer = 0.0f;
    float VisualUpdateTimer = 0.0f;
    
    // Constants
    static constexpr float HEAT_DAMAGE_INTERVAL = 1.0f;
    static constexpr float VISUAL_UPDATE_INTERVAL = 0.1f;
};