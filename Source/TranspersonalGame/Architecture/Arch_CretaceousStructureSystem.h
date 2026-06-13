#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_CretaceousStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneDwelling,
    RockShelter,
    CaveEntrance,
    WoodPlatform,
    BoneFramework,
    ClayHut
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Fresh,
    Aged,
    Weathered,
    Ancient,
    Ruins
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Fresh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 400.0f, 300.0f);

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::StoneDwelling;
        WeatheringLevel = EArch_WeatheringLevel::Fresh;
        StructuralIntegrity = 100.0f;
        bHasInterior = true;
        Dimensions = FVector(400.0f, 400.0f, 300.0f);
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_CretaceousStructureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_CretaceousStructureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMeshComponent*> StructureComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableWeathering = true;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(const FArch_StructureConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_StructureType GetStructureType() const { return StructureConfig.StructureType; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetStructuralIntegrity() const { return StructureConfig.StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanEnterInterior() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> GetInteriorSpawnPoints() const;

private:
    void CreateStructureComponents();
    void UpdateMaterialsForWeathering();
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);
    UMaterialInterface* GetMaterialForWeathering(EArch_WeatheringLevel Level);

    UPROPERTY()
    TArray<FVector> InteriorSpawnPoints;

    float WeatheringTimer = 0.0f;
};