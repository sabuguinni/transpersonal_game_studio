#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave        UMETA(DisplayName = "Natural Cave"),
    RockShelter UMETA(DisplayName = "Rock Overhang"),
    Hut         UMETA(DisplayName = "Primitive Hut"),
    Windbreak   UMETA(DisplayName = "Wind Break")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bIsOccupied = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float InteriorTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float ShelterIntegrity = 100.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool EnterShelter(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool ExitShelter(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void LightFire();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanAccommodate(int32 AdditionalOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateShelterCondition(float DeltaTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnOccupantEntered(AActor* Occupant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnOccupantExited(AActor* Occupant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnFireLit();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnFireExtinguished();

protected:
    UFUNCTION()
    void OnInteriorVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float FireBurnTime = 0.0f;
    float MaxFireDuration = 300.0f; // 5 minutes
    float WeatherDamageRate = 0.1f;
    float RepairRate = 0.05f;
};