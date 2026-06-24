#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Plains      UMETA(DisplayName = "Plains"),
    Cave        UMETA(DisplayName = "Cave"),
    Danger      UMETA(DisplayName = "Danger"),
    Camp        UMETA(DisplayName = "Camp")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bDangerZone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bIsActive = true;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ZoneType GetZoneType() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetMusicIntensity() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float CurrentFadeAlpha = 0.0f;
    bool bFadingIn = false;
};
