#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Engine/DataTable.h"
#include "ProceduralAmbience.generated.h"

USTRUCT(BlueprintType)
struct FAmbientSoundData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDelay = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDelay = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectedByTension = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TensionVolumeMultiplier = 0.5f; // Volume quando tensão está alta
};

UENUM(BlueprintType)
enum class EAmbienceZoneType : uint8
{
    Forest,
    River,
    Cave,
    Clearing,
    DinosaurNest,
    PlayerBase,
    DangerZone
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralAmbience : public USceneComponent
{
    GENERATED_BODY()

public:
    UProceduralAmbience();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    EAmbienceZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    class UDataTable* AmbientSoundsTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    class USoundAttenuation* DefaultAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    int32 MaxConcurrentSounds = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    bool bEnabled = true;

    // Controle dinâmico
    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void SetTensionLevel(float TensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = meia-noite, 0.5 = meio-dia

    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void SetWeatherIntensity(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void EnableAmbience(bool bEnable);

protected:
    UPROPERTY()
    TArray<class UAudioComponent*> ActiveAudioComponents;

    float CurrentTensionLevel;
    float CurrentTimeOfDay;
    float CurrentWeatherIntensity;
    
    FTimerHandle SoundSpawnTimer;
    
    void SpawnRandomAmbientSound();
    void CleanupFinishedSounds();
    FVector GetRandomLocationInZone() const;
    FAmbientSoundData* GetRandomSoundForCurrentConditions();
    void UpdateExistingSounds();

private:
    TArray<FAmbientSoundData*> CachedSoundData;
    void CacheSoundData();
};