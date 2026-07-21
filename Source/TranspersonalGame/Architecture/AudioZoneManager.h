#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EArch_AudioZoneType : uint8
{
    Cave        UMETA(DisplayName = "Cave"),
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Structure   UMETA(DisplayName = "Structure"),
    Water       UMETA(DisplayName = "Water"),
    Underground UMETA(DisplayName = "Underground")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AudioZoneSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EArch_AudioZoneType ZoneType = EArch_AudioZoneType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bUseReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float ReverbIntensity = 1.0f;

    FArch_AudioZoneSettings()
    {
        ZoneType = EArch_AudioZoneType::Plains;
        Volume = 0.5f;
        Pitch = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bUseReverb = true;
        ReverbIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FArch_AudioZoneSettings ZoneSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    int32 Priority = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone")
    TArray<AActor*> ActorsInZone;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneSettings(const FArch_AudioZoneSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    FArch_AudioZoneSettings GetZoneSettings() const { return ZoneSettings; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeIn(float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeOut(float Duration = 2.0f);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* Player);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateAudioSettings();
    void ApplyReverbSettings();

private:
    bool bIsFading = false;
    float FadeStartTime = 0.0f;
    float FadeDuration = 0.0f;
    float FadeStartVolume = 0.0f;
    float FadeTargetVolume = 0.0f;
};