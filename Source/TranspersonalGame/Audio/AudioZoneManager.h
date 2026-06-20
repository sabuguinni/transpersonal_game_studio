#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    CampSite        UMETA(DisplayName = "Camp Site"),
    DangerZone      UMETA(DisplayName = "Danger Zone")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MusicIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bDangerousZone = false;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetZoneType(EAudio_ZoneType NewZoneType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ZoneType GetZoneType() const { return ZoneData.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetBlendRadius() const { return ZoneData.BlendRadius; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDangerAlert();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ClearDangerAlert();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneData ZoneData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* DangerStinger = nullptr;

private:
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent = nullptr;

    bool bDangerActive = false;
    float DangerCooldown = 0.0f;
};
