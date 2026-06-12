#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    StoneShelter    UMETA(DisplayName = "Stone Shelter"),
    CliffDwelling   UMETA(DisplayName = "Cliff Dwelling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructuralIntegrity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bProvidesSafety = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::CaveEntrance;
        WeatherProtection = 0.8f;
        StructuralIntegrity = 1.0f;
        MaxOccupants = 4;
        bProvidesSafety = true;
        TemperatureModifier = 5.0f;
    }
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
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EntranceCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bIsOccupied = false;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const { return ShelterData.WeatherProtection; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const { return ShelterData.TemperatureModifier; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsSafe() const { return ShelterData.bProvidesSafety; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    int32 GetAvailableSpace() const;

protected:
    UFUNCTION()
    void OnEntranceBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnOccupantEntered(AActor* Occupant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnOccupantExited(AActor* Occupant);

    void UpdateShelterStatus();
    void ApplyWeatherDamage(float DeltaTime);
};