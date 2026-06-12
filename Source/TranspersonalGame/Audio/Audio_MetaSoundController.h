#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MetaSoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Config")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Config")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Config")
    float DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Config")
    float MinValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Config")
    float MaxValue;

    FAudio_MetaSoundConfig()
        : DefaultValue(1.0f)
        , MinValue(0.0f)
        , MaxValue(1.0f)
    {
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // MetaSound configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TArray<FAudio_MetaSoundConfig> MetaSoundConfigs;

    // Dynamic parameter control
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMetaSoundParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void PlayMetaSound(const FString& ConfigName);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void StopMetaSound(const FString& ConfigName);

    // Prehistoric-specific audio controls
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayDinosaurFootsteps(float Intensity, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayDinosaurRoar(float Aggression, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void UpdateAmbientForest(float TimeOfDay, float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayCraftingSound(const FString& MaterialType);

private:
    // Internal MetaSound management
    UPROPERTY()
    TMap<FString, class UAudioComponent*> ActiveMetaSounds;

    // Parameter interpolation
    UPROPERTY()
    TMap<FString, float> TargetParameters;

    UPROPERTY()
    TMap<FString, float> CurrentParameters;

    UPROPERTY(EditAnywhere, Category = "MetaSound")
    float ParameterInterpolationSpeed;

    // Prehistoric audio state
    UPROPERTY()
    float CurrentFootstepIntensity;

    UPROPERTY()
    float CurrentAmbientLevel;

    void InitializeMetaSounds();
    void UpdateParameterInterpolation(float DeltaTime);
    UAudioComponent* GetOrCreateAudioComponent(const FString& ConfigName);
};