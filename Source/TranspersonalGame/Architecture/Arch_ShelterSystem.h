#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    NaturalShelter  UMETA(DisplayName = "Natural Shelter")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bProvidesSafety = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ProtectionZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UMaterialInterface*> WeatheredMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsPlayerInShelter() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerEnterShelter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerExitShelter();

protected:
    UFUNCTION()
    void OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInShelter = false;
    float LastWeatherCheck = 0.0f;
    const float WeatherCheckInterval = 2.0f;
};