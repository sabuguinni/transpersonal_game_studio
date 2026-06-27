#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    ForestAmbient   UMETA(DisplayName = "Forest Ambient"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
    WateringHole    UMETA(DisplayName = "Watering Hole"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    Riverbank       UMETA(DisplayName = "Riverbank")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::ForestAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MusicIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float DangerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLoopAmbient = true;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioZoneManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioZoneManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TransitionSpeed = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void EnterZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void ExitZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetCurrentDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetMusicIntensity(float NewIntensity);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float CurrentVolume = 0.0f;
    float TargetVolume = 0.0f;
    bool bPlayerInZone = false;
};
