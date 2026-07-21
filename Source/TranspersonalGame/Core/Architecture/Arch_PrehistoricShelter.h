#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_PrehistoricShelter.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    NaturalArch     UMETA(DisplayName = "Natural Arch"),
    StoneLedge      UMETA(DisplayName = "Stone Ledge")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float InteriorSpace = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bProvidesSafety = true;

    FArch_ShelterProperties()
    {
        WeatherProtection = 0.8f;
        TemperatureModifier = 5.0f;
        InteriorSpace = 100.0f;
        bHasFireplace = false;
        bProvidesSafety = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AArch_PrehistoricShelter : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricShelter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EntranceVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* InteriorLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    float ShelterScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    bool bAutoConfigureForBiome = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ConfigureShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsPlayerInShelter() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetupFireplace(bool bEnable);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerEnterShelter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerExitShelter();

protected:
    UFUNCTION()
    void OnEntranceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void InitializeShelterMesh();
    void ConfigureLighting();
    void SetupCollisionVolumes();

    bool bPlayerInShelter;
    float CurrentWeatherProtection;
    float CurrentTemperatureBonus;
};