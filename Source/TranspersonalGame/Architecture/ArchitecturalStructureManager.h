#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "TranspersonalGame/SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    StoneArch,
    Pillar,
    Wall,
    Foundation,
    Ruins,
    CaveEntrance,
    RockFormation
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    None = 0,
    Limestone,
    Sandstone,
    Basalt,
    Granite,
    Wood,
    Clay,
    Bone
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionMaterial Material = EArch_ConstructionMaterial::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Age = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuined = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Material = EArch_ConstructionMaterial::None;
        Dimensions = FVector::ZeroVector;
        Age = 0.0f;
        WeatheringLevel = 0.0f;
        bHasMossGrowth = false;
        bIsRuined = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString SpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector SpaceDimensions = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> FurnitureItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float LightLevel = 0.5f;

    FArch_InteriorSpace()
    {
        SpaceName = TEXT("");
        SpaceDimensions = FVector::ZeroVector;
        FurnitureItems.Empty();
        bHasFirePit = false;
        bHasStorage = false;
        LightLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_InteriorSpace> InteriorSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UStaticMesh* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* WeatheredMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bIsHistoricalSite = false;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType InType, EArch_ConstructionMaterial InMaterial, FVector InDimensions);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorSpace(const FArch_InteriorSpace& NewSpace);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMossGrowth(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_InteriorSpace> GetInteriorSpaces() const { return InteriorSpaces; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetStructuralIntegrity() const { return StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureRuined() const { return StructureData.bIsRuined; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetAsHistoricalSite(bool bIsHistorical) { bIsHistoricalSite = bIsHistorical; }

protected:
    UFUNCTION()
    void UpdateStructuralIntegrity(float DeltaTime);

    UFUNCTION()
    void ProcessWeathering(float DeltaTime);

    UFUNCTION()
    UMaterialInterface* GetMaterialForType(EArch_ConstructionMaterial MaterialType);
};