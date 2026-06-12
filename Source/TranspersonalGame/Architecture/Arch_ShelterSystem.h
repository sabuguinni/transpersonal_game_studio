#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    NaturalAlcove   UMETA(DisplayName = "Natural Alcove")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bProvidesWarmth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bBlocksWind = true;
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
    UBoxComponent* EntranceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    TArray<AActor*> CurrentOccupants;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsOccupied() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    int32 GetAvailableSpace() const;

protected:
    UFUNCTION()
    void OnEntranceOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnActorEnteredShelter(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnActorExitedShelter(AActor* Actor);

private:
    void UpdateShelterEffects();
    void ApplyShelterBenefits(AActor* Actor);
    void RemoveShelterBenefits(AActor* Actor);
};