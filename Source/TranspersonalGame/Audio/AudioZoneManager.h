#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Safe        UMETA(DisplayName = "Safe Zone"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    River       UMETA(DisplayName = "River Ambient"),
    Forest      UMETA(DisplayName = "Forest Ambient"),
    Cave        UMETA(DisplayName = "Cave Ambient"),
    Combat      UMETA(DisplayName = "Combat Zone")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MusicIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetCurrentIntensity() const { return CurrentIntensity; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* MusicLayer = nullptr;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* MusicAudioComponent = nullptr;

private:
    bool bPlayerInZone = false;
    float CurrentIntensity = 0.0f;
    float TargetIntensity = 0.0f;
    float BlendAlpha = 0.0f;
};
