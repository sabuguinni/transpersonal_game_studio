#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None = 0,
    RockOverhang,
    Cave,
    ElevatedPlatform,
    CliffDwelling,
    TemporaryLean
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone = 0,
    Wood,
    AnimalHide,
    Mud,
    Vine,
    Bone
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float SafetyRating = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float CapacityPersons = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    TArray<EArch_ConstructionMaterial> Materials;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::None;
        Location = FVector::ZeroVector;
        SafetyRating = 0.0f;
        CapacityPersons = 1.0f;
        bHasFirePit = false;
        bHasWaterAccess = false;
        Materials.Empty();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterShelter(const FArch_ShelterData& ShelterData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_ShelterData> FindNearbyShelters(FVector PlayerLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_ShelterData GetBestShelterForLocation(FVector Location, float SearchRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanBuildShelterAt(FVector Location, EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnShelterActor(const FArch_ShelterData& ShelterData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateShelterSafety(const FArch_ShelterData& ShelterData, FVector ThreatLocation);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_ShelterData> RegisteredShelters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float MaxShelterDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 MaxSheltersPerArea = 3;

private:
    void InitializeDefaultShelters();
    bool IsLocationSuitable(FVector Location, EArch_ShelterType ShelterType);
    float CalculateTerrainSuitability(FVector Location, EArch_ShelterType ShelterType);
};