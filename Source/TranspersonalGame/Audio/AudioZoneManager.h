#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeZone BiomeType = EAudio_BiomeZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DangerMusicBlend = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bPlayDinosaurDistantCalls = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DinosaurCallInterval = 15.0f;
};

/**
 * UAudioZoneManager — manages biome-based ambient audio zones.
 * Each zone blends ambient sounds based on player proximity and danger level.
 * Part of the Transpersonal Game audio system (prehistoric survival).
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioZoneManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioZoneManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetCurrentAmbientVolume() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_BiomeZone GetBiomeType() const;

private:
    float CurrentDangerLevel = 0.0f;
    float DinosaurCallTimer = 0.0f;

    void UpdateAmbientBlend(float DeltaTime);
    void TriggerDinosaurDistantCall();
};
