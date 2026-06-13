#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "Arch_StructuralIntegrity.h"
#include "Arch_PrehistoricShelter.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave            UMETA(DisplayName = "Natural Cave"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    StoneShelter    UMETA(DisplayName = "Stone Shelter"),
    WoodLean        UMETA(DisplayName = "Wood Lean-to"),
    BoneHut         UMETA(DisplayName = "Bone Hut")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureInsulation = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float MaxOccupants = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ComfortLevel = 0.5f;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::Cave;
        WeatherProtection = 0.8f;
        TemperatureInsulation = 0.6f;
        MaxOccupants = 4.0f;
        bHasFirePit = false;
        bHasStorageArea = false;
        ComfortLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricShelter : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricShelter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArch_StructuralIntegrity* StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float LastUsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    EBiomeType PreferredBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float MinDistanceFromWater = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float MaxDistanceFromWater = 2000.0f;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanAccommodateOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool AddOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool RemoveOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtectionAt(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsLocationProtected(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateShelterCondition();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetStorageLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FVector GetFirePitLocation() const;

private:
    void InitializeShelterMesh();
    void SetupInteriorVolume();
    void UpdateOccupancyStatus();
    bool ValidatePlacement() const;
};