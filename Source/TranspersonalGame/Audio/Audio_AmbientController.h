#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Audio_SoundManager.h"
#include "Audio_AmbientController.generated.h"

USTRUCT(BlueprintType)
struct FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Zone")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Zone")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Zone")
    float Volume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Zone")
    bool bActive = true;

    FAudio_AmbientZone()
    {
        BiomeType = EAudio_BiomeType::Savanna;
        Radius = 5000.0f;
        Volume = 0.8f;
        bActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Settings")
    FAudio_AmbientZone AmbientZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio References")
    TSoftObjectPtr<class USoundCue> AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bDayNightVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    TSoftObjectPtr<class USoundCue> NightAmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bWeatherVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TSoftObjectPtr<class USoundCue> RainAmbientSoundCue;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void ActivateAmbientZone();

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void DeactivateAmbientZone();

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void SetAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void SwitchToNightAmbient();

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void SwitchToDayAmbient();

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void EnableWeatherAmbient(bool bRaining);

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    UPROPERTY()
    bool bPlayerInZone = false;

    UPROPERTY()
    float CurrentVolume = 0.0f;

    UPROPERTY()
    float TargetVolume = 0.0f;

    UPROPERTY()
    bool bIsNightTime = false;

    UPROPERTY()
    bool bIsRaining = false;

    void UpdateAmbientSound();
    void FadeAudioVolume(float DeltaTime);
    USoundCue* GetCurrentAmbientSound();
};