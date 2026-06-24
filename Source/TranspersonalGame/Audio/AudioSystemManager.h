// AudioSystemManager.h
// Agent #16 — Audio Agent
// Adaptive audio zone system for prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory"),
    Cave            UMETA(DisplayName = "Cave"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bTriggerScreenShake = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ScreenShakeIntensity = 1.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneTriggerComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneTriggerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    USoundBase* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    USoundBase* MusicLayer = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }
};

UCLASS(ClassGroup = (Audio), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneTriggerComponent* TriggerZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetZoneType(EAudio_ZoneType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerDangerAlert(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

private:
    bool bPlayerInZone = false;
    float CurrentFadeProgress = 0.0f;

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
